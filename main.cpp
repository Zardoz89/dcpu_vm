#include <iostream>
#include <fstream>
#include <cstdint>
#include <algorithm>

#include <stdio.h>

#include "dcpu.hpp"
#include "fake_lem1802.hpp"

using namespace cpu;


int main (int argc, char **argv)
{

    char* filename;
    std::ifstream binfile;
    uint16_t* data = NULL;
    size_t size = 0;
    
    
    if (argc <= 1) {
        std::cerr << "Missing input file\n";
        return 0;
    }
    
    filename = argv[1];
    std::cout <<  "Input BIN File : " << filename << "\n";
    
    binfile.open (filename, std::ios::in | std::ios::binary );
    
    if (!binfile) {
        std::cerr << "ERROR: I can open file\n";
        exit (1);
    }
    
    // get length of file:
    binfile.seekg (0, binfile.end);
    size = binfile.tellg();
    binfile.seekg (0, binfile.beg);
    
    data = new uint16_t[size / 2 + 1]();
    std::fill_n (data, size / 2, 0); // Clean it
    
    int i = 0;
    
    while (! binfile.eof() ) {
        uint16_t word = 0;
        binfile.read ( (char*) &word, 2);
        unsigned char tmp = ( (word & 0xFF00) >> 8) & 0x00FF;
        word = ( (word & 0x00FF) << 8) | tmp;
        data[i] = word;
        i++;
    }
    
    binfile.close();
    
    std::cout << "Readed " << size << " bytes - " << size / 2 << " words\n";
    size /= 2;
    
    auto cpu = std::make_shared<DCPU>();
    auto screen = std::make_shared<Fake_Lem1802>();
    
    cpu->attachHardware (screen);
    cpu->reset();
    cpu->loadProgram (data, size);
    
    
    //std::cout << cpu->dumpRegisters() << "\n";
    
    while (getchar() != 'q') {
        //std::cout << "RAM[PC] = " << cpu->dumpRam() << "\n";
        for (int i = 0; i < 100; i++)
            cpu->tick();
            
        //std::cout << cpu->dumpRegisters() << "\n";
        //std::cout << "T cycles " << cpu->getTotCycles() << "\n";
        //if (cpu->GetSP() != 0x0000)
        //    std::cout << "STACK : "<< cpu->dumpRam(cpu->GetSP(), 0xFFFF) << "\n";
        
        
    }
    
    
    delete[] data;
    return 0;
}
