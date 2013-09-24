#pragma once
#ifndef _GENERIC_KEYBOARD_HPP_
#define _GENERIC_KEYBOARD_HPP_ 1

#include <dcpu/dcpu.hpp>

#include <cstdint>
#include <deque>

namespace cpu {

namespace keyboard {

enum COMMANDS { /// A register commands
    CLEAR_BUFFER,
    POP,
    IS_PRESS,
    SET_MSG,
    GET_STATUS,
    SET_STATUS,
    PUSH
};

enum SCANCODES {
  BACKSPACE = 0x10,
  RETURN,
  INSERT,
  DELETE,

  ESC = 0x1B,
  
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

    static const size_t BUFFER_SIZE         = 64;

    uint32_t getId() const {
        return 0x30CF7406;
    }
    virtual uint16_t getRevision() const {
        return 0x0001;
    }
    uint32_t getManufacturer() const {
        return 0x90099009;
    }
   
    bool checkInterrupt (uint16_t &msg);

    virtual unsigned handleInterrupt();
    
    virtual void attachTo (DCPU* cpu, size_t index);

    /**
     * Push to the keyboard buffer a keycode
     * @param keydown True if the key is being pressed down. False is being
     * released
     * @param scancode A scancode from 0 to 255
     */
    void pushKeyEvent(bool keydown, unsigned char scancode);

    size_t bufferSize()
    {
        return keybuffer.size();
    }

    void tick() 
    { }

protected:
    std::deque<uint16_t> keybuffer;

    uint16_t msg;           /// Interrupt Msg
    uint_fast16_t events;   /// Was a keyboard event ?

private:
};

} // END OF NAMESPACE keyboard

} // END OF NAMESPACE cpu

#endif // _GENERIC_KEYBOARD_HPP_
