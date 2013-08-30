#include "lem1802.hpp"

#include <algorithm>
#include <cctype>
#include <string>


namespace cpu {

    const uint16_t Lem1802::def_font_map[128*2] = {   /// Default font map
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

   const uint16_t Lem1802::def_palette_map[16] = {    /// Default palette
        0x0000, 0x000a, 0x00a0, 0x00aa, 0x0a00, 0x0a0a, 0x0a50, 0x0aaa, 
        0x0555, 0x055f, 0x05f5, 0x05ff, 0x0f55, 0x0f5f, 0x0ff5, 0x0fff
   };

   // Clears the screen texture
   const static sf::Uint8 clear[Lem1802::WIDTH*Lem1802::HEIGHT*4]= {0};


    Lem1802::Lem1802() : screen_map (0), font_map (0), palette_map (0),
    border_col (0), ticks (0), enable (true), blink(0), renderguy(NULL)
    { }

    Lem1802::~Lem1802() 
    {
        if (window.isOpen()) {
            window.close();
        }
        if (renderguy) {
            delete renderguy;
            renderguy = NULL;
        }
    }

    void Lem1802::attachTo (DCPU* cpu, size_t index) {
        this->IHardware::attachTo(cpu, index);

        tick_per_refresh = cpu->cpu_clock / Lem1802::FPS;

        title = "LEM1802 DevId= ";
        title.append( std::to_string(index));

        window.create(sf::VideoMode(Lem1802::WIDTH*3 +20, 
                    Lem1802::HEIGHT*3 + 20), 
                    title, sf::Style::Close | sf::Style::Titlebar);
        
        window.setFramerateLimit(Lem1802::FPS);
        texture.create(Lem1802::WIDTH, Lem1802::HEIGHT);
        texture.update(clear);
        texture.setRepeated(false);
        texture.setSmooth(false);

        window.setActive(false);
    	if (renderguy)
		   delete renderguy;
		renderguy = new sf::Thread(&Lem1802::render,this);
        renderguy->launch();


    }

    void Lem1802::handleInterrupt()
    {
        if (this->cpu == NULL)
            return;

        size_t s;
        switch (cpu->GetA() ) {
            case MEM_MAP_SCREEN:
                screen_map = cpu->GetB();

                if (screen_map != 0)
                    ticks = tick_per_refresh +1; // Force to do initial print

                break;

            case MEM_MAP_FONT:
                font_map = cpu->GetB();
                break;

            case MEM_MAP_PALETTE:
                palette_map = cpu->GetB();
                break;

            case SET_BORDER_COLOR:
                border_col = cpu->GetB() & 0xf;
                break;

            case MEM_DUMP_FONT:
                s = RAM_SIZE - 1 - cpu->GetB() < 256 ? 
                        RAM_SIZE - 1 - cpu->GetB() : 256 ;
                std::copy_n (Lem1802::def_font_map, s, cpu->getMem() + cpu->GetB() );
                break;

            case MEM_DUMP_PALETTE:
                s = RAM_SIZE - 1 - cpu->GetB() < 16 ?
                        RAM_SIZE - 1 - cpu->GetB() : 16 ;
                std::copy_n (Lem1802::def_palette_map, s, 
                        cpu->getMem() + cpu->GetB() );
                break;

            default:
                // do nothing
                break;
        }
    }

    void Lem1802::tick()
    {
        if (++ticks > tick_per_refresh) {
            // Update screen at 60Hz aprox
            ticks = 0;
            this->show();
        }
        if (++blink > Lem1802::BLINKRATE*2)
            blink = 0;
    }

    void Lem1802::show()
    {
        using namespace std;

        if (this->cpu == NULL)
            return;
        
        if (screen_map != 0 && enable) { // Update the texture
            for (unsigned row=0; row < Lem1802::ROWS; row++) {
                for (unsigned col=0; col < Lem1802::COLS; col++) {
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
                    if (blink > Lem1802::BLINKRATE &&  
                           ((cpu->getMem()[pos] & 0x80) > 0) ) {
                        fg_col = bg_col;
                    }

                    // Composes RGBA values from palette colors
                    sf::Uint8 fg[] = {
                        (sf::Uint8)(((fg_col & 0xF00)>> 8) *0x11),
                        (sf::Uint8)(((fg_col & 0x0F0)>> 4) *0x11),
                        (sf::Uint8)( (fg_col & 0x00F)      *0x11),
                        0xFF };
                    sf::Uint8 bg[] = {
                        (sf::Uint8)(((bg_col & 0xF00)>> 8) *0x11),
                        (sf::Uint8)(((bg_col & 0x0F0)>> 4) *0x11),
                        (sf::Uint8)( (bg_col & 0x00F)      *0x11),
                        0xFF };
    
                    uint16_t glyph[2];
                    if (font_map == 0) { // Default font
                        glyph[0] = Lem1802::def_font_map[ascii*2]; 
                        glyph[1] = Lem1802::def_font_map[ascii*2+1];
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


    void Lem1802::setEnable(bool enable) 
    {
        this->enable = enable;
    }

    
    void Lem1802::render() 
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
            sprite.scale(3.0, 3.0);
            sprite.setPosition(10.0, 10.0);
           
            // Clear and set the border color
            uint16_t border;
            if (palette_map == 0) { // Use default palette
                border = Lem1802::def_palette_map[border_col];
            } else {
                border = cpu->getMem()[palette_map+ border_col];
            }
            window.clear(sf::Color(
                        ((border &0x0F00) >> 8) *0x11 ,
                        ((border &0x00F0) >> 4) *0x11 ,
                        ((border &0x000F)     ) *0x11 ,
                        0xFF ));
            
            window.draw(sprite);

            window.display();
        }
    }

} // END of NAMESPACE
