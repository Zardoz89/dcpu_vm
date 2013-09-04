#include "gkeyboard.hpp"

#include <algorithm>
namespace cpu {

namespace keyboard {

    GKeyboard::GKeyboard() : msg(0)
    { }

    GKeyboard::~GKeyboard()
    { }

    void GKeyboard::attachTo (DCPU* cpu, size_t index)
    {
        this->IHardware::attachTo(cpu, index);

        msg = 0;
        keybuffer.clear();
    }

    bool GKeyboard::checkInterrupt (uint16_t &msg)
    {
        if (event && msg !=0) {
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
                cpu->SetC(keybuffer.front() & 0x00FF);
                keybuffer.pop_front();
            }
            break;

        case IS_PRESSED:
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

        case SET_INTERRUPT:
            msg = cpu->GetB();
            break;

        default:
            ;
        }
    }

    void GKeyboard::pushKeyEvent (bool keydown, unsigned char keycode)
    {
        uint16_t k = keycode | ( keydown ? 0x0100 : 0x0000 );
        keybuffer.push_back(k);
        event = true;
    }

} // END OF NAMESPACE keyboard

} // END OF NAMESPACE cpu
