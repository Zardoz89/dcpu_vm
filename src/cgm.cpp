#include "cgm.hpp"

#include <algorithm>
#include <cctype>
#include <string>


namespace cpu {

namespace cgm {

    const uint16_t CGM::def_fonts[256*2 + 256*4] = {  /// Font maps
#       include "lem1802_font.inc"
        // TODO Upper half of 8-bit Extended ASCII to 4x8 font
        // TODO Add font for 8x8
    }; 

    const  uint16_t CGM::def_palette_map[64] = {    /// Default palette
#       include "64_palette.inc"
    };
   
    const unsigned int CGM::ROWS[3] = {24, 24, 96};
    const unsigned int CGM::COLS[3] = {64, 32, 32};

    CGM::CGM() : 
        bitfield_map (0), attribute_map (0), palette_map (0), font_map (0),
        videomode(0), border_col (0), ticks (0), enable (true), blink(0) 
    { }

    CGM::~CGM() 
    { }

    void CGM::attachTo (DCPU* cpu, size_t index) 
    {
        this->IHardware::attachTo(cpu, index);

        tick_per_refresh = cpu->cpu_clock / CGM::FPS;
        blink_max = cpu->cpu_clock / CGM::BLINKPERSECOND;

        videomode = 0; // Mode 0

        screen.create(CGM::WIDTH, CGM::HEIGHT, sf::Color::Black);
    }

    void CGM::handleInterrupt()
    {
        if (this->cpu == NULL)
            return;

        size_t s;
        switch (cpu->GetA() ) {
        case MEM_BITPLANE_SCREEN:
            if (bitfield_map == 0 && attribute_map != 0 && cpu->GetB() != 0) {
                ticks = tick_per_refresh +1; // Force to do initial print
                videomode = 0; 
            }
            bitfield_map = cpu->GetB();
            break;

        case MEM_ATTRIBUTE_SCREEN:
            if (bitfield_map != 0 && attribute_map == 0 && cpu->GetB() != 0) {
                ticks = tick_per_refresh +1; // Force to do initial print
                videomode = 0;
            }
            attribute_map = cpu->GetB();
            break;

        case MEM_MAP_PALETTE:
            palette_map = cpu->GetB();
            break;

        case SET_BORDER_COLOR:
            border_col = cpu->GetB() & 0x1F;
            break;

        case SET_VIDEO_MODE:
            videomode = cpu->GetB() & 3;
            break;

        case GET_VIDEO_MODE:
            cpu->SetB(videomode);
            break;

        case MEM_DUMP_PALETTE:
            s = RAM_SIZE - 1 - cpu->GetB() < 16 ?
                    RAM_SIZE - 1 - cpu->GetB() : 16 ;
            std::copy_n (CGM:: def_palette_map, s, 
                    cpu->getMem() + cpu->GetB() );
            break;
        
        case MEM_DUMP_FONT:
            if (videomode == 4) {       // 4x8 fonts
                s = RAM_SIZE - 1 - cpu->GetB() < 512 ? 
                    RAM_SIZE - 1 - cpu->GetB() : 512 ;
                std::copy_n (CGM::def_fonts, s, 
                        cpu->getMem() + cpu->GetB() );
            } else if (videomode == 5) { // 8x8 fonts
                s = RAM_SIZE - 1 - cpu->GetB() < 512*2 ? 
                    RAM_SIZE - 1 - cpu->GetB() : 512*2 ;
                std::copy_n (&CGM::def_fonts[512], s, 
                        cpu->getMem() + cpu->GetB() );
            }
            break;

        case MEM_MAP_FONT:
            font_map = cpu->GetB();
            break;

        default:
            // do nothing
            ;
        }
    }

    void CGM::tick()
    {
        if (++ticks > tick_per_refresh) {
            // Update screen at desired FPS
            ticks = 0;
            this->show();
        }
        if (++blink > blink_max*2)
            blink = 0;
    }

