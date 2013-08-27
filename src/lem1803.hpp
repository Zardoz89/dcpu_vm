#ifndef _LEM1803_HPP
#define _LEM1803_HPP

#include <iostream>
#include <cstdint>

#include "lem1802.hpp"

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Texture.hpp>


#define LEGACY_MODE      255

namespace cpu {


/**
 * @brief LEM1802 that uses SFML
 */
class Lem1803 : public Lem1802 {
public:
    Lem1803();
    virtual ~Lem1803();
    
    static const uint16_t revision2     = 0x1803;

    static const unsigned int WIDTH     = 384;
    static const unsigned int HEIGHT    = 288;

    static const unsigned int ROWS      = 36;
    static const unsigned int COLS      = 96;

    virtual uint16_t getRevision() {
        return revision2;
    }
    
    virtual void handleInterrupt();
    
    virtual void attachTo (DCPU* cpu, size_t index);

    virtual void show();
    
    const static uint16_t def_palette_map2[64];   /// Default palette
    const static uint16_t def_font_map2[512];     /// Default fontmap

protected:
    bool emulation_mode;

};

}

#endif // _LEM1802_HPP
