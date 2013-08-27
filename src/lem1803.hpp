#ifndef _LEM1803_HPP
#define _LEM1803_HPP

#include <iostream>
#include <cstdint>

#include "dcpu.hpp" // Base class: cpu::IHardware

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Texture.hpp>


#define MEM_MAP_SCREEN   0
#define MEM_MAP_FONT     1
#define MEM_MAP_PALETTE  2
#define SET_BORDER_COLOR 3
#define MEM_DUMP_FONT    4
#define MEM_DUMP_PALETTE 5
#define LEGACY_MODE      255

namespace cpu {


/**
 * @brief LEM1802 that uses SFML
 */
class Lem1803 : public cpu::IHardware {
public:
    Lem1803();
    virtual ~Lem1803();
    
    static const uint32_t id            = 0x7349f615;
    static const uint16_t revision      = 0x1803;
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

    const static uint16_t def_palette_map[64];   /// Default palette
    const static uint16_t def_font_map[512];     /// Default fontmap

private:

    uint16_t screen_map;            /// Where map VIDEO RAM 
    uint16_t font_map;              /// Where map FONT
    uint16_t palette_map;           /// Where map PALETTE
    
    uint8_t border_col;             /// Border color (unused)
    uint32_t ticks;                 /// CPU clock ticks (for timing)
    
    uint32_t tick_per_refresh;      /// How many ticks before refresh

    bool enable;                    /// Can print to stdout ?

    uint16_t blink;                 /// Counter for blinking
   
    sf::RenderWindow window;        /// SFML window
    sf::Texture texture;            /// SFML texture were to paint


};

}

#endif // _LEM1802_HPP