    void CGM::show()
    {
        if (this->cpu == NULL)
            return;
        
        if (bitfield_map != 0 && attribute_map != 0  && enable) { 
            // Update the texture
            switch (videomode) {
            case 0: // Mode 0 256x192-64x24 cells of 4x8 pixels. 64 colors
            
                for (unsigned i=0; i < CGM::WIDTH * CGM::HEIGHT; i++) {
                    unsigned row = i >> 3; // /8
                    unsigned col = i % 4*CGM::COLS[videomode];

                    uint16_t attr_pos = row * CGM::COLS[0] + col;
                    attr_pos += attribute_map;
                    // Get palette indexes and other attributes
                    uint16_t fg_ind = (cpu->getMem()[attr_pos] & 0x0FC0) >> 6;
                    uint16_t bg_ind = (cpu->getMem()[attr_pos] & 0x003F);

                    uint16_t fg_col, bg_col;
                    if (palette_map == 0) { // Use default palette
                        fg_col = CGM::def_palette_map[fg_ind];
                        bg_col = CGM::def_palette_map[bg_ind];
                    } else {
                        fg_col = cpu->getMem()[palette_map+ fg_ind];
                        bg_col = cpu->getMem()[palette_map+ bg_ind];
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

                    auto bit = 15 - (i % 16); // From MSB to LSB
                    uint16_t word = bitfield_map + (i >> 4);
                    unsigned x = i % CGM::WIDTH;
                    unsigned y = i / CGM::WIDTH;
                    if (cpu->getMem()[word] & 1<<bit) {
                        // Foreground
                        screen.setPixel (x, y, fg);
                    } else {
                        // Backgorund
                        screen.setPixel (x, y, bg);
                    }
                }
                break;

            case 1: // 256x192-32x24 cells of 8x8 pixels. 64 colors
                // TODO Implement the rest modes
                break;

            case 2: // 256x192-16x192 cells of 16x1 pixels. 16 colors
                break;

            case 3: // 256x192 B&W. 2 colors
                break;

            case 4: // 256x192 4x8 font Text mode
                for (unsigned row=0; row < CGM::ROWS[0]; row++) {
                    for (unsigned col=0; col < CGM::COLS[0]; col++) {
                        uint16_t pos = row * (CGM::COLS[0]/2) + (col/2);
                        pos += bitfield_map;
                        uint16_t attr_pos = row * CGM::COLS[0] + row;
                        attr_pos += attribute_map;

                        // Get palette indexes and other attributes
                        uint16_t fg_ind = (cpu->getMem()[attr_pos] & 0x0FC0) 
                            >> 6;
                        uint16_t bg_ind = (cpu->getMem()[attr_pos] & 0x003F);
                        bool blinkf = (cpu->getMem()[attr_pos] & 0x1000) > 0;
                        bool underf = (cpu->getMem()[attr_pos] & 0x2000) > 0;

                        uint16_t fg_col, bg_col;
                        if (palette_map == 0) { // Use default palette
                            fg_col = CGM::def_palette_map[fg_ind];
                            bg_col = CGM::def_palette_map[bg_ind];
                        } else {
                            fg_col = cpu->getMem()[palette_map+ fg_ind];
                            bg_col = cpu->getMem()[palette_map+ bg_ind];
                        }

                        if (blink > blink_max && blinkf)
                            fg_col = bg_col;

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

                        // Every word contains two characters
                        unsigned char ascii;
                        if (col%2 == 0) {  
                            ascii = (unsigned char) (cpu->getMem()[pos] & 0x00FF);
                        } else {
                            ascii = (unsigned char) ((cpu->getMem()[pos] & 0xFF00) >> 8);
                        }
                        
                        uint16_t glyph[2];
                        if (font_map == 0) { // Default font
                            glyph[0] = CGM::def_fonts[ascii*2]; 
                            glyph[1] = CGM::def_fonts[ascii*2+1];
                        } else {
                            glyph[0] = cpu->getMem()[font_map+ (ascii*2)]; 
                            glyph[1] = cpu->getMem()[font_map+ (ascii*2)+1]; 
                        }

                        // Display it
                        for (int i=8; i< 16; i++) { // Puts MSB of Words
                            // First word 
                            bool pixel = ((1<<i) & glyph[0]) > 0;
                            if (pixel) {
                                screen.setPixel (col*4, row*8 +i-8, fg);
                            } else {
                                screen.setPixel (col*4, row*8 +i-8, bg);
                            }
                            // Second word
                            pixel = ((1<<i) & glyph[1]) > 0;
                            if (pixel) {
                                screen.setPixel (col*4 +2, row*8 +i-8, fg);
                            } else {
                                screen.setPixel (col*4 +2, row*8 +i-8, bg);
                            }
                        }

                        for (int i=0; i< 8; i++) { // Puts LSB of Words
                            // First word 
                            bool pixel = ((1<<i) & glyph[0]) >0;
                            if (pixel) {
                                screen.setPixel (col*4 +1, row*8 +i, fg);
                            } else {
                                screen.setPixel (col*4 +1, row*8 +i, bg);
                            }
                            // Secodn word
                            pixel = ((1<<i) & glyph[1]) > 0;
                            if (pixel) {
                                screen.setPixel (col*4 +3, row*8 +i, fg);
                            } else {
                                screen.setPixel (col*4 +3, row*8 +i, bg);
                            }
                        }

                        if (underf) { // Underline, puts last row to ON
                            for (int i=0; i<4; i++)
                                screen.setPixel (col*4 +i, row*8 +8, fg);
                        }

                    }
                }
                break;
            
            case 5: // 256x192 8x8 font Text mode
                break;
            default:
                ; 
            }
            
            
        
        } else { // Not active blank screen
            screen.create(CGM::WIDTH, CGM::HEIGHT, sf::Color::Black);
        }
    }


    void CGM::setEnable(bool enable) 
    {
        this->enable = enable;
    }

    sf::Color CGM::getBorder()
    {
        uint16_t border;
        if (palette_map == 0) { // Use default palette
            border = CGM::def_palette_map[border_col];
        } else {
            border = cpu->getMem()[palette_map+ border_col];
        }
        return sf::Color(
                    (sf::Uint8)(((border & 0x7C00)>> 10) *8),
                    (sf::Uint8)(((border & 0x03E0)>> 5)  *8),
                    (sf::Uint8)( (border & 0x001F)       *8),
                    0xFF );
    }

} // END of NAMESPACE cgm

} // END of NAMESPACE cpu
