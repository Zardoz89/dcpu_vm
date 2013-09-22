#include <dcpu/devices/lem1802.hpp>

#include <algorithm>

namespace cpu {

namespace lem {

const uint16_t Lem1802::def_font_map[128*2] = {     /// Default font map
#   include "lem1802_font.inc"
};

const uint16_t Lem1802::def_palette_map[16] = {     /// Default palette
#   include "lem1802_palette.inc"
};

Lem1802::Lem1802() : screen_map (0), font_map (0), palette_map (0),
                     border_col (0), ticks(0), blink(0) {
    _width = Lem1802::WIDTH;
    _height = Lem1802::HEIGHT;
    pixels = new uint8_t[4 * _width * _height]();
}

Lem1802::~Lem1802() 
{
    delete[] pixels;
}

void Lem1802::attachTo (DCPU* cpu, size_t index) 
{
    this->IHardware::attachTo(cpu, index);

    blink_max = cpu->getClock() / Lem1802::BLINKPERSECOND;
    blink = 0;

    screen_map = font_map, palette_map = 0;
    border_col = 0;
}

unsigned Lem1802::handleInterrupt()
{
    if (this->cpu == NULL)
        return 0;
    
    size_t s;
    switch (cpu->getA() ) {
        case MEM_MAP_SCREEN:
            if (screen_map == 0 && cpu->getB() != 0) {
                splash = true;
                splashtime = cpu->getClock() * SPLASHTIME;
            }

            screen_map = cpu->getB();
            powered = screen_map != 0x0000;
            break;

        case MEM_MAP_FONT:
            font_map = cpu->getB();
            break;

        case MEM_MAP_PALETTE:
            palette_map = cpu->getB();
            break;

        case SET_BORDER_COLOR:
            border_col = cpu->getB() & 0xf;
            break;

        case MEM_DUMP_FONT:
            s = RAM_SIZE - 1 - cpu->getB() < 256 ? 
                    RAM_SIZE - 1 - cpu->getB() : 256 ;
            std::copy_n (Lem1802::def_font_map,s,cpu->getMem()+cpu->getB());
            break;

        case MEM_DUMP_PALETTE:
            s = RAM_SIZE - 1 - cpu->getB() < 16 ?
                    RAM_SIZE - 1 - cpu->getB() : 16 ;
            std::copy_n (Lem1802::def_palette_map, s, 
                    cpu->getMem() + cpu->getB() );
            break;

        default:
            // do nothing
            break;
    }
    return 0;
}

void Lem1802::tick()
{
    if (this->cpu == NULL) return;

    if (++ticks > cpu->getClock() /REFRESHRATE) {
        // Update screen at 50Hz aprox
        ticks -= cpu->getClock() / REFRESHRATE;
        this->updateScreen();
    }

    if (splash && splashtime-- == 0)
        splash = false;

    if (++blink > (blink_max << 1))
        blink -= blink_max << 1;
}

void Lem1802::updateScreen()
{
    if (this->cpu == NULL || !need_render)
        return;

    need_render = false;
    
    
    //4 pixel per col 4 value per pixels
    if (screen_map != 0) { // Update the texture
        uint8_t* pixel_pos = pixels;
        const unsigned row_pixel_size = Lem1802::WIDTH*4; 
        const unsigned row_pixel_size_7 = row_pixel_size*7; 
        for (unsigned row=0; row < Lem1802::ROWS; ++row) {
            for (unsigned col=0; col < Lem1802::COLS; ++col) {
                uint16_t pos = screen_map + row * Lem1802::COLS + col;
                unsigned char ascii = (unsigned char) (cpu->getMem()[pos] 
                                                        & 0x007F);
                // Get palette indexes
                uint16_t fg_ind = (cpu->getMem()[pos] & 0xF000) >> 12;
                uint16_t bg_ind = (cpu->getMem()[pos] & 0x0F00) >> 8;
                uint16_t fg_col, bg_col;

                if (palette_map == 0) { // Use default palette
                    fg_col = Lem1802::def_palette_map[fg_ind &0xF];
                    bg_col = Lem1802::def_palette_map[bg_ind &0xF];
                } else {
                    fg_col = cpu->getMem()[palette_map+ (fg_ind &0xF)];
                    bg_col = cpu->getMem()[palette_map+ (bg_ind &0xF)];
                }
                
                // Does the blink
                if (blink > blink_max &&  
                       ((cpu->getMem()[pos] & 0x80) > 0) ) {
                    fg_col = bg_col;
                }

                // Composes RGBA values from palette colors
                uint8_t fg[] = {
                    (uint8_t)(((fg_col & 0xF00)>> 8) *0x11),
                    (uint8_t)(((fg_col & 0x0F0)>> 4) *0x11),
                    (uint8_t)( (fg_col & 0x00F)      *0x11),
                    0xFF };
                uint8_t bg[] = {
                    (uint8_t)(((bg_col & 0xF00)>> 8) *0x11),
                    (uint8_t)(((bg_col & 0x0F0)>> 4) *0x11),
                    (uint8_t)( (bg_col & 0x00F)      *0x11),
                    0xFF };

                uint16_t glyph[2];
                if (font_map == 0) { // Default font
                    glyph[0] = Lem1802::def_font_map[ascii*2]; 
                    glyph[1] = Lem1802::def_font_map[ascii*2+1];
                } else {
                    glyph[0] = cpu->getMem()[font_map+ (ascii*2)]; 
                    glyph[1] = cpu->getMem()[font_map+ (ascii*2)+1]; 
                }
                uint8_t* current_pixel_pos = pixel_pos;
                for (int i=8; i< 16; ++i) { // Puts MSB of Words
                    // First word 
                    bool pixel = ((1<<i) & glyph[0]) > 0;
                    if (pixel) {
                        *(current_pixel_pos+0) = fg[0];
                        *(current_pixel_pos+1) = fg[1];
                        *(current_pixel_pos+2) = fg[2];
                        *(current_pixel_pos+3) = fg[3];
                    } else {
                        *(current_pixel_pos+0) = bg[0];
                        *(current_pixel_pos+1) = bg[1];
                        *(current_pixel_pos+2) = bg[2];
                        *(current_pixel_pos+3) = bg[3];
                    }
                    // Second word
                    pixel = ((1<<i) & glyph[1]) > 0;
                    if (pixel) {
                        *(current_pixel_pos+0+2*4) = fg[0];
                        *(current_pixel_pos+1+2*4) = fg[1];
                        *(current_pixel_pos+2+2*4) = fg[2];
                        *(current_pixel_pos+3+2*4) = fg[3];
                    } else {
                        *(current_pixel_pos+0+2*4) = bg[0];
                        *(current_pixel_pos+1+2*4) = bg[1];
                        *(current_pixel_pos+2+2*4) = bg[2];
                        *(current_pixel_pos+3+2*4) = bg[3];
                    }
                    current_pixel_pos += row_pixel_size;
                }
                current_pixel_pos = pixel_pos;

                for (int i=0; i< 8; ++i) { // Puts LSB of Words
                    // First word 
                    bool pixel = ((1<<i) & glyph[0]) >0;
                    if (pixel) {
                        *(current_pixel_pos+0+1*4) = fg[0];
                        *(current_pixel_pos+1+1*4) = fg[1];
                        *(current_pixel_pos+2+1*4) = fg[2];
                        *(current_pixel_pos+3+1*4) = fg[3];
                    } else {
                        *(current_pixel_pos+0+1*4) = bg[0];
                        *(current_pixel_pos+1+1*4) = bg[1];
                        *(current_pixel_pos+2+1*4) = bg[2];
                        *(current_pixel_pos+3+1*4) = bg[3];
                    }
                    // Secodn word
                    pixel = ((1<<i) & glyph[1]) > 0;
                    if (pixel) {
                        *(current_pixel_pos+0+3*4) = fg[0];
                        *(current_pixel_pos+1+3*4) = fg[1];
                        *(current_pixel_pos+2+3*4) = fg[2];
                        *(current_pixel_pos+3+3*4) = fg[3];
                    } else {
                        *(current_pixel_pos+0+3*4) = bg[0];
                        *(current_pixel_pos+1+3*4) = bg[1];
                        *(current_pixel_pos+2+3*4) = bg[2];
                        *(current_pixel_pos+3+3*4) = bg[3];
                    }
                    current_pixel_pos += row_pixel_size;
                }
                pixel_pos+=16;
            }
            pixel_pos+=row_pixel_size_7;
        }
    }
    
    ///Non Optimised
    /// Leave this yet : if you implement something implement on
    /// this code then optimise !!!!
    /*if (screen_map != 0) { // Update the texture

        for (unsigned row=0; row < Lem1802::ROWS; row++) {
            uint16_t row_offset = row * Lem1802::COLS;
            auto r8 = row*8;
            for (unsigned col=0; col < Lem1802::COLS; col++) {
                uint16_t pos = screen_map + row_offset + col;
                unsigned char ascii = (unsigned char) (cpu->getMem()[pos] 
                                                        & 0x007F);
                // Get palette indexes
                uint16_t fg_ind = (cpu->getMem()[pos] & 0xF000) >> 12;
                uint16_t bg_ind = (cpu->getMem()[pos] & 0x0F00) >> 8;
                uint16_t fg_col, bg_col;

                if (palette_map == 0) { // Use default palette
                    fg_col = Lem1802::def_palette_map[fg_ind &0xF];
                    bg_col = Lem1802::def_palette_map[bg_ind &0xF];
                } else {
                    fg_col = cpu->getMem()[palette_map+ (fg_ind &0xF)];
                    bg_col = cpu->getMem()[palette_map+ (bg_ind &0xF)];
                }
                
                // Does the blink
                if (blink > blink_max &&  
                       ((cpu->getMem()[pos] & 0x80) > 0) ) {
                    fg_col = bg_col;
                }

                // Composes RGBA values from palette colors
                Color fg (
                        ((fg_col & 0xF00)>> 8) *0x11,
                        ((fg_col & 0x0F0)>> 4) *0x11,
                         (fg_col & 0x00F)      *0x11, // 0xF * 0x11 = 0xFF
                        0xFF );
                Color bg (
                        ((bg_col & 0xF00)>> 8) *0x11,
                        ((bg_col & 0x0F0)>> 4) *0x11,
                         (bg_col & 0x00F)      *0x11,
                        0xFF );

                uint16_t glyph[2];
                if (font_map == 0) { // Default font
                    glyph[0] = Lem1802::def_font_map[ascii<<1];
                    glyph[1] = Lem1802::def_font_map[(ascii<<1) +1];
                } else {
                    glyph[0] = cpu->getMem()[font_map + (ascii<<1)];
                    glyph[1] = cpu->getMem()[font_map + (ascii<<1) +1];
                }
                
                auto c4 = col*4;
                for (int i=0; i< 8; i++) { 
                    // *** MSB ***
                    // First word 

                    bool pixel = ((1<<(i+8)) & glyph[0]) > 0;

                    if (pixel) {
                        setPixel (c4, r8 +i, fg);
                    } else {
                        setPixel (c4, r8 +i, bg);
                    }


                    // Second word
                    pixel = ((1<<(i+8)) & glyph[1]) > 0;
                    if (pixel) {
                        setPixel (c4 +2, r8 +i, fg);
                    } else {
                        setPixel (c4 +2, r8 +i, bg);
                    }

                    // *** LSB ***
                    // First word 
                    pixel = ((1<<i) & glyph[0]) >0;
                    if (pixel) {
                        setPixel (c4 +1, r8 +i, fg);
                    } else {
                        setPixel (c4 +1, r8 +i, bg);
                    }
                    // Second word
                    pixel = ((1<<i) & glyph[1]) > 0;
                    if (pixel) {
                        setPixel (c4 +3, r8 +i, fg);
                    } else {
                        setPixel (c4 +3, r8 +i, bg);
                    }
                }
            }
        }
    }*/
}

uint32_t Lem1802::getBorder() const
{
    uint16_t border;
    if (palette_map == 0) { // Use default palette
        border = Lem1802::def_palette_map[border_col];
    } else {
        border = cpu->getMem()[palette_map+ border_col];
    }
    return  (((((border &0x0F00) >> 8) *0x11) & 0xFF) << 24) |
            (((((border &0x00F0) >> 4) *0x11) & 0xFF) << 16) |
            (((((border &0x000F) ) *0x11) & 0xFF) << 8) |
            (0xFF );
}

} // END of NAMESPACE lem

} // END of NAMESPACE cpu
