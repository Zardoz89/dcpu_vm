#include <devices/lem1803.hpp>

#include <algorithm>

namespace cpu {

namespace lem {

const uint16_t Lem1803::def_font_map2[256*2] = {   /// Default font map
#   include "lem1802_font.inc"
    // TODO Upper half of 8-bit Extended ASCII
};

const uint16_t Lem1803::def_palette_map2[64] = {    /// Default palette
#   include "64_palette.inc"
};


Lem1803::Lem1803() : Lem1802(), emulation_mode(true) 
{ }

Lem1803::~Lem1803() 
{ }

unsigned Lem1803::handleInterrupt()
{
    if (this->cpu == NULL)
        return 0;

    if (cpu->getA() == LEGACY_MODE) {
        emulation_mode = !emulation_mode;
        font_map = palette_map = screen_map = 0;
        blink = 0;
        if (emulation_mode) {
            screen.create(Lem1802::WIDTH,Lem1802::HEIGHT,sf::Color::Black);
        } else {
            screen.create(Lem1803::WIDTH,Lem1803::HEIGHT,sf::Color::Black);
        }
        return 0;
    } 

    size_t s;
    if (!emulation_mode) { // Only this commands are diferent 
        if (cpu->getA() == MEM_DUMP_FONT) {
            s = RAM_SIZE - 1 - cpu->getB() < 512 ? 
                    RAM_SIZE - 1 - cpu->getB() : 512 ;
            std::copy_n (Lem1803::def_font_map2, s, cpu->getMem() + cpu->getB() );
            return 0 ;
        } else if (cpu->getA() == MEM_DUMP_PALETTE) {
            s = RAM_SIZE - 1 - cpu->getB() < 64 ?
                    RAM_SIZE - 1 - cpu->getB() : 64 ;
            std::copy_n (Lem1803::def_palette_map2, s, 
                    cpu->getMem() + cpu->getB() );
            return 0;
        }
    }
    return Lem1802::handleInterrupt();
}


void Lem1803::updateScreen()
{

    if (this->cpu == NULL || !need_render)
        return;
    if (emulation_mode) {
        Lem1802::updateScreen();
        return;
    }
    need_render = false;
    if (screen_map != 0) { // Update the texture
        for (unsigned row=0; row < Lem1803::ROWS; row++) {
            for (unsigned col=0; col < Lem1803::COLS; col++) {
                uint16_t pos = screen_map + row * (Lem1803::COLS/2) + (col/2);
                uint16_t pos_attr = screen_map + 1728 + row*Lem1803::COLS + col;
                // Every word contains two characters
                unsigned char ascii;
                if (col%2 == 0) {  
                    ascii = (unsigned char) (cpu->getMem()[pos] & 0x00FF);
                } else {
                    ascii = (unsigned char) ((cpu->getMem()[pos] & 0xFF00) >> 8);
                }
                
                // Get palette indexes
                uint16_t fg_ind = (cpu->getMem()[pos_attr] & 0x0FC0) >> 6;
                uint16_t bg_ind = (cpu->getMem()[pos_attr] & 0x003F);
                uint16_t fg_col, bg_col;
                
                if (palette_map == 0) { // Use default palette
                    fg_col = Lem1803::def_palette_map2[fg_ind];
                    bg_col = Lem1803::def_palette_map2[bg_ind];
                } else {
                    fg_col = cpu->getMem()[palette_map+ fg_ind];
                    bg_col = cpu->getMem()[palette_map+ bg_ind];
                }
                
                // Does the blink
                if (blink > blink_max &&  
                       ((cpu->getMem()[pos_attr] & 0x1000) > 0) ) {
                    fg_col = bg_col;
                }
                
                // Composes RGBA values from palette colors
                sf::Color fg (
                        ((fg_col & 0x7C00)>> 10) *8,
                        ((fg_col & 0x03E0)>> 5)  *8,
                         (fg_col & 0x001F)       *8,
                        0xFF );
                sf::Color bg (
                        ((bg_col & 0x7C00)>> 10) *8,
                        ((bg_col & 0x03E0)>> 5)  *8,
                         (bg_col & 0x001F)       *8,
                        0xFF );

                uint16_t glyph[2];
                if (font_map == 0) { // Default font
                    glyph[0] = Lem1803::def_font_map2[ascii*2]; 
                    glyph[1] = Lem1803::def_font_map2[ascii*2+1];
                } else {
                    glyph[0] = cpu->getMem()[font_map+ (ascii*2)]; 
                    glyph[1] = cpu->getMem()[font_map+ (ascii*2)+1]; 
                }
                
                for (int i=0; i< 8; i++) { 
                    // *** MSB ***
                    // First word 
                    bool pixel = ((1<<(i+8)) & glyph[0]) > 0;
                    if (pixel) {
                        screen.setPixel (col*4, row*8 +i, fg);
                    } else {
                        screen.setPixel (col*4, row*8 +i, bg);
                    }
                    // Second word
                    pixel = ((1<<(i+8)) & glyph[1]) > 0;
                    if (pixel) {
                        screen.setPixel (col*4 +2, row*8 +i, fg);
                    } else {
                        screen.setPixel (col*4 +2, row*8 +i, bg);
                    }
                
                    // *** LSB ***
                    // First word 
                    pixel = ((1<<i) & glyph[0]) >0;
                    if (pixel) {
                        screen.setPixel (col*4 +1, row*8 +i, fg);
                    } else {
                        screen.setPixel (col*4 +1, row*8 +i, bg);
                    }
                    // Second word
                    pixel = ((1<<i) & glyph[1]) > 0;
                    if (pixel) {
                        screen.setPixel (col*4 +3, row*8 +i, fg);
                    } else {
                        screen.setPixel (col*4 +3, row*8 +i, bg);
                    }
                }
            }
        }
    } 
}

sf::Color Lem1803::getBorder() const
{
    if (emulation_mode)
        return Lem1802::getBorder();

    uint16_t border;
    if (palette_map == 0) { // Use default palette
        border = Lem1803::def_palette_map[border_col];
    } else {
        border = cpu->getMem()[palette_map+ border_col];
    }
    return sf::Color(
                (sf::Uint8)(((border & 0x7C00)>> 10) *8),
                (sf::Uint8)(((border & 0x03E0)>> 5)  *8),
                (sf::Uint8)( (border & 0x001F)       *8),
                0xFF );
}

} // END OF NAMESPACE lem

} // END of NAMESPACE cpu