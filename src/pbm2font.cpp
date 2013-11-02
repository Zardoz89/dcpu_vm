/**//**************************************************************************
 *   PBM to LEM & CGM fonts tool
 *   This program reads a plain text PBM image file, and generates a 4x8 (LEM) 
 *   or 8x8 (CGM) font from it
 *   It outputs in .DAT format or in a list of HEX values
 *
 *   For more information about PBM file format, see: 
 *      http://en.wikipedia.org/wiki/Netpbm_format#PBM_example
 *****************************************************************************/

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <iomanip> 
#include <cstdlib>
#include <cstdint>
#include <cctype>

#include "config.hpp"

enum OUTPUT_FORMAT {
    DAT,
    HEX_DUMP,
};

enum CHARSET_TYPE {
    C4x8,
    C8x8,
};

void print_help(std::string program_name)
{
    using namespace std;
    cout << _PRG_NAME << " " << _VERSION << endl;
    cout << "usage : " << program_name << " [-options] <input-file>\n";
    cout << "--------------------------------------------------------\n";
    cout << "  options:" << endl;
    cout << "    -output <filename> (-o) : output filename\n";
    cout << "    -charset=<charset_type> : use the following charset type\n";
    cout << "            4x8 -> Generates 4x8 font charset\n";
    cout << "            8x8 -> Generates 8x8 font charset\n";
    cout << "    -format=<output_format> : use the following format\n";
    cout << "            dat -> Uses universal .dat output format\n";
    cout << "            hex_dump -> Generates a hexadecimal dump\n";
    cout << "            By defaults, the ouput format is \"dat\" and the"
         << " charset is 4x8\n";
}

