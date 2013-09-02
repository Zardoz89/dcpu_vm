#ifndef _CGM_HPP
#define _CGM_HPP

#include <cstdint>

#include "dcpu.hpp" // Base class: cpu::IHardware

#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Texture.hpp>

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

/**
 * @brief CGM 1084 monitor that uses SFML
 */
class CGM : public cpu::IHardware {
public:
    CGM();
    virtual ~CGM();
                                          
    static const uint32_t id                = 0x7349043C;
    static const uint16_t revision          = 0x1084;
    static const uint32_t manufacturer      = 0x0ca0fe84;

    static const unsigned int WIDTH         = 256;
    static const unsigned int HEIGHT        = 192;

    static const unsigned int ROWS[3]; 
    static const unsigned int COLS[3]; 
    
    static const unsigned int BORDER_SIZE   = 10;
    
    static const int FPS                    = 30;
    static const uint16_t BLINKPERSECOND    = 10;

    uint32_t getId() {
        return id;
    }
    virtual uint16_t getRevision() {
        return revision;
    }
    uint32_t getManufacturer() {
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

    static constexpr float scaleX = 1.5;
    static constexpr float scaleY = 1.5;
    static const int videoWidth = (CGM::WIDTH * scaleX);
    static const int videoHeight = (CGM::HEIGHT * scaleY);

    virtual int getScaleX() {return scaleX;}
    virtual int getScaleY() {return scaleY;}
    virtual int getVideoWidth() {return videoWidth + BORDER_SIZE*2;}
    virtual int getVideoHeight() {return videoHeight + BORDER_SIZE*2;}
    virtual int getWidth() {return HEIGHT;}
    virtual int getHeight() {return WIDTH;}
    int getBorderSize() {return BORDER_SIZE;}
    
    /**
     * @brief Sets if it can display
     */
    void setEnable(bool enable);

    const static uint16_t def_palette_map[64];          /// Default palette
    const static uint16_t def_fonts[256*2 + 256*4];     /// Font maps

protected:

    uint16_t bitfield_map;          /// Where map BIT-FIELD 
    uint16_t attribute_map;         /// Where map ATTRIBUTE CELLS 
    uint16_t palette_map;           /// Where map PALETTE
    uint16_t font_map;              /// Where map FONT in text modex
    uint16_t videomode;             /// Video mode
    
    uint8_t border_col;             /// Border color (unused)
    uint_fast32_t ticks;            /// CPU clock ticks (for timing)
    
    uint_fast32_t tick_per_refresh; /// How many ticks before refresh

    bool enable;                    /// Can print to stdout ?

    uint_fast32_t blink;            /// Counter for blinking
    uint_fast32_t blink_max;        /// How many count to blink
   
    sf::Image screen;       /// SFML compatible array representation of screen

};

} // END OF cgm NAMESPACE

} // END OF cpu NAMESPACE

#endif // _CGM_HPP
