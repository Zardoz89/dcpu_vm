#ifndef _LEM1803_HPP
#define _LEM1803_HPP

#include <dcpu/devices/lem1802.hpp>

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
    

    static const unsigned int WIDTH     = 384;
    static const unsigned int HEIGHT    = 288;

    static const unsigned int ROWS      = 36;
    static const unsigned int COLS      = 96;

    virtual uint16_t getRevision() const {
        return 0x1803;
    }
    
    virtual unsigned handleInterrupt();
    
    virtual void updateScreen();
    
    virtual uint32_t getBorder() const;
    
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
