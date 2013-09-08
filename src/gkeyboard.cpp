#include "gkeyboard.hpp"

#include <iostream>

#include <algorithm>
namespace cpu {

namespace keyboard {

    GKeyboard::GKeyboard() : msg(0), event(false)
    { }

    GKeyboard::~GKeyboard()
    { }

    void GKeyboard::attachTo (DCPU* cpu, size_t index)
    {
        this->IHardware::attachTo(cpu, index);

        msg = 0;
        event = false;
        keybuffer.clear();
    }

    bool GKeyboard::checkInterrupt (uint16_t &msg)
    {
        if (event && this->msg > 0) {
            event = false;
            return true;
        }
        return false;
    }

    void GKeyboard::handleInterrupt()
    {
        switch (cpu->GetA()) {
        case CLEAR_BUFFER:
            keybuffer.clear();
            break;

        case POP:
            if (keybuffer.empty()) {
                cpu->SetC(0);
            } else {
                cpu->SetC(keybuffer.back() & 0x01FF);
                keybuffer.pop_back();
            }
            break;

        case IS_PRESSED: // PUSH...
        {   // Search the last time that key was down/up
            unsigned char key = cpu->GetB() & 0x00FF;
            auto result = std::find_if(keybuffer.rbegin(), keybuffer.rend(), 
                            [&key](uint16_t &i) -> bool {
                                return (i & 0x00FF) == key;
                            } );
            // if have the KeyDown flag, then is pressed
            if (!(result == keybuffer.rend()) && (*result)& 0xFF00 ) {
                cpu->SetC(1);
            } else {
                cpu->SetC(0);
            }
            break;
        }

        case SET_MSG:
            msg = cpu->GetB();
            break;

        default:
            ;
        }
    }

    void GKeyboard::pushKeyEvent (bool keydown, unsigned char scancode)
    {
        uint16_t k = scancode | ( keydown ? 0x0100 : 0x0000 );
        keybuffer.push_front(k);
        event = true;
        while (keybuffer.size() > BUFFER_SIZE) {
            keybuffer.pop_back(); // Removes old elements
        }
    }

} // END OF NAMESPACE keyboard

} // END OF NAMESPACE cpu
