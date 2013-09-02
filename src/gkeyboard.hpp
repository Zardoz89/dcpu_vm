#pragma once
#ifndef _GENERIC_KEYBOARD_HPP_
#define _GENERIC_KEYBOARD_HPP_ 1

#include "dcpu.hpp"

#include <cstdint>
#include <deque>

namespace cpu {

namespace keyboard {

enum COMMANDS { /// A register commands
    CLEAR_BUFFER,
    POP,
    IS_PRESSED,
    SET_INTERRUPT,
};

enum KEYCODES {
  BACKSPACE = 0x10,
  RETURN,
  INSERT,
  DELETE,
  // 0x20-0x7f: ASCII characters
  ARROW_UP = 0x80,
  ARROW_DOWN,
  ARROW_LEFT,
  ARROW_RIGHT,
  SHIFT = 0x90,
  CONTROL,
};

class GKeyboard : public IHardware {
public:

    GKeyboard();
    virtual ~GKeyboard();

    static const uint32_t ID                = 0x30CF7406;
    static const uint16_t REV               = 0x0001;
    static const uint32_t MANUFACTURER      = 0x00000000;

    uint32_t getId() {
        return ID;
    }
    virtual uint16_t getRevision() {
        return REV;
    }
    uint32_t getManufacturer() {
        return MANUFACTURER;
    }
   
    bool checkInterrupt (uint16_t &msg);

    virtual void handleInterrupt();
    
    virtual void attachTo (DCPU* cpu, size_t index);

    /**
     * Push to the keyboard buffer a keycode
     * @param keydown True if the key is being pressed down. False is being
     * released
     * @param keycode A keycode from 0 to 255
     */
    void pushKeyEvent(bool keydown, unsigned char keycode);

    size_t bufferSize()
    {
        return keybuffer.size();
    }

    void tick() 
    { }

protected:
    std::deque<uint16_t> keybuffer;

    int16_t msg;
    bool event;

private:
};

} // END OF NAMESPACE keyboard

} // END OF NAMESPACE cpu

#endif // _GENERIC_KEYBOARD_HPP_
