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
    border_col (0), ticks (0), enable (true)
    {
        // Default font
        def_font_map = new uint16_t[256]{
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

        // Default palette
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

    void Fake_Lem1802::attachTo (DCPU* cpu, size_t index) {
        this->IHardware::attachTo(cpu, index);

        tick_per_refresh = cpu->cpu_clock / 60;
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
        if (++ticks > tick_per_refresh) {
            // Update screen at 60Hz aprox
            ticks = 0;
            this->show();
        }
    }

    void Fake_Lem1802::show()
    {
        using namespace std;

        if (this->cpu == NULL)
            return;

        if (screen_map != 0 && enable) {
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


    void Fake_Lem1802::setEnable(bool enable) {
        this->enable = enable;
    }

} // END of NAMESPACE
