#ifndef FAKE_LEM1802_HPP
#define FAKE_LEM1802_HPP

#include <iostream>
#include <cstdint>

#include "dcpu.hpp" // Base class: cpu::IHardware

#define MEM_MAP_SCREEN   0
#define MEM_MAP_FONT     1
#define MEM_MAP_PALETTE  2
#define SET_BORDER_COLOR 3
#define MEM_DUMP_FONT    4
#define MEM_DUMP_PALETTE 5

namespace cpu {

/**
 * @brief Fake LEM1802 that type ascii text to STDOUT
 */
class Fake_Lem1802 : public cpu::IHardware {
public:
    Fake_Lem1802();
    virtual ~Fake_Lem1802();
    
    static const uint32_t id            = 0x7349f615;
    static const uint16_t revision      = 0x1802;
    static const uint32_t manufacturer  = 0x1c6c8b36;
    virtual uint32_t getId() {
        return id;
    }
    virtual uint16_t getRevision() {
        return revision;
    }
    virtual uint32_t getManufacturer() {
        return manufacturer;
    }
    
    bool checkInterrupt (uint16_t &msg) {
        return false;
    }
    virtual void handleInterrupt();
    virtual void tick();
    
    virtual void attachTo (DCPU* cpu, size_t index);
    
    /**
     * @brief Try to show the screen in the terminal
     */
    void show();

    /**
     * @brief Sets if it can display to stdout
     */
    void setEnable(bool enable);
    
private:

    uint16_t screen_map;            /// Where map VIDEO RAM 
    uint16_t font_map;              /// Where map FONT
    uint16_t palette_map;           /// Where map PALETTE
    
    uint8_t border_col;             /// Border color (unused)
    const uint16_t *def_font_map;   /// Default font map
    uint16_t def_palette_map[16];   /// Default palette
    uint32_t ticks;                 /// CPU clock ticks (for timing)
    
    uint32_t tick_per_refresh;      /// How many ticks before refresh

    bool enable;                    /// Can print to stdout ?
    
};

}

#endif // FAKE_LEM1802_HPP
