#ifndef _LEM1803_HPP
#define _LEM1803_HPP

#include "lem1802.hpp"

namespace cpu {

namespace lem {

static const uint16_t LEGACY_MODE = 255;

/**
 * @brief LEM1803 that uses SFML
 */
class Lem1803 : public Lem1802 {
public:
    Lem1803();
    virtual ~Lem1803();
    
    static const uint16_t REV2     = 0x1803;

    static const unsigned int WIDTH     = 384;
    static const unsigned int HEIGHT    = 288;

    static const unsigned int ROWS      = 36;
    static const unsigned int COLS      = 96;

    virtual uint16_t getRevision() {
        return REV2;
    }
    
    virtual void handleInterrupt();
    
    virtual sf::Image* updateScreen() const;
    
    virtual sf::Color getBorder() const;

    virtual unsigned int width() const 
    { // Logical screen resolution!
        if (emulation_mode)
            return Lem1802::width();
        else
            return WIDTH;
    }

    virtual unsigned int height() const 
    {
        if (emulation_mode)
            return Lem1802::height();
        else
            return HEIGHT;
    }
    
    /**
     * @brief is the LEM1803 emulating the LEM1802 ?
     */
    bool isEmulating()
    {
        return emulation_mode;
    } 
    
    virtual unsigned int phyWidth() const {return WIDTH;}
    virtual unsigned int phyHeight() const {return HEIGHT;}

    static const uint16_t def_palette_map2[64];   /// Default palette
    static const uint16_t def_font_map2[512];     /// Default fontmap

protected:
    bool emulation_mode;


};

} // end of namespace len

} // end of namespace cpu

#endif // _LEM1802_HPP
