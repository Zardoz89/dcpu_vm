#pragma once
#ifndef _LEM1802_HPP
#define _LEM1802_HPP
#include <dcpu/dcpu.hpp> 
#include <dcpu/devices/amonitor.hpp> //base class AbstractMonitor

#include <cstdint>


namespace cpu {

namespace lem {
static const uint16_t MEM_MAP_SCREEN        = 0;
static const uint16_t MEM_MAP_FONT          = 1;
static const uint16_t MEM_MAP_PALETTE       = 2;
static const uint16_t SET_BORDER_COLOR      = 3;
static const uint16_t MEM_DUMP_FONT         = 4;
static const uint16_t MEM_DUMP_PALETTE      = 5;

static const unsigned REFRESHRATE           = 50;
static const unsigned SPLASHTIME            = 1; // In seconds

/**
 * @brief LEM1802 that uses SFML
 */
class Lem1802 : public cpu::AbstractMonitor {
public:
    Lem1802();
    virtual ~Lem1802();
    
    static const unsigned int WIDTH             = 128;
    static const unsigned int HEIGHT            = 96;
    static const unsigned int BORDER_SIZE       = 10;

    static const unsigned int ROWS              = 12;
    static const unsigned int COLS              = 32;

    static const unsigned int BLINKPERSECOND    = 2;

    virtual uint32_t getId() const {
        return 0x7349f615;
    }
    virtual uint16_t getRevision() const {
        return 0x1802;
    }
    virtual uint32_t getManufacturer() const {
        return 0x1c6c8b36;
    }
    
    virtual unsigned int phyWidth() const {return WIDTH;}
    virtual unsigned int phyHeight() const {return HEIGHT;}
    unsigned int borderSize() {return BORDER_SIZE;}
    
    bool checkInterrupt (uint16_t &msg) {
        return false;
    }

    virtual unsigned handleInterrupt();

    virtual void tick();
    
    virtual void attachTo (DCPU* cpu, size_t index);

    

    virtual uint32_t getBorder() const;


    const static uint16_t def_palette_map[16];   /// Default palette
    const static uint16_t def_font_map[128*2];   /// Default fontmap

protected:
    virtual void updateScreen();

    uint16_t screen_map;                /// Where map VIDEO RAM 
    uint16_t font_map;                  /// Where map FONT
    uint16_t palette_map;               /// Where map PALETTE
    
    uint8_t border_col;                 /// Border color palette index
    uint32_t ticks;                     /// CPU clock ticks (for timing)
    
    uint_fast32_t blink;                /// Counter for blinking
    uint_fast32_t blink_max;            /// Max ticks to change blink state

};

}//end of lem NAMESPACE

}//end of cpu NAMESPACE

#endif // _LEM1802_HPP
