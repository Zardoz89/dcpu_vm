#include <devices/m35fd.hpp>

namespace cpu {

namespace m35fd {

M35FD::M35FD() : state(STATE_CODES::NO_MEDIA), error(ERROR_CODES::NONE), 
    tick_counter(0)
{ }

M35FD::~M35FD()
{ }

void M35FD::attachTo (DCPU* cpu, size_t index)
{
    this->IHardware::attachTo(cpu, index);
}

bool M35FD::checkInterrupt (uint16_t& msg)
{
    if (this->msg != 0 && true) {
        msg = this->msg;
        return true;
    }
    return false;
}

void M35FD::handleInterrupt()
{
    if (cpu == NULL)
        return;

    switch (static_cast<COMMANDS>(cpu->getA())) {
    case COMMANDS::POLL :
        cpu->setB(static_cast<uint16_t>(state));
        cpu->setC(static_cast<uint16_t>(error));
        Debug(LogLevel::INFO) << "[M35FD] polling" 
            << static_cast<uint16_t>(state) << " " 
            << static_cast<uint16_t>(error);
        break;

    case COMMANDS::SET_INTERRUPT :
        msg = cpu->getX();
        break;

    case COMMANDS::READ_SECTOR :
        if (state == STATE_CODES::READY || state == STATE_CODES::READY_WP) {
            // TODO Read to the floppy
        } else {
            if (state == STATE_CODES::NO_MEDIA)
                error = ERROR_CODES::NO_MEDIA;

            cpu->setB(0);
        }
        break;

    case COMMANDS::WRITE_SECTOR :
        if (state == STATE_CODES::READY) {
            // TODO Write to the floppy
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

    return;
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
}

void M35FD::eject()
{
    if (this->floppy)
        this->floppy = std::shared_ptr<M35_Floppy>(); // like = NULL

    if (state == STATE_CODES::BUSY) // Wops!
        error = ERROR_CODES::EJECT;
    else
        error = ERROR_CODES::NONE;

    state = STATE_CODES::NO_MEDIA;
}


} // END OF NAMESPACE m35fd

} // END OF NAMESPACE cpu

