#include <devices/m35fd.hpp>

#include <cassert>

namespace cpu {

namespace m35fd {

// M35FD class ****************************************************************

M35FD::M35FD() : state(STATE_CODES::NO_MEDIA), error(ERROR_CODES::NONE), 
                 tick_counter(0)
{ }

M35FD::~M35FD()
{ }

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
}

bool M35FD::checkInterrupt (uint16_t& msg)
{
    if (this->msg != 0 && true) {
        msg = this->msg;
        return true;
    }
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
        LOG_DEBUG << "[M35FD] polling E:" << static_cast<uint16_t>(state) <<
                                    " R:" << static_cast<uint16_t>(error);
        break;

    case COMMANDS::SET_INTERRUPT :
        msg = cpu->getX();
        break;

    case COMMANDS::READ_SECTOR :
        if (state == STATE_CODES::READY || state == STATE_CODES::READY_WP) {
            // TODO Read to the floppy
            state == STATE_CODES::BUSY;
            cpu->setB(1);
        } else {
            if (state == STATE_CODES::NO_MEDIA)
                error = ERROR_CODES::NO_MEDIA;

            cpu->setB(0);
        }
        break;

    case COMMANDS::WRITE_SECTOR :
        if (state == STATE_CODES::READY) {
            // TODO Write to the floppy
            state == STATE_CODES::BUSY;
            cpu->setB(1);
        } else {
            if (state == STATE_CODES::NO_MEDIA)
                error = ERROR_CODES::NO_MEDIA;
            else if (state == STATE_CODES::READY_WP)
                error = ERROR_CODES::PROTECTED;

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
    tick_counter++;
}

void M35FD::insertFloppy (std::shared_ptr<M35_Floppy> floppy)
{
    if (this->floppy)
        eject();

    this->floppy = floppy;
    state = floppy->isProtected()? STATE_CODES::READY_WP : STATE_CODES::READY;
    error = ERROR_CODES::NONE;
    
    LOG << "[M35FD] Disk inserted!"; 
}

void M35FD::eject()
{
    if (this->floppy)
        this->floppy = std::shared_ptr<M35_Floppy>(); // like = NULL

    LOG << "[M35FD] Disk ejected!"; 

    if (state == STATE_CODES::BUSY) // Wops!
        error = ERROR_CODES::EJECT;
    else
        error = ERROR_CODES::NONE;

    state = STATE_CODES::NO_MEDIA;
}

// M35_Floppy class ***********************************************************

M35_Floppy::M35_Floppy(const std::string filename, uint8_t tracks, bool wp) :
                        tracks(tracks), bad_sectors(NULL), 
                        wp_flag(wp), last_sector(0)
{
    assert(tracks == 80 || tracks == 40);
    datafile.open(filename, std::ios::in | std::ios::out | std::ios::binary);
}

M35_Floppy::~M35_Floppy()
{
    if (datafile.is_open())
        datafile.close();
}

bool M35_Floppy::isSectorBad (uint16_t sector) const
{
    return false; // TODO
}

void M35_Floppy::setSectorBad (uint16_t sector, bool state)
{
    return; // TODO
}

ERROR_CODES M35_Floppy::write (uint16_t sector, unsigned& cycles, 
                                const uint16_t* data, size_t size)
{
    return ERROR_CODES::NONE;
}

ERROR_CODES M35_Floppy::read (uint16_t sector, unsigned& cycles, 
                                uint16_t* data, size_t size)
{
    return ERROR_CODES::NONE;
}


} // END OF NAMESPACE m35fd

} // END OF NAMESPACE cpu