int main (int argc, char **argv)
{
    // TODO Debug level should be set at compile time, or have a hidden program
    // option to set this, like the -v -vv -vvv flags
    logger::LOG_level = logger::LogLevel::INFO; 

    OUTPUT_FORMAT format = OUTPUT_FORMAT::DAT;
    CHARSET_TYPE type = CHARSET_TYPE::C4x8;
    std::string filename;
    std::ifstream fin;
    std::ofstream fout;
    bool use_fout = false;

    //TODO make a function that parse argument into a program struct
    for (int k=1; k < argc; k++) { //parse arguments
        if (argv[k][0] == '-') {
            std::string opt = argv[k];
            
            if (opt=="--help"||opt=="-help"||opt=="-h") {
                std::string pn = argv[0];
                pn.erase(0,pn.find_last_of('\\')+1); //windows
                pn.erase(0,pn.find_last_of('/')+1); //linux
                print_help(pn);
                return 0;
            } else if ((opt == "-output" || opt == "-o") && argc > k+1) {
                if (fout.is_open())
                    fout.close(); // Safeguard

                fout.open(argv[++k]);
                use_fout = fout.is_open();
            } else if  (opt == "-output" || opt == "-o") {
                LOG_WARN << "Option " + opt +
                        " requiered another argument it will be ignored here";
            } else if (opt == "-format=dat" ) {
                format = OUTPUT_FORMAT::DAT;
            } else if (opt == "-format=hex_dump") {
                format = OUTPUT_FORMAT::HEX_DUMP;
            } else if (opt == "-charset=4x8") {
                type = CHARSET_TYPE::C4x8;
            } else if (opt == "-charset=8x8") {
                type = CHARSET_TYPE::C8x8;
            } else {
                LOG_WARN << "Unknown option " + opt + " it will be ignored !";
            }
        } else {
            filename = argv[k];
        }
    }
    if (filename.size() <= 0) {
        LOG_ERROR << "Missing or invalid input filename";
        return -1;
    }

    fin.open(filename);
    if (! fin.is_open()) {
        LOG_ERROR << "Error opening input file " + filename;
        return -1;
    }

    if (use_fout) {
        fout << std::hex;
    } else {
        std::cout << std::hex;
    }

    // Read the PBM file
    std::string str; 
    std::getline(fin, str);

    // Check header
    if (str.size() <= 0 || str.substr(0, 2) != "P1") {
        LOG_ERROR << "Invalid input file. Must be a ASCII PBM file. Aborting.";
        return -1;
    }

    long width = -1;
    long height = -1;

    size_t num_glyphs = 0;

    bool framebuffer = false;
    while (std::getline(fin, str)) { // Read frambeuffer size
        // Remove comments
        auto pos = str.find("#");
        if (pos != std::string::npos) {
            str.resize(pos);
        }

        if (str.size() <= 0)
            continue;

        // Reads framebuffer size
        pos = str.find(" ");
        if (pos == std::string::npos)
            continue; // Not valid line

        width = std::atol(str.substr(0, pos).c_str());
        height = std::atol(str.substr(pos).c_str());
        
        // Calcs the number of glyphs
        if (type == CHARSET_TYPE::C4x8) {
            num_glyphs = (width / 4) * (height / 8);
        } else { // 8x8 font
            num_glyphs = (width / 8) * (height / 8);
        }

        framebuffer = true;
        break;
    }

    if (!framebuffer) {
        LOG_ERROR << "Invalid file format. Missing framebuffer size";
        return -1;
    }

    // reads the frame buffer itself
    unsigned words_per_glyph = (type== CHARSET_TYPE::C4x8)? 2 : 4;
    unsigned glyphs_row = (type== CHARSET_TYPE::C4x8)? (width /4) : (width /8);
    unsigned num_words = words_per_glyph * glyphs_row;
    unsigned glyph_height = 8;

    // We store glyphs_row glyphs, because we read rows in the PBM file
    uint16_t* glyphs = new uint16_t[num_words]();  
    
    LOG << "PBM file of X: " << width << " Y: " << height;
    LOG << "Num of glyphs: " << num_glyphs;

    char c;
    bool comment = false;
    unsigned x = 0;
    unsigned y = 0;
    unsigned addr = 0;
    while ((c = fin.get()) != std::char_traits<char>::eof() ) {
        if (c == '\r' || c == '\n' ) {
            comment = false;
            continue;
        } else if (c == '#') {
            comment = true;
            continue;
        } else if (comment || ! std::isalnum(c)) {
            continue;
        }

        if (type == CHARSET_TYPE::C4x8) {
            auto glyph = x / 4;
            auto word = glyph*2 + ((x%4)>1);
            if (x & 1) { // Odd
                glyphs[word] |= (c == '1') << (y%8);
            } else {     // Even
                glyphs[word] |= (c == '1') << (8+(y%8));
            }
        
        } else { // 8x8
            auto glyph = x / 8;
            auto word = glyph*4 + ((y%8)>>1);
            if (y & 1) { // Odd
                glyphs[word] |= (c == '1') << (7-(x%8));
            } else {     // Even
                glyphs[word] |= (c == '1') << (15-(x%8));
            }

        }
        
        if (++x >= (unsigned) width) {
            x = 0;
            y++;
        }

        if (((y%glyph_height) == 0) && (x == 0) && (y > 0)) {
            // Output
            if (format == OUTPUT_FORMAT::DAT) {
                if (use_fout) {
                    for (unsigned i=0; i< num_words; i++)
                        fout << "dat 0x" << glyphs[i] << std::endl;
                } else {
                    for (unsigned i=0; i< num_words; i++)
                        std::cout << "dat 0x" << glyphs[i] << std::endl;
                }

            } else {                
                if (use_fout) {
                    for (unsigned i=0; i< num_words; i++) {
                        if (i % 8 == 0) {
                            fout << std::setw(0) << "0x";
                            fout << std::setfill('0') << std::setw(4);
                            fout << addr +i << ":";
                            fout << std::setw(0);
                        }
                        fout << " ";
                        fout << std::setfill('0') << std::setw(4);
                        fout << glyphs[i];
                        if (i % 8 == 7 || i+1 == num_words)
                            fout << std::endl;
                    }
                } else {
                    for (unsigned i=0; i< num_words; i++) {
                        if (i % 8 == 0) {
                            std::cout << std::setw(0) << "0x";
                            std::cout << std::setfill('0') << std::setw(4);
                            std::cout << addr +i << ":";
                            std::cout << std::setw(0);
                        }
                        std::cout << " ";
                        std::cout << std::setfill('0') << std::setw(4);
                        std::cout << glyphs[i];
                        if (i % 8 == 7 || i+1 == num_words)
                            std::cout << std::endl;
                    }
                }

            }
            
            addr += num_words;
            std::fill_n(glyphs, num_words, 0);
        }
    }

    delete[] glyphs;
    return 0;
}
