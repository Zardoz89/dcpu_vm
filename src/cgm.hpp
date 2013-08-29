#ifndef _CGM_HPP
#define _CGM_HPP

#include <thread>
#include <cstdint>

#include "dcpu.hpp" // Base class: cpu::IHardware

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Texture.hpp>

namespace cpu {

namespace cgm {

enum COMMANDS {
    MEM_BITPLANE_SCREEN,
    MEM_ATTRIBUTE_SCREEN,
    MEM_MAP_PALETTE,
    SET_BORDER_COLOR,
    SET_VIDEO_MODE,
    GET_VIDEO_MODE,
    MEM_DUMP_PALETTE,
    MEM_DUMP_ASCII_FONT,
};

/**
 * @brief CGM 1084 monitor that uses SFML
 */
class CGM : public cpu::IHardware {
public:
    CGM();
    virtual ~CGM();
                                          
    static const uint32_t id            = 0x0340043c;
    static const uint16_t revision      = 0x1084;
    static const uint32_t manufacturer  = 0x0ca0fe84;

    static const int FPS                = 30;
    static const unsigned int WIDTH     = 256;
    static const unsigned int HEIGHT    = 192;

    static const unsigned int ROWS[3]; 
    static const unsigned int COLS[3]; 
    
    // Frac of second that blinks
    static const uint16_t BLINKTIME     = 10;

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
     * @brief Sets if it can display to stdout
     */
    void setEnable(bool enable);

    const static uint16_t def_palette_map[64];          /// Default palette
    const static uint16_t font_maps[256*2 + 256*4];  /// Font maps

protected:

    uint16_t bitfield_map;          /// Where map BIT-FIELD 
    uint16_t attribute_map;         /// Where map ATTRIBUTE CELLS 
    uint16_t palette_map;           /// Where map PALETTE
    uint16_t videomode;             /// Video mode

    
    uint8_t border_col;             /// Border color (unused)
    uint_fast32_t ticks;                 /// CPU clock ticks (for timing)
    
    uint_fast32_t tick_per_refresh;      /// How many ticks before refresh

    bool enable;                    /// Can print to stdout ?

    uint_fast32_t blink;            /// Counter for blinking
    uint_fast32_t blink_max;        /// How many count to blink
   
    sf::RenderWindow window;        /// SFML window
    sf::Texture texture;            /// SFML texture were to paint

    std::string title;              /// Title window
    std::thread renderguy;          /// Rendered thread

    virtual void render();          /// Renders the screen to the window

};

} // END OF cgm NAMESPACE

} // END OF cpu NAMESPACE

#endif // _CGM_HPP
