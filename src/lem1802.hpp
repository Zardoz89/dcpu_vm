#ifndef _LEM1802_HPP
#define _LEM1802_HPP

#include <cstdint>

#include "dcpu.hpp" // Base class: cpu::IHardware

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Image.hpp>

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
class Lem1802 : public cpu::IHardware {
public:
    Lem1802();
    virtual ~Lem1802();
    
    static const uint32_t id                = 0x7349f615;
    static const uint16_t revision          = 0x1802;
    static const uint32_t manufacturer      = 0x1c6c8b36;

    static const unsigned int WIDTH         = 128;
    static const unsigned int HEIGHT        = 96;

    static const unsigned int ROWS          = 12;
    static const unsigned int COLS          = 32;

    static const unsigned int BORDER_SIZE   = 10;

    static const int FPS                    = 30;
    static const uint16_t BLINKPERSECOND    = 2;

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
     * @brief Updates the screen texture
     */
    virtual void show();

    /**
     * @brief Return the screen array representation
     */
    const sf::Image& getScreen() const
    {
        return this->screen;
    }

    /**
     * @brief Return border color
     */
    virtual sf::Color getBorder();

    static const int scaleX = 3;
    static const int scaleY = 3;
    static const int videoWidth = Lem1802::WIDTH * scaleX;
    static const int videoHeight = Lem1802::HEIGHT * scaleY;

    virtual int getScaleX() {return scaleX;}
    virtual int getScaleY() {return scaleY;}
    virtual int getVideoWidth() {return videoWidth + BORDER_SIZE*2;}
    virtual int getVideoHeight() {return videoHeight + BORDER_SIZE*2;}
    virtual int getWidth() {return HEIGHT;}
    virtual int getHeight() {return WIDTH;}
    int getBorderSize() {return BORDER_SIZE;}

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
    uint32_t blink_max;             /// Max ticks to change blink state
  
    sf::Image screen;       /// SFML compatible array representation of screen

};

} // end of namespace lem

} // end of namespace cpu

#endif // _LEM1802_HPP
