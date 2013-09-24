#include <dcpu/devices/gkeyboard.hpp>
#include <config.hpp>

#include <algorithm>

namespace cpu {

namespace keyboard {

GKeyboard::GKeyboard() : msg(0), events(0)
{ }

GKeyboard::~GKeyboard()
{ }

void GKeyboard::attachTo (DCPU* cpu, size_t index)
{
    this->IHardware::attachTo(cpu, index);

    msg = 0;
    events = 0;
    keybuffer.clear();
}

bool GKeyboard::checkInterrupt (uint16_t &msg)
{
    if (events > 0 && this->msg > 0) {
        msg = this->msg;
        events--;
        LOG_DEBUG << "[GKeyboard] Interrupt!";
        return true;
    }
    return false;
}

unsigned GKeyboard::handleInterrupt()
{
    switch (cpu->getA()) {
    case CLEAR_BUFFER:
        keybuffer.clear();
        break;

    case POP:
        if (keybuffer.empty()) {
            cpu->setC(0);
        } else {
            cpu->setC(keybuffer.back() & 0x01FF);
            keybuffer.pop_back();
        }
        break;

    case IS_PRESS:
    {   // Search the last time that key was down/up
        unsigned char key = cpu->getB() & 0x00FF;
        auto result = std::find_if(keybuffer.rbegin(), keybuffer.rend(), 
                        [&key](uint16_t &i) -> bool {
                            return (i & 0x00FF) == key;
                        } );
        // if have the KeyDown flag, then is pressed
        if (!(result == keybuffer.rend()) && (*result)& 0xFF00 ) {
            cpu->setC(1);
        } else {
            cpu->setC(0);
        }
        break;
    }

    case SET_MSG:
        msg = cpu->getB();
        break;

    default:
        ;
    }

    return 0;
}

void GKeyboard::pushKeyEvent (bool keydown, unsigned char scancode)
{
    uint16_t k = scancode | ( keydown ? 0x0100 : 0x0000 );
    keybuffer.push_front(k);
    events++;
    while (keybuffer.size() > BUFFER_SIZE) {
        keybuffer.pop_back(); // Removes old elements
    }

    if (keydown)
        LOG_DEBUG << "[GKeyboard] Pushed Key : " << scancode;
    else
        LOG_DEBUG << "[GKeyboard] Released Key : " << scancode;

}

} // END OF NAMESPACE keyboard

} // END OF NAMESPACE cpu
