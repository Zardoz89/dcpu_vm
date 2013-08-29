#include "lem1803.hpp"

#include <algorithm>
#include <cctype>
#include <string>


namespace cpu {

    
    const uint16_t Lem1803::def_font_map2[512] = {   /// Default font map
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
   };

    const uint16_t Lem1803::def_palette_map2[64] = {    /// Default palette
#       include "64_palette.inc"
    };
   
    // Clears the screen texture
    const static sf::Uint8 clear[Lem1803::WIDTH*Lem1803::HEIGHT*4]= {0};

    Lem1803::Lem1803() : emulation_mode(true) { }

    Lem1803::~Lem1803() { }

    void Lem1803::attachTo (DCPU* cpu, size_t index) {
        this->IHardware::attachTo(cpu, index);

        tick_per_refresh = cpu->cpu_clock / FPS;

        title = "LEM1803 DevId= ";
        title.append( std::to_string(index));
        window.create(sf::VideoMode(Lem1802::WIDTH*3 +20, 
                    Lem1802::HEIGHT*3 + 20), 
                    title, sf::Style::Close | sf::Style::Titlebar);
        
        window.setFramerateLimit(FPS);
        texture.create(Lem1803::WIDTH, Lem1803::HEIGHT);
        texture.update(clear);
        texture.setSmooth(false);
        texture.setRepeated(false);

        window.setActive(false);
        renderguy = boost::thread(&Lem1803::render, this);
        renderguy.detach();
    }

    void Lem1803::handleInterrupt()
    {
        if (this->cpu == NULL)
            return;


        if (cpu->GetA() == LEGACY_MODE) {
            emulation_mode = !emulation_mode;
            font_map = palette_map = screen_map = 0;
            return;
        } 

        size_t s;
        if (!emulation_mode) { // Only this commands are diferent 
            if (cpu->GetA() == MEM_DUMP_FONT) {
                s = RAM_SIZE - 1 - cpu->GetB() < 512 ? 
                        RAM_SIZE - 1 - cpu->GetB() : 512 ;
                std::copy_n (Lem1803::def_font_map2, s, cpu->getMem() + cpu->GetB() );
                return;
            } else if (cpu->GetA() == MEM_DUMP_PALETTE) {
                s = RAM_SIZE - 1 - cpu->GetB() < 64 ?
                        RAM_SIZE - 1 - cpu->GetB() : 64 ;
                std::copy_n (Lem1803::def_palette_map2, s, 
                        cpu->getMem() + cpu->GetB() );
                return;
            }
        }
        Lem1802::handleInterrupt();
    }

    void Lem1803::show()
    {

        if (this->cpu == NULL)
            return;

        if (emulation_mode) {
            Lem1802::show();
            return;
        }
        
        if (screen_map != 0 && enable) { // Update the texture
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
                    if (blink > BLINKRATE &&  
                           ((cpu->getMem()[pos_attr] & 0x1000) > 0) ) {
                        fg_col = bg_col;
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
    
                    uint16_t glyph[2];
                    if (font_map == 0) { // Default font
                        glyph[0] = Lem1803::def_font_map2[ascii*2]; 
                        glyph[1] = Lem1803::def_font_map2[ascii*2+1];
                    } else {
                        glyph[0] = cpu->getMem()[font_map+ (ascii*2)]; 
                        glyph[1] = cpu->getMem()[font_map+ (ascii*2)+1]; 
                    }
                    
                    for (int i=8; i< 16; i++) { // Puts MSB of Words
                        // First word 
                        bool pixel = ((1<<i) & glyph[0]) > 0;
                        if (pixel) {
                            texture.update(fg, 1, 1, 
                                    col*4, row*8 +i-8);
                        } else {
                            texture.update(bg, 1, 1, 
                                    col*4, row*8 +i-8);
                        }
                        // Secodn word
                        pixel = ((1<<i) & glyph[1]) > 0;
                        if (pixel) {
                            texture.update(fg, 1, 1, 
                                    col*4 +2, row*8 +i-8);
                        } else {
                            texture.update(bg, 1, 1, 
                                    col*4 +2, row*8 +i-8);
                        }
                    }

                    for (int i=0; i< 8; i++) { // Puts LSB of Words
                        // First word 
                        bool pixel = ((1<<i) & glyph[0]) >0;
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
                }
            }
        } else {
            texture.update(clear);
        }
    }

    void Lem1803::render() 
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
            if (emulation_mode) { 
                sprite.setScale(3, 3);
            } else {
                sprite.setScale(1, 1);
            }
            sprite.setPosition(10.0, 10.0);
           
            // Clear and set the border color
            uint16_t border;
            if (palette_map == 0) { // Use default palette
                border = Lem1803::def_palette_map[border_col];
            } else {
                border = cpu->getMem()[palette_map+ border_col];
            }
            window.clear(sf::Color(
                        (sf::Uint8)(((border & 0x7C00)>> 10) *8),
                        (sf::Uint8)(((border & 0x03E0)>> 5) *8),
                        (sf::Uint8)( (border & 0x001F)      *8),
                        0xFF ));
            
            window.draw(sprite);

            window.display();
        }

    }

} // END of NAMESPACE
