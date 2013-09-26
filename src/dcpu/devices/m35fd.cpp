#include <dcpu/devices/m35fd.hpp>

#include <cassert>

namespace cpu {

namespace m35fd {

// M35FD class ****************************************************************

M35FD::M35FD() : state(STATE_CODES::NO_MEDIA), error(ERROR_CODES::NONE), 
                 busy_cycles(0), trigger(false)
{ }

M35FD::~M35FD()
{
    if (floppy)
        eject();
}

void M35FD::attachTo (DCPU* cpu, size_t index)
{
    this->IHardware::attachTo(cpu, index);
    if (floppy) {
        state = floppy->isProtected() ? 
                    STATE_CODES::READY_WP : STATE_CODES::READY;
    } else {
        state = STATE_CODES::NO_MEDIA;
    }

    error = ERROR_CODES::NONE;
    trigger = false;
}

bool M35FD::checkInterrupt (uint16_t& msg)
{
    if (this->msg != 0 && trigger) {
        msg = this->msg;
        trigger = false;
        return true;
    }

    trigger = false;
    return false;
}

unsigned M35FD::handleInterrupt()
{
    if (cpu == NULL)
        return 0;

    switch (static_cast<COMMANDS>(cpu->getA())) {
    case COMMANDS::POLL :
        cpu->setB(static_cast<uint16_t>(state));
        cpu->setC(static_cast<uint16_t>(error));
        break;

    case COMMANDS::SET_INTERRUPT :
        msg = cpu->getX();
        LOG_DEBUG << "[M35FD] msg set to " << msg; 
        break;

    case COMMANDS::READ_SECTOR :
        if (state == STATE_CODES::READY || state == STATE_CODES::READY_WP) {
            if (! floppy) { // This not should happen never, but...
                state = STATE_CODES::NO_MEDIA;
                error = ERROR_CODES::BROKEN;
                cpu->setB(0);
                LOG_ERROR << "[M35FD] Something weird happen trying to Read";
                break;
            }
            error = floppy->read(cpu->getX(), cpu->getY(), busy_cycles);
            state = STATE_CODES::BUSY;
            if (error == ERROR_CODES::NONE)
                cpu->setB(1);
            else
                cpu->setB(0);
            
            trigger = true; // State changes, and error could
        } else {
            if (state == STATE_CODES::NO_MEDIA) {
                error = ERROR_CODES::NO_MEDIA;
                trigger = true; // error changes
            } else if (state == STATE_CODES::BUSY) {
                error = ERROR_CODES::BUSY;
                trigger = true; // error changes
            }

            LOG_DEBUG << "[M35FD] Reading set to Error: " 
                        << static_cast<int>(error);
            cpu->setB(0);
        }
        break;

    case COMMANDS::WRITE_SECTOR :
        if (state == STATE_CODES::READY) {
            if (! floppy) { // This not should happen never, but...
                state = STATE_CODES::NO_MEDIA;
                error = ERROR_CODES::BROKEN;
                cpu->setB(0);
                LOG_ERROR << "[M35FD] Something weird happen trying to Write";
                break;
            }
            error = floppy->write(cpu->getX(), cpu->getY(), busy_cycles);
            state = STATE_CODES::BUSY;
            if (error == ERROR_CODES::NONE)
                cpu->setB(1);
            else
                cpu->setB(0);
            
            trigger = true; // State changes, and error could
        } else {
            if (state == STATE_CODES::NO_MEDIA) {
                error = ERROR_CODES::NO_MEDIA;
                trigger = true; // error changes
            } else if (state == STATE_CODES::READY_WP) {
                error = ERROR_CODES::PROTECTED;
                trigger = true; // error changes
            } else if (state == STATE_CODES::BUSY) {
                error = ERROR_CODES::BUSY;
                trigger = true; // error changes
            }

            LOG_DEBUG << "[M35FD] Writing set to Error: " 
                        << static_cast<int>(error);
            cpu->setB(0);
        }
        break;

    default:
        break;
    }

    return 0;
}

void M35FD::tick()
{
    if (busy_cycles > 0 && state == STATE_CODES::BUSY) {
        busy_cycles--;
        // Read/write 1 word each 3 cycles. A bit more faster that the suposed
        // specs speeds, but more easy to do
        if (floppy && busy_cycles % 3) 
            floppy->tick();

    } else if (floppy && state == STATE_CODES::BUSY ) {
        state = floppy->isProtected() ? 
            STATE_CODES::READY_WP : STATE_CODES::READY;

        trigger = true; // State changes
    }
}

void M35FD::insertFloppy (std::shared_ptr<M35_Floppy> floppy)
{
    if (this->floppy)
        eject();

    this->floppy = floppy;
    state = floppy->isProtected()? STATE_CODES::READY_WP : STATE_CODES::READY;
    error = ERROR_CODES::NONE;
    this->floppy->drive = this;
    
    trigger = true; // State changes, and error could
    LOG << "[M35FD] Disk inserted! " << floppy->getFilename(); 
}

void M35FD::eject()
{
    if (this->floppy) {
        this->floppy->drive = NULL;
        this->floppy = std::shared_ptr<M35_Floppy>(); // like = NULL
    }

    LOG << "[M35FD] Disk ejected! "; 

    if (state == STATE_CODES::BUSY) // Wops!
        error = ERROR_CODES::EJECT;
    else
        error = ERROR_CODES::NONE;

    state = STATE_CODES::NO_MEDIA;
    trigger = true; // State changes, and error could
}

// M35_Floppy class ***********************************************************

size_t data_pos (size_t sector, size_t index)
{
    return 4 + sector*SECTOR_SIZE*2 + index*2; 
    // We wrok with Words, but file post is in bytes!!   
}

M35_Floppy::M35_Floppy(const std::string filename, uint8_t tracks, bool wp) :
                        filename(filename), tracks(tracks), bad_sectors(NULL),
                        wp_flag(wp), last_sector(0), cursor(0), drive(NULL)
{
    assert(tracks == 80 || tracks == 40);
   
    // Qucik and dirty way to see if file exists
    bool create_header = false;
    std::ifstream f(filename);
    if (!f.good()) {
        create_header = true;
        LOG << "[M35FD] Disk datafile not exists. Creating it."; 
    } 
    f.close();

    bad_sectors = new uint8_t[(tracks * SECTORS_PER_TRACK) >> 3];

    if (create_header) {
        datafile.open(filename, std::ios::in | std::ios::out | 
                                std::ios::binary | std::ios::trunc);
        datafile.write(&FileHeader[0], 1);
        datafile.write(&FileHeader[1], 1);
        datafile.seekg(1, std::fstream::cur);
        datafile.write((const char*)(&tracks), 1);
    } else {
        datafile.open(filename, std::ios::in | std::ios::out |
                                std::ios::binary);
    }
}

M35_Floppy::~M35_Floppy()
{
    if (datafile.is_open()) {
        datafile.close();
        LOG << "[M35FD] Datafile closed"; 
    }

    delete[] bad_sectors;
}

bool M35_Floppy::isSectorBad (uint16_t sector) const
{
    return false; // TODO
}

void M35_Floppy::setSectorBad (uint16_t sector, bool state)
{
    return; // TODO
}

ERROR_CODES M35_Floppy::write (uint16_t sector, uint16_t addr,
                                unsigned& cycles) 
{
    // From 0 to max
    if (sector >= SECTORS_PER_TRACK*tracks)
        return ERROR_CODES::BAD_SECTOR;

    cycles = setTrack(sector / SECTORS_PER_TRACK);
    cycles += WRITE_CYCLES_PER_SECTOR;
   
    cursor = addr;
    count = 0;
    reading = false;

    last_sector = sector;
    return ERROR_CODES::NONE;
}

ERROR_CODES M35_Floppy::read (uint16_t sector, uint16_t addr,
                                unsigned& cycles) 
{
    // From 0 to max
    if (sector >= SECTORS_PER_TRACK*tracks)
        return ERROR_CODES::BAD_SECTOR;
    
    cycles = setTrack(sector / SECTORS_PER_TRACK);
    cycles += READ_CYCLES_PER_SECTOR;
    
    cursor = addr;
    count = 0;
    reading = true;

    last_sector = sector;
    return ERROR_CODES::NONE;
}

void M35_Floppy::tick()
{
    char buff[2];
    if (count < SECTOR_SIZE) {
        datafile.seekp(data_pos (last_sector, count), std::ios::beg);
        // TODO Force endianess
        if (reading) {
            uint8_t tmp;
            datafile.read (buff, 2);
            tmp = buff[0];
            buff[0] = buff[1];
            buff[1] = tmp;
            drive->cpu->getMem()[cursor + count++] = *((uint16_t*)buff);
        } else {
            buff[0] = drive->cpu->getMem()[cursor + count] >> 8;
            buff[1] = 0xFF & drive->cpu->getMem()[cursor + count++];
            datafile.write (buff, 2);
        }
    }
}

} // END OF NAMESPACE m35fd

} // END OF NAMESPACE cpu

