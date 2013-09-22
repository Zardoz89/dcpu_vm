#pragma once
#ifndef _CGM_HPP
#define _CGM_HPP

#include <dcpu/dcpu.hpp> // Base class: cpu::IHardware
#include <dcpu/devices/amonitor.hpp>

#include <cstdint>

#include <SFML/Graphics.hpp>

namespace cpu {

namespace cgm {

enum COMMANDS { /// A register commands
    MEM_BITPLANE_SCREEN,
    MEM_ATTRIBUTE_SCREEN,
    MEM_MAP_PALETTE,
    SET_BORDER_COLOR,
    SET_VIDEO_MODE,
    GET_VIDEO_MODE,
    MEM_DUMP_PALETTE,
    MEM_DUMP_FONT,
    MEM_MAP_FONT,
};

static const unsigned REFRESHRATE           = 50;
static const unsigned SPLASHTIME            = 2; // In seconds

/**
 * @brief CGM 1084 monitor that uses SFML
 */
class CGM : public cpu::AbstractMonitor {
public:
    CGM();
    virtual ~CGM();
                                          
    static const unsigned int WIDTH         = 256;
    static const unsigned int HEIGHT        = 192;
    static const unsigned int BORDER_SIZE   = 10;

    static const unsigned int ROWS[];
    static const unsigned int COLS[];
    
    
    static const uint16_t BLINKPERSECOND    = 2;

    uint32_t getId() const {
        return 0x7349043C;
    }
    virtual uint16_t getRevision() const {
        return 0x1084;
    }
    uint32_t getManufacturer() const {
        return 0x0ca0fe84;
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

    const static uint16_t def_palette_map[64];          /// Default palette
    const static uint16_t def_fonts[256*2 + 256*4];     /// Font maps

protected:
    virtual void updateScreen();


    uint16_t bitfield_map;          /// Where map BIT-FIELD 
    uint16_t attribute_map;         /// Where map ATTRIBUTE CELLS 
    uint16_t palette_map;           /// Where map PALETTE
    uint16_t font_map;              /// Where map FONT in text modex
    uint16_t videomode;             /// Video mode
    
    uint8_t border_col;             /// Border color (unused)
    uint32_t ticks;                 /// CPU clock ticks (for timing)

    uint_fast32_t blink;            /// Counter for blinking
    uint_fast32_t blink_max;        /// How many count to blink
   

};

} // END OF cgm NAMESPACE

} // END OF cpu NAMESPACE

#endif // _CGM_HPP
