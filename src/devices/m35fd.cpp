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

    state = STATE_CODES::NO_MEDIA;
    error = ERROR_CODES::NONE;
}


} // END OF NAMESPACE m35fd

} // END OF NAMESPACE cpu

