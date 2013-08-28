#ifndef _LEM1802_HPP
#define _LEM1802_HPP

#include <thread>
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

namespace cpu {

/**
 * @brief LEM1802 that uses SFML
 */
class Lem1802 : public cpu::IHardware {
public:
    Lem1802();
    virtual ~Lem1802();
    
    static const uint32_t id            = 0x7349f615;
    static const uint16_t revision      = 0x1802;
    static const uint32_t manufacturer  = 0x1c6c8b36;

    static const int FPS                = 30;
    static const unsigned int WIDTH     = 128;
    static const unsigned int HEIGHT    = 96;

    static const unsigned int ROWS      = 12;
    static const unsigned int COLS      = 32;

    static const uint16_t BLINKRATE    = 10000; // Change Blink state each N ticks

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
    virtual void show();

    /**
     * @brief Sets if it can display to stdout
     */
    void setEnable(bool enable);

    const static uint16_t def_palette_map[16];   /// Default palette
    const static uint16_t def_font_map[128*2];   /// Default fontmap

protected:

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

    std::string title;              /// Title window
    std::thread renderguy;          /// Rendered thread

    virtual void render();          /// Renders the screen to the window

};

}

#endif // _LEM1802_HPP
