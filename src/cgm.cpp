#include "cgm.hpp"

#include <algorithm>
#include <cctype>
#include <string>


namespace cpu {

namespace cgm {

    const uint16_t CGM::def_fonts[256*2 + 256*4] = {  /// Font maps
        0xb79e, 0x388e, 0x722c, 0x75f4, 0x19bb, 0x7f8f, 0x85f9, 0xb158,
        0x242e, 0x2400, 0x082a, 0x0800, 0x0008, 0x0000, 0x0808, 0x0808,
        0x00ff, 0x0000, 0x00f8, 0x0808, 0x08f8, 0x0000, 0x080f, 0x0000,
        0x000f, 0x0808, 0x00ff, 0x0808, 0x08f8, 0x0808, 0x08ff, 0x0000,
        0x080f, 0x0808, 0x08ff, 0x0808, 0x6633, 0x99cc, 0x9933, 0x66cc,
        0xfef8, 0xe080, 0x7f1f, 0x0701, 0x0107, 0x1f7f, 0x80e0, 0xf8fe,
        0x5500, 0xaa00, 0x55aa, 0x55aa, 0xffaa, 0xff55, 0x0f0f, 0x0f0f,
        0xf0f0, 0xf0f0, 0x0000, 0xffff, 0xffff, 0x0000, 0xffff, 0xffff,
        0x0000, 0x0000, 0x005f, 0x0000, 0x0300, 0x0300, 0x3e14, 0x3e00,
        0x266b, 0x3200, 0x611c, 0x4300, 0x3629, 0x7650, 0x0002, 0x0100,
        0x1c22, 0x4100, 0x4122, 0x1c00, 0x1408, 0x1400, 0x081c, 0x0800,
        0x4020, 0x0000, 0x0808, 0x0800, 0x0040, 0x0000, 0x601c, 0x0300,
        0x3e49, 0x3e00, 0x427f, 0x4000, 0x6259, 0x4600, 0x2249, 0x3600,
        0x0f08, 0x7f00, 0x2745, 0x3900, 0x3e49, 0x3200, 0x6119, 0x0700,
        0x3649, 0x3600, 0x2649, 0x3e00, 0x0024, 0x0000, 0x4024, 0x0000,
        0x0814, 0x2200, 0x1414, 0x1400, 0x2214, 0x0800, 0x0259, 0x0600,
        0x3e59, 0x5e00, 0x7e09, 0x7e00, 0x7f49, 0x3600, 0x3e41, 0x2200,
        0x7f41, 0x3e00, 0x7f49, 0x4100, 0x7f09, 0x0100, 0x3e41, 0x7a00,
        0x7f08, 0x7f00, 0x417f, 0x4100, 0x2040, 0x3f00, 0x7f08, 0x7700,
        0x7f40, 0x4000, 0x7f06, 0x7f00, 0x7f01, 0x7e00, 0x3e41, 0x3e00,
        0x7f09, 0x0600, 0x3e61, 0x7e00, 0x7f09, 0x7600, 0x2649, 0x3200,
        0x017f, 0x0100, 0x3f40, 0x7f00, 0x1f60, 0x1f00, 0x7f30, 0x7f00,
        0x7708, 0x7700, 0x0778, 0x0700, 0x7149, 0x4700, 0x007f, 0x4100,
        0x031c, 0x6000, 0x417f, 0x0000, 0x0201, 0x0200, 0x8080, 0x8000,
        0x0001, 0x0200, 0x2454, 0x7800, 0x7f44, 0x3800, 0x3844, 0x2800,
        0x3844, 0x7f00, 0x3854, 0x5800, 0x087e, 0x0900, 0x4854, 0x3c00,
        0x7f04, 0x7800, 0x047d, 0x0000, 0x2040, 0x3d00, 0x7f10, 0x6c00,
        0x017f, 0x0000, 0x7c18, 0x7c00, 0x7c04, 0x7800, 0x3844, 0x3800,
        0x7c14, 0x0800, 0x0814, 0x7c00, 0x7c04, 0x0800, 0x4854, 0x2400,
        0x043e, 0x4400, 0x3c40, 0x7c00, 0x1c60, 0x1c00, 0x7c30, 0x7c00,
        0x6c10, 0x6c00, 0x4c50, 0x3c00, 0x6454, 0x4c00, 0x0836, 0x4100,
        0x0077, 0x0000, 0x4136, 0x0800, 0x0201, 0x0201, 0x0205, 0x0200
    }; // TODO Fill the upper half ASCII of the 4x8 font and add a 8x8 font

