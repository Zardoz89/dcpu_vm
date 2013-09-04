#ifndef _LEM1802_HPP
#define _LEM1802_HPP
#include "config.hpp"

#include <cstdint>

#include "dcpu.hpp" 
#include "monitor.hpp" //base class AbstractMonitor
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>


namespace cpu {

namespace lem {
static const uint16_t MEM_MAP_SCREEN        = 0;
static const uint16_t MEM_MAP_FONT          = 1;
static const uint16_t MEM_MAP_PALETTE       = 2;
static const uint16_t SET_BORDER_COLOR      = 3;
static const uint16_t MEM_DUMP_FONT         = 4;
static const uint16_t MEM_DUMP_PALETTE      = 5;

/**
 * @brief LEM1802 that uses SFML
 */
class Lem1802 : public cpu::AbstractMonitor {
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

    static const uint16_t BLINKRATE    = 20000; // Change Blink state each N ticks

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
    
    /**
     * @The screen must at least size WIDTH*HEIGHT*4 rgba
     */
    virtual void initScreen()
    {
      if (screen)
        delete screen;
      screen = new uint8_t[Lem1802::WIDTH*Lem1802::HEIGHT*4];
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
    
    virtual const sf::Texture& getScreen()
    {
       return texture;
    }
    
    virtual unsigned int width() const
    {
        return texture.getSize().x;
    }
    
    virtual unsigned int height() const
    {
        return texture.getSize().y;
    }

    virtual unsigned int phyWidth() const
    {
        return Lem1802::WIDTH;
    }
    
    virtual unsigned int phyHeight() const
    {
        return Lem1802::HEIGHT;
    }
    
    virtual sf::Color getBorder() const;

protected:

    uint16_t screen_map;              /// Where map VIDEO RAM 
    uint16_t font_map;                /// Where map FONT
    uint16_t palette_map;             /// Where map PALETTE
    
    uint8_t border_col;               /// Border color (unused)
    uint32_t ticks;                   /// CPU clock ticks (for timing)
    
    uint32_t tick_per_refresh;        /// How many ticks before refresh

    bool enable;                      /// Can print to stdout ?

    uint16_t blink;                   /// Counter for blinking
   
    sf::Texture texture;              /// SFML screen texture
    
    
    
    // Threads and windows are not used anymore because 
    // Win32 portability forbit window event in threads !
    // It more logical that the following class just create the texture...
    
    /*
    #ifdef __NO_THREAD_11__
    sf::Thread* renderguy;            /// Rendered thread for mingw 4.7
    #else
    std::thread renderguy;            /// Rendered thread
    #endif
    */
    uint8_t* screen;
};

}//end of lem NAMESPACE

}//end of cpu NAMESPACE

#endif // _LEM1802_HPP
