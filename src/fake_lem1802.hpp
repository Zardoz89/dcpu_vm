#ifndef FAKE_LEM1802_HPP
#define FAKE_LEM1802_HPP

#include <iostream>
#include <cstdint>

#include "dcpu.hpp" // Base class: cpu::IHardware


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
   
    const static uint16_t def_palette_map[16];   /// Default palette
    const static uint16_t def_font_map[128*2];   /// Default fontmap

private:

    uint16_t screen_map;            /// Where map VIDEO RAM 
    uint16_t font_map;              /// Where map FONT
    uint16_t palette_map;           /// Where map PALETTE
    
    uint8_t border_col;             /// Border color (unused)
    uint32_t ticks;                 /// CPU clock ticks (for timing)
    
    uint32_t tick_per_refresh;      /// How many ticks before refresh

    bool enable;                    /// Can print to stdout ?
    
};

}

#endif // FAKE_LEM1802_HPP
