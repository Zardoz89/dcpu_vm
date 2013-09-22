#include <dcpu/devices/cgm.hpp>

#include <algorithm>
#include <cctype>

namespace cpu {

namespace cgm {

const uint16_t CGM::def_fonts[256*2 + 256*4] = {  /// Font maps
#   include "lem1802_font.inc"
#   include "lem1802_font.inc" // TODO Upper half of 8-bit Extended ASCII
    // 8x8 Font from 256*2
#   include "cgm8x8font.inc"
}; 

const  uint16_t CGM::def_palette_map[64] = {    /// Default palette
#   include "64_palette.inc"
};

const unsigned int CGM::ROWS[] = {24, 24, 96, 0, 24, 24};
const unsigned int CGM::COLS[] = {64, 32, 32, 0, 64, 32};

CGM::CGM() : 
    bitfield_map (0), attribute_map (0), palette_map (0), font_map (0),
    videomode(0), border_col (0), ticks(0), blink(0) 
{
    _width = CGM::WIDTH;
    _height = CGM::HEIGHT;
    pixels = new uint8_t[4 * _width * _height]();
}

CGM::~CGM() 
{
    delete[] pixels;
}

void CGM::attachTo (DCPU* cpu, size_t index) 
{
    this->IHardware::attachTo(cpu, index);

    blink_max = cpu->getClock() / CGM::BLINKPERSECOND;
    bitfield_map = attribute_map = font_map = palette_map = 0;
    blink = 0;
    border_col = 0;
    videomode = 0; // Mode 0

}

unsigned CGM::handleInterrupt()
{
    if (this->cpu == NULL)
        return 0;

    size_t s;
    switch (cpu->getA() ) {
    case MEM_BITPLANE_SCREEN:
        if (bitfield_map == 0 && attribute_map != 0 && cpu->getB() != 0) {
            videomode = 0;
            splash = true;
            splashtime = cpu->getClock() * SPLASHTIME;
        }

        bitfield_map = cpu->getB();

        powered = (bitfield_map != 0x0000) && (attribute_map != 0x0000);
        break;

    case MEM_ATTRIBUTE_SCREEN:
        if (bitfield_map != 0 && attribute_map == 0 && cpu->getB() != 0) {
            videomode = 0;
            splash = true;
            splashtime = cpu->getClock() * SPLASHTIME;
        }

        attribute_map = cpu->getB();

        powered = (bitfield_map != 0x0000) && (attribute_map != 0x0000);
        break;

    case MEM_MAP_PALETTE:
        palette_map = cpu->getB();
        break;

    case SET_BORDER_COLOR:
        border_col = cpu->getB() & 0x1F;
        break;

    case SET_VIDEO_MODE:
        videomode = cpu->getB() & 0x7;
        LOG << "[CGM] Videomode : " << videomode;
        break;

    case GET_VIDEO_MODE:
        cpu->setB(videomode);
        break;

    case MEM_DUMP_PALETTE:
        s = RAM_SIZE - 1 - cpu->getB() < 16 ?
                RAM_SIZE - 1 - cpu->getB() : 16 ;
        std::copy_n (CGM:: def_palette_map, s, 
                cpu->getMem() + cpu->getB() );
        break;
    
    case MEM_DUMP_FONT:
        if (videomode == 4) {       // 4x8 fonts
            s = RAM_SIZE - 1 - cpu->getB() < 512 ? 
                RAM_SIZE - 1 - cpu->getB() : 512 ;
            std::copy_n (CGM::def_fonts, s, cpu->getMem() + cpu->getB() );

        } else if (videomode == 5) { // 8x8 fonts
            s = RAM_SIZE - 1 - cpu->getB() < 512*2 ? 
                RAM_SIZE - 1 - cpu->getB() : 512*2 ;
            std::copy_n (&CGM::def_fonts[512], s, cpu->getMem() + cpu->getB() );
        }
        break;

    case MEM_MAP_FONT:
        font_map = cpu->getB();
        break;

    default:
        // do nothing
        break;
    }

    return 0;
}

void CGM::tick()
{
    if (this->cpu == NULL) return;
    if (++ticks > cpu->getClock() /REFRESHRATE) {
        // Update screen at Refresh rate aprox.
        ticks -= cpu->getClock() /REFRESHRATE;
        this->updateScreen();
    }

    if (splash && splashtime-- == 0)
        splash = false;

    if (++blink > (blink_max << 1))
        blink -= blink_max << 1;
}

void CGM::updateScreen()
{
    if (this->cpu == NULL || !need_render)
        return;
    
    need_render = false;
    if (bitfield_map != 0 && attribute_map != 0) { 
        // Update the texture
        switch (videomode) {
        case 0: // Mode 0 256x192-64x24 cells of 4x8 pixels. 64 colors
            for (unsigned i=0; i < CGM::WIDTH * CGM::HEIGHT; i++) {
                unsigned col = (i /4) % (CGM::COLS[videomode]);
                unsigned row = i / (8*4*CGM::COLS[videomode]); 

                uint16_t attr_pos = row * CGM::COLS[videomode] + col;
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
                uint32_t fg = 
                        (((((fg_col & 0x7C00)>> 10) *8) & 0xFF) << 24) |
                        (((((fg_col & 0x03E0)>> 5)  *8) & 0xFF) << 16) |
                         ((((fg_col & 0x001F)       *8) & 0xFF) << 8) |
                          0xFF ;
                uint32_t bg =
                        (((((bg_col & 0x7C00)>> 10) *8) & 0xFF) << 24) |
                        (((((bg_col & 0x03E0)>> 5)  *8) & 0xFF) << 16) |
                        ((((bg_col & 0x001F)       *8) & 0xFF) << 8) |
                          0xFF ;

                auto bit = 15 - (i % 16); // From MSB to LSB
                uint16_t word = bitfield_map + (i >> 4);
                unsigned x = i % CGM::WIDTH;
                unsigned y = i / CGM::WIDTH;
                if (cpu->getMem()[word] & 1<<bit) {
                    // Foreground
                    setPixel (x, y, fg);
                } else {
                    // Background
                    setPixel (x, y, bg);
                }
            }
            break;

        case 1: // 256x192-32x24 cells of 8x8 pixels. 64 colors
            for (unsigned i=0; i < CGM::WIDTH * CGM::HEIGHT; i++) {
                unsigned col = (i /8) % (CGM::COLS[videomode]);
                unsigned row = i / (8*8*CGM::COLS[videomode]); 

                uint16_t attr_pos = row * CGM::COLS[videomode] + col;
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
                uint32_t fg = ((((fg_col & 0x7C00)>> 10) *8) << 24) |
                              (((((fg_col & 0x03E0)>> 5)  *8) & 0xFF) << 16) |
                              ((((fg_col & 0x001F)       *8) & 0xFF) << 8) |
                              0xFF ;
                uint32_t bg = 
                        ((((bg_col & 0x7C00)>> 10) *8) << 24) |
                        (((((bg_col & 0x03E0)>> 5)  *8) & 0xFF) << 16) |
                        ((( (bg_col & 0x001F)       *8) & 0xFF) << 8)  | 
                        0xFF;

                auto bit = 15 - (i % 16); // From MSB to LSB
                uint16_t word = bitfield_map + (i >> 4);
                unsigned x = i % CGM::WIDTH;
                unsigned y = i / CGM::WIDTH;
                if (cpu->getMem()[word] & 1<<bit) {
                    // Foreground
                    setPixel (x, y, fg);
                } else {
                    // Background
                    setPixel (x, y, bg);
                }
            }
            break;

        case 2: // 256x192-16x192 cells of 8x2 pixels. 64 colors
            for (unsigned i=0; i < CGM::WIDTH * CGM::HEIGHT; i++) {
                unsigned col = (i /8) % (CGM::COLS[videomode]);
                unsigned row = i / (2*8*CGM::COLS[videomode]); 

                uint16_t attr_pos = row * CGM::COLS[videomode] + col;
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
                uint32_t fg = 
                        (((((fg_col & 0x7C00)>> 10) *8) & 0xFF) << 24) |
                        (((((fg_col & 0x03E0)>> 5)  *8) & 0xFF) << 16) |
                         ((((fg_col & 0x001F)       *8) & 0xFF) << 8)  |
                        0xFF ;
                uint32_t bg = 
                        (((((bg_col & 0x7C00)>> 10) *8) & 0xFF) << 24) |
                        (((((bg_col & 0x03E0)>> 5)  *8) & 0xFF) << 16) |
                        ((((bg_col & 0x001F)       *8)  & 0xFF) << 8 ) |
                        0xFF ;

                auto bit = 15 - (i % 16); // From MSB to LSB
                uint16_t word = bitfield_map + (i >> 4);
                unsigned x = i % CGM::WIDTH;
                unsigned y = i / CGM::WIDTH;
                if (cpu->getMem()[word] & 1<<bit) {
                    // Foreground
                    setPixel (x, y, fg);
                } else {
                    // Background
                    setPixel (x, y, bg);
                }
            }
            break;

        case 3: // 256x192 B&W. 2 colors 
            //Get the 2 colors
            { //Stop case label error
                uint16_t fg_ind = (cpu->getMem()[attribute_map] & 0x0FC0)
                                                                       >> 6;
                uint16_t bg_ind = (cpu->getMem()[attribute_map] & 0x003F);
                uint16_t fg_col, bg_col;
                if (palette_map == 0) { // Use default palette
                    fg_col = CGM::def_palette_map[fg_ind];
                    bg_col = CGM::def_palette_map[bg_ind];
                } else {
                    fg_col = cpu->getMem()[palette_map+ fg_ind];
                    bg_col = cpu->getMem()[palette_map+ bg_ind];
                }
                // Composes RGBA values from palette colors
                uint32_t fg =
                        (((((fg_col & 0x7C00)>> 10) *8) & 0xFF) << 24) |
                        (((((fg_col & 0x03E0)>> 5)  *8) & 0xFF) << 16) |
                        ((((fg_col & 0x001F)       *8) & 0xFF) << 8) |
                        0xFF ;
                uint32_t bg =
                        (((((bg_col & 0x7C00)>> 10) *8) & 0xFF) << 24) |
                        (((((bg_col & 0x03E0)>> 5)  *8) & 0xFF) << 16) |
                        ((((bg_col & 0x001F)       *8) & 0xFF) << 8) |
                        0xFF ;
                const unsigned loop_condition = CGM::WIDTH * CGM::HEIGHT/16;
                unsigned x = 0;
                unsigned y = 0;
            
                for (unsigned i=0; i < loop_condition; i++) {
                    uint16_t bit = cpu->getMem()[bitfield_map+i];
                    for (unsigned j = 0; j < 16; j++) {
                        if (bit & 1<<j) {
                            // Foreground
                            setPixel (x, y, fg);
                        } else {
                            // Background
                            setPixel (x, y, bg);
                        }
                        x++;
                    }

                    if (x>=CGM::WIDTH) {
                        y++;
                        x=0;
                    }
                }
            }
            break;

        case 4: // 256x192 4x8 font Text mode
            for (unsigned row=0; row < CGM::ROWS[videomode]; row++) {
                uint16_t row_offset_attr = row * CGM::COLS[videomode];
                uint16_t row_offset = row_offset_attr/2;

                for (unsigned col=0; col < CGM::COLS[videomode]; col++) {
                    uint16_t pos = bitfield_map + row_offset + (col/2);
                    uint16_t attr_pos = row * CGM::COLS[videomode] + col;
                    attr_pos += attribute_map;

                    auto row8 = row << 3;

                    // Every word contains two characters
                    unsigned char ascii;
                    if (col%2 == 0) {
                        ascii = (unsigned char) (cpu->getMem()[pos] & 0x00FF);
                    } else {
                        ascii = (unsigned char) ((cpu->getMem()[pos] & 0xFF00) >> 8);
                    }

                    // Get palette indexes
                    uint16_t fg_ind = (cpu->getMem()[attr_pos] & 0x0FC0) >> 6;
                    uint16_t bg_ind = (cpu->getMem()[attr_pos] & 0x003F);
                    // Get palette indexes and other attributes
                    uint16_t fg_col, bg_col;

                    if (palette_map == 0) { // Use default palette
                        fg_col = CGM::def_palette_map[fg_ind];
                        bg_col = CGM::def_palette_map[bg_ind];
                    } else {
                        fg_col = cpu->getMem()[palette_map+ fg_ind];
                        bg_col = cpu->getMem()[palette_map+ bg_ind];
                    }

                    // Does the blink
                    if (blink > blink_max &&
                           ((cpu->getMem()[attr_pos] & 0x1000) > 0) ) {
                        fg_col = bg_col;
                    }

                    // Composes RGBA values from palette colors
                    uint32_t fg =
                            (((((fg_col & 0x7C00)>> 10) << 3) & 0xFF) << 24) |
                            (((((fg_col & 0x03E0)>> 5)  << 3) & 0xFF) << 16) |
                            ((( (fg_col & 0x001F)       << 3) & 0xFF) << 8) |
                            0xFF ;
                    uint32_t bg =
                            (((((bg_col & 0x7C00)>> 10) << 3) & 0xFF) << 24) |
                            (((((bg_col & 0x03E0)>> 5)  << 3) & 0xFF) << 16) |
                            ((( (bg_col & 0x001F)       << 3) & 0xFF) << 8) |
                            0xFF ;

                    uint16_t glyph[2];
                    if (font_map == 0) { // Default font
                        glyph[0] = CGM::def_fonts[ascii*2]; 
                        glyph[1] = CGM::def_fonts[ascii*2+1];
                    } else {
                        glyph[0] = cpu->getMem()[font_map+ (ascii*2)]; 
                        glyph[1] = cpu->getMem()[font_map+ (ascii*2)+1]; 
                    }

                    auto col4 = col << 2;
                    for (int i=0; i< 8; i++) { 
                        // *** MSB ***
                        // First word 
                        bool pixel = ((1<<(i+8)) & glyph[0]) > 0;
                        if (pixel) {
                            setPixel (col4, row8 +i, fg);
                        } else {
                            setPixel (col4, row8 +i, bg);
                        }
                        // Second word
                        pixel = ((1<<(i+8)) & glyph[1]) > 0;
                        if (pixel) {
                            setPixel (col4 +2, row8 +i, fg);
                        } else {
                            setPixel (col4 +2, row8 +i, bg);
                        }

                        // *** LSB ***
                        // First word 
                        pixel = ((1<<i) & glyph[0]) >0;
                        if (pixel) {
                            setPixel (col4 +1, row8 +i, fg);
                        } else {
                            setPixel (col4 +1, row8 +i, bg);
                        }
                        // Second word
                        pixel = ((1<<i) & glyph[1]) > 0;
                        if (pixel) {
                            setPixel (col4 +3, row8 +i, fg);
                        } else {
                            setPixel (col4 +3, row8 +i, bg);
                        }
                    }

                    // Underline
                    if ((cpu->getMem()[attr_pos] & 0x2000) > 0)
                        for (int i=0; i< 4; i++)
                            setPixel(col4 +i, row8+ 7, fg);

                }
            }
            break;
        
        case 5: // 256x192 8x8 font Text mode

            for (unsigned row=0; row < CGM::ROWS[videomode]; row++) {
                uint16_t row_offset_attr = row * CGM::COLS[videomode];
                uint16_t row_offset = row_offset_attr/2;

                for (unsigned col=0; col < CGM::COLS[videomode]; col++) {
                    uint16_t pos = bitfield_map + row_offset + (col/2);
                    uint16_t attr_pos = row * CGM::COLS[videomode] + col;
                    attr_pos += attribute_map;

                    auto row8 = row << 3;

                    // Every word contains two characters
                    unsigned char ascii;
                    if (col%2 == 0) {
                        ascii = (unsigned char) (cpu->getMem()[pos] & 0x00FF);
                    } else {
                        ascii = (unsigned char) ((cpu->getMem()[pos] & 0xFF00) >> 8);
                    }

                    // Get palette indexes
                    uint16_t fg_ind = (cpu->getMem()[attr_pos] & 0x0FC0) >> 6;
                    uint16_t bg_ind = (cpu->getMem()[attr_pos] & 0x003F);
                    // Get palette indexes and other attributes
                    uint16_t fg_col, bg_col;

                    if (palette_map == 0) { // Use default palette
                        fg_col = CGM::def_palette_map[fg_ind];
                        bg_col = CGM::def_palette_map[bg_ind];
                    } else {
                        fg_col = cpu->getMem()[palette_map+ fg_ind];
                        bg_col = cpu->getMem()[palette_map+ bg_ind];
                    }

                    // Does the blink
                    if (blink > blink_max &&
                           ((cpu->getMem()[attr_pos] & 0x1000) > 0) ) {
                        fg_col = bg_col;
                    }

                    // Composes RGBA values from palette colors
                    uint32_t fg =
                            (((((fg_col & 0x7C00)>> 10) << 3) & 0xFF) << 24) |
                            (((((fg_col & 0x03E0)>> 5)  << 3) & 0xFF) << 16) |
                             ((((fg_col & 0x001F)       << 3) & 0xFF) << 8) |
                            0xFF ;
                    uint32_t bg = 
                            (((((bg_col & 0x7C00)>> 10) << 3) & 0xFF) << 24) |
                            (((((bg_col & 0x03E0)>> 5)  << 3) & 0xFF) << 16) |
                             ((((bg_col & 0x001F)       << 3) & 0xFF) << 8) |
                            0xFF ;

                    // From here changes from mode 4
                    uint16_t glyph[4];
                    if (font_map == 0) { // Default font
                        glyph[0] = CGM::def_fonts[256*2 + ascii*4   ];
                        glyph[1] = CGM::def_fonts[256*2 + ascii*4 +1];
                        glyph[2] = CGM::def_fonts[256*2 + ascii*4 +2];
                        glyph[3] = CGM::def_fonts[256*2 + ascii*4 +3];
                    } else {
                        glyph[0] = cpu->getMem()[font_map + ascii*4   ];
                        glyph[1] = cpu->getMem()[font_map + ascii*4 +1];
                        glyph[2] = cpu->getMem()[font_map + ascii*4 +2];
                        glyph[3] = cpu->getMem()[font_map + ascii*4 +3];
                    }

                    auto col8 = col << 3;
                    for (int i=0; i< 8; i++) {
                        // First word
                        // *** MSB ***
                        bool pixel = ((1<<(15-i)) & glyph[0]) > 0;
                        if (pixel) {
                            setPixel (col8+i, row8, fg);
                        } else {
                            setPixel (col8+i, row8, bg);
                        }
                        // *** LSB ***
                        pixel = ((1<<(7-i)) & glyph[0]) >0;
                        if (pixel) {
                            setPixel (col8+i, row8+1, fg);
                        } else {
                            setPixel (col8+i, row8+1, bg);
                        }

                        // Second word
                        // *** MSB ***
                        pixel = ((1<<(15-i)) & glyph[1]) > 0;
                        if (pixel) {
                            setPixel (col8+i, row8+2, fg);
                        } else {
                            setPixel (col8+i, row8+2, bg);
                        }
                        // *** LSB ***
                        pixel = ((1<<(7-i)) & glyph[1]) >0;
                        if (pixel) {
                            setPixel (col8+i, row8+3, fg);
                        } else {
                            setPixel (col8+i, row8+3, bg);
                        }

                        // Third word
                        // *** MSB ***
                        pixel = ((1<<(15-i)) & glyph[2]) > 0;
                        if (pixel) {
                            setPixel (col8+i, row8+4, fg);
                        } else {
                            setPixel (col8+i, row8+4, bg);
                        }
                        // *** LSB ***
                        pixel = ((1<<(7-i)) & glyph[2]) >0;
                        if (pixel) {
                            setPixel (col8+i, row8+5, fg);
                        } else {
                            setPixel (col8+i, row8+5, bg);
                        }

                        // Fourth word
                        // *** MSB ***
                        pixel = ((1<<(15-i)) & glyph[3]) > 0;
                        if (pixel) {
                            setPixel (col8+i, row8+6, fg);
                        } else {
                            setPixel (col8+i, row8+6, bg);
                        }
                        // *** LSB ***
                        pixel = ((1<<(7-i)) & glyph[3]) >0;
                        if (pixel) {
                            setPixel (col8+i, row8+7, fg);
                        } else {
                            setPixel (col8+i, row8+7, bg);
                        }
                    }

                    // Underline
                    if ((cpu->getMem()[attr_pos] & 0x2000) > 0)
                        for (int i=0; i< 8; i++)
                            setPixel(col8 +i, row8+7, fg);

                }
            }
            break;
        default:
            break; 
        }
    } 
}

uint32_t CGM::getBorder() const
{
    uint16_t border;
    if (palette_map == 0) { // Use default palette
        border = CGM::def_palette_map[border_col];
    } else {
        border = cpu->getMem()[palette_map+ border_col];
    }
    return  (((((border & 0x7C00)>> 10) *8) & 0xFF) << 24) |
            (((((border & 0x03E0)>> 5)  *8) & 0xFF) << 16) |
            ((( (border & 0x001F)       *8) & 0xFF) << 8)  |
            (0xFF) ;
}

} // END of NAMESPACE cgm

} // END of NAMESPACE cpu
