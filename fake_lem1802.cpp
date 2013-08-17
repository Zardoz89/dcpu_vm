#include "fake_lem1802.hpp"

#include <algorithm>
#include <cctype>

namespace cpu {

// Consts
const char esc = 27; // ESC character -> ANSI escape codes

/**
 * @brief Moves the terminal cursor at row X
 */
inline void row (int x)
{
    std::cout << esc << "[0;" << (x) << "H";
}

Fake_Lem1802::Fake_Lem1802() : screen_map (0), font_map (0), palette_map (0),
    border_col (0), ticks (0)
{
    // Fake default font
    def_font_map = new uint16_t[256];
    
    // Load default palette
    def_palette_map[ 0] = 0x0000;
    def_palette_map[ 1] = 0x000a;
    def_palette_map[ 2] = 0x00a0;
    def_palette_map[ 3] = 0x00aa;
    def_palette_map[ 4] = 0x0a00;
    def_palette_map[ 5] = 0x0a0a;
    def_palette_map[ 6] = 0x0a50;
    def_palette_map[ 7] = 0x0aaa;
    def_palette_map[ 8] = 0x0555;
    def_palette_map[ 9] = 0x055f;
    def_palette_map[10] = 0x05f5;
    def_palette_map[11] = 0x05ff;
    def_palette_map[12] = 0x0f55;
    def_palette_map[13] = 0x0f5f;
    def_palette_map[14] = 0x0ff5;
    def_palette_map[15] = 0x0fff;
}

Fake_Lem1802::~Fake_Lem1802()
{
    delete[] def_font_map;
}

void Fake_Lem1802::handleInterrupt()
{
    if (this->cpu == NULL)
        return;
        
    size_t s;
    
    switch (cpu->GetA() ) {
    case MEM_MAP_SCREEN:
        screen_map = cpu->GetB();
        
        if (screen_map != 0)
            ticks = 1667; // Force to do initial print
            
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
        s = RAM_SIZE - 1 - cpu->GetB() < 256 ? RAM_SIZE - 1 - cpu->GetB() : 256 ;
        std::copy_n (def_font_map, s, cpu->getMem() + cpu->GetB() );
        break;
        
    case MEM_DUMP_PALETTE:
        s = RAM_SIZE - 1 - cpu->GetB() < 16 ? RAM_SIZE - 1 - cpu->GetB() : 16 ;
        std::copy_n (def_palette_map, s, cpu->getMem() + cpu->GetB() );
        break;
        
    default:
        // do nothing
        break;
    }
}

void Fake_Lem1802::tick()
{
    if (++ticks > 1666) {
        // Update screen at 60Hz aprox if CPU clock runs at 100Khz
        ticks = 0;
        this->show();
    }
}

void Fake_Lem1802::show()
{
    using namespace std;
    
    if (this->cpu == NULL)
        return;
        
    if (screen_map != 0) {
        row (this->index * 32);
        
        for (int row = 0; row < 12; row++) {
            for (int col = 0; col < 32; col++) {
                uint16_t pos = screen_map + row * 32 + col;
                char tmp = (char) (cpu->getMem() [pos] & 0x00FF);
                
                if (isprint (tmp) )
                    cout << tmp;
                else
                    cout << ' ';
            }
            
            cout << endl;
        }
        
    }
}

} // END of NAMESPACE