    const  uint16_t CGM::def_palette_map[64] = {    /// Default palette
#       include "64_palette.inc"
    };
   
    const unsigned int CGM::ROWS[3] = {24, 24, 96};
    const unsigned int CGM::COLS[3] = {64, 32, 32};

    // Clears the screen texture
    const static sf::Uint8 clear[CGM::WIDTH*CGM::HEIGHT*4]= {0};


    CGM::CGM() : 
        bitfield_map (0), attribute_map (0), palette_map (0), font_map (0),
        videomode(0), border_col (0), ticks (0), enable (true), blink(0) 
    { }

    CGM::~CGM() 
    {
        if (window.isOpen()) {
            window.close();
        }
        if (renderguy != NULL)
            delete renderguy;
    }

    void CGM::attachTo (DCPU* cpu, size_t index) 
    {
        this->IHardware::attachTo(cpu, index);

        tick_per_refresh = cpu->cpu_clock / CGM::FPS;
        blink_max = cpu->cpu_clock / BLINKTIME;

        title = "CGM 1084 DevId= ";
        char strbuff[33];
        snprintf(strbuff, 33,"%zu",index);
        title.append(strbuff);

        videomode = 0; // Mode 0

        //window.create(sf::VideoMode(CGM::WIDTH +20, 
        //            CGM::HEIGHT + 20), 
        //            title, sf::Style::Close | sf::Style::Titlebar);
                    
        // Set same size that LEM1802/3 screens
        window.create(sf::VideoMode((CGM::WIDTH/2)*3 +20, 
                    (CGM::HEIGHT/2)*3 + 20), 
                    title, sf::Style::Close | sf::Style::Titlebar);
        
        window.setFramerateLimit(CGM::FPS);
        texture.create(CGM::WIDTH, CGM::HEIGHT);
        texture.update(clear);
        texture.setRepeated(false);
        texture.setSmooth(false);

        window.setActive(false);
    	if (renderguy)
		   delete renderguy;
        renderguy = new sf::Thread(&CGM::render, this);
        renderguy->launch();

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
        using namespace std;

        if (this->cpu == NULL)
            return;
        
        if (bitfield_map != 0 && attribute_map != 0  && enable) { 
            // Update the texture
            switch (videomode) {
            case 0: // Mode 0 256x192-64x24 cells of 4x8 pixels. 64 colors
            
                for (unsigned row=0; row < CGM::ROWS[0]; row++) {
                    for (unsigned col=0; col < CGM::COLS[0]; col++) {
                        uint16_t attr_pos = row * CGM::COLS[0] + row;
                        attr_pos += attribute_map;
                        // Get palette indexes and other attributes
                        uint16_t fg_ind = (cpu->getMem()[attr_pos] & 0x0FC0) 
                            >> 6;
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
                        sf::Uint8 fg[] = {
                            (sf::Uint8)(((fg_col & 0x7C00)>> 10) *8),
                            (sf::Uint8)(((fg_col & 0x03E0)>> 5) *8),
                            (sf::Uint8)( (fg_col & 0x001F)      *8),
                            0xFF };
                        sf::Uint8 bg[] = {
                            (sf::Uint8)(((bg_col & 0x7C00)>> 10) *8),
                            (sf::Uint8)(((bg_col & 0x03E0)>> 5) *8),
                            (sf::Uint8)( (bg_col & 0x001F)      *8),
                            0xFF };

                        // Each cell needs two words
                        uint16_t pos = 2*row*CGM::COLS[0] + 2*row;
                        for (unsigned bit=0; bit < 8; bit++) {
                            // *** FIRST WORD
                            // MSB
                            if ((cpu->getMem()[pos] & (1<<(bit+8)) ) > 0) {
                                // Pixel active
                                texture.update(fg, 1, 1, col*4, row*8 +bit-8);
                            } else {
                                // Pixel off
                                texture.update(bg, 1, 1, col*4, row*8 +bit-8);
                            }
                            
                            // LSB
                            if ((cpu->getMem()[pos] & (1<<bit) ) > 0) {
                                // Pixel active
                                texture.update(fg, 1, 1, col*4+1, row*8 +bit-8);
                            } else {
                                // Pixel off
                                texture.update(bg, 1, 1, col*4+1, row*8 +bit-8);
                            }

                            // *** SECOND WORD
                            // MSB
                            if ((cpu->getMem()[pos+1] & (1<<(bit+8)) ) > 0) {
                                // Pixel active
                                texture.update(fg, 1, 1, col*4+2, row*8 +bit-8);
                            } else {
                                // Pixel off
                                texture.update(bg, 1, 1, col*4+2, row*8 +bit-8);
                            }
                            
                            // LSB
                            if ((cpu->getMem()[pos+1] & (1<<bit) ) > 0) {
                                // Pixel active
                                texture.update(fg, 1, 1, col*4+3, row*8 +bit-8);
                            } else {
                                // Pixel off
                                texture.update(bg, 1, 1, col*4+3, row*8 +bit-8);
                            }
                        }
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
                        sf::Uint8 fg[] = {
                            (sf::Uint8)(((fg_col & 0x7C00)>> 10) *8),
                            (sf::Uint8)(((fg_col & 0x03E0)>> 5) *8),
                            (sf::Uint8)( (fg_col & 0x001F)      *8),
                            0xFF };
                        sf::Uint8 bg[] = {
                            (sf::Uint8)(((bg_col & 0x7C00)>> 10) *8),
                            (sf::Uint8)(((bg_col & 0x03E0)>> 5) *8),
                            (sf::Uint8)( (bg_col & 0x001F)      *8),
                            0xFF };

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
                        for (int i=0; i< 8; i++) { 
                            // ***** MSB 
                            // First word 
                            bool pixel = ((1<<(i+8)) & glyph[0]) > 0;
                            if (pixel) {
                                texture.update(fg, 1, 1, 
                                        col*4, row*8 +i);
                            } else {
                                texture.update(bg, 1, 1, 
                                        col*4, row*8 +i);
                            }
                            // Second word
                            pixel = ((1<<(i+8)) & glyph[1]) > 0;
                            if (pixel) {
                                texture.update(fg, 1, 1, 
                                        col*4 +2, row*8 +i);
                            } else {
                                texture.update(bg, 1, 1, 
                                        col*4 +2, row*8 +i);
                            }

                            // ***** LSB
                            // First word 
                            pixel = ((1<<i) & glyph[0]) >0;
                            if (pixel) {
                                texture.update(fg, 1, 1, 
                                        col*4 +1, row*8 +i);
                            } else {
                                texture.update(bg, 1, 1, 
                                        col*4 +1, row*8 +i);
                            }
                            // Secodn word
                            pixel = ((1<<i) & glyph[1]) > 0;
                            if (pixel) {
                                texture.update(fg, 1, 1, 
                                        col*4 +3, row*8 +i);
                            } else {
                                texture.update(bg, 1, 1, 
                                        col*4 +3, row*8 +i);
                            }
                        }

                        for (int i=0; i< 8; i++) {
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
            texture.update(clear);
        }
    }


    void CGM::setEnable(bool enable) 
    {
        this->enable = enable;
    }

    
    void CGM::render() 
    {
        while (window.isOpen() ) { // Update the window draw
            sf::Event event;
            while (window.pollEvent(event)) {
                // "close requested" event: we close the window
                if (event.type == sf::Event::Closed) {
                    window.close();
                    return;
                }
            }

            sf::Sprite sprite;
            sprite.setTexture(texture);
            sprite.scale(1.5, 1.5);
            sprite.setPosition(10.0, 10.0);
           
            // Clear and set the border color
            uint16_t border;
            if (palette_map == 0) { // Use default palette
                border = CGM::def_palette_map[border_col];
            } else {
                border = cpu->getMem()[palette_map+ border_col];
            }
            window.clear(sf::Color(
                        (sf::Uint8)(((border & 0x7C00)>> 10) *8),
                        (sf::Uint8)(((border & 0x03E0)>> 5)  *8),
                        (sf::Uint8)( (border & 0x001F)       *8),
                        0xFF ));

            
            window.draw(sprite);

            window.display();
        }
    }

} // END of NAMESPACE cgm

} // END of NAMESPACE cpu
