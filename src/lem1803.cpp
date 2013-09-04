#include "lem1803.hpp"

#include <algorithm>
#include <cctype>
#include <string>

namespace cpu {

namespace lem {

    
    const uint16_t Lem1803::def_font_map2[256*2] = {   /// Default font map
#       include "lem1802_font.inc"
        // TODO Upper half of 8-bit Extended ASCII
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
		initScreen();
		if (emulation_mode)
			texture.create(Lem1802::WIDTH, Lem1802::HEIGHT);
		else
			texture.create(Lem1803::WIDTH, Lem1803::HEIGHT);
        texture.update(clear);
        texture.setSmooth(false);
        texture.setRepeated(false);
    }

    void Lem1803::handleInterrupt()
    {
        if (this->cpu == NULL)
            return;


        if (cpu->GetA() == LEGACY_MODE) {
            emulation_mode = !emulation_mode;
            font_map = palette_map = screen_map = 0;
			initScreen();
			if (emulation_mode)
			    texture.create(Lem1802::WIDTH, Lem1802::HEIGHT);
			else
				texture.create(Lem1803::WIDTH, Lem1803::HEIGHT);
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

        if (this->cpu == NULL || !need_render)
            return;
        if (emulation_mode) {
            Lem1802::show();
            return;
        }
        need_render = false;
        if (screen_map != 0 && enable) { // Update the texture
		    
		    uint8_t* pixel_pos = screen;
			//4 pixel per col 4 value per pixels
			const unsigned row_pixel_size = Lem1803::WIDTH*4; 
			const unsigned row_pixel_size_8 = row_pixel_size*8; 
            for (unsigned row=0; row < Lem1803::ROWS; row++) {
			    pixel_pos=screen + row*row_pixel_size_8;
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
                    
                    uint8_t* current_pixel_pos = pixel_pos;
                    for (int i=8; i< 16; i++) { // Puts MSB of Words
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

                    for (int i=0; i< 8; i++) { // Puts LSB of Words
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
            }
			texture.update(screen);
        } else {
            texture.update(clear);
        }
    }

} // END OF NAMESPACE lem

} // END of NAMESPACE cpu
