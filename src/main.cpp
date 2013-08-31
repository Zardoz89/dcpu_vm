#include <iostream>
#include <fstream>
#include <cstdint>
#include <algorithm>
#include <memory>
#include <stdio.h>
#include <chrono>

#include "config.hpp"
#include <chrono>

#include "dcpu_opcodes.hpp"
#include "dcpu.hpp"
#include "disassembler.hpp"

#include "gclock.hpp"
#include "fake_lem1802.hpp"
#include "lem1802.hpp"
#include "lem1803.hpp"

using namespace cpu;

int main (int argc, char **argv)
{

    std::string filename;
	size_t size = 0;
	uint16_t* data;
    std::ifstream binfile;
    
    
    if (argc <= 1) {
        std::cerr << "Missing input file\n";
        return 0;
    }
    
    filename = argv[1];
    std::cout <<  "Input BIN File : " << filename << "\n";
    
    binfile.open (filename.c_str(), std::ios::in | std::ios::binary );
    
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
	std::cout << "Reading " << filename << " size : " << size << "bytes\n";
    
    while (! binfile.eof() ) { 
        uint16_t word = 0;
        binfile.read ( (char*) &word, 2);
        uint16_t tmp = ( (word & 0xFF00) >> 8) & 0x00FF;
        word = ( (word & 0x00FF) << 8) | tmp;
        data[i] = word;
		//printf("Word 0x%x op Ox%x a 0x%x b 0x%x\n",word, word & 0x1F, word >> 10, (word >> 5) & 0x1F);
        i++;
    }
    
    binfile.close();
	
	/*std::string disassembled = disassembly(data,size / 2);
	std::ofstream dfile;
	filename += ".asm";
	dfile.open(filename.c_str(), std::ios::out);
	dfile << disassembled;
	dfile.close();*/
	
	
    
    std::cout << "Readed " << size << " bytes - " << size / 2 << " words\n";
    size /= 2;
    
    //Try win32 compatible emulation code
    //sf::RenderWindow window1802;
	sf::RenderWindow window1803;
    //window1802.create(sf::VideoMode(Lem1802::WIDTH, Lem1802::HEIGHT),"Lem 1802");
    window1803.setFramerateLimit(60);
	window1803.create(sf::VideoMode(Lem1803::WIDTH, Lem1803::HEIGHT),"Lem 1803");
	
    auto dcpu = std::make_shared<DCPU>();
    //auto lem1802 = std::make_shared<Lem1802>();
	auto lem1803 = std::make_shared<Lem1803>();
	auto gclock = std::make_shared<Generic_Clock>();
    //dcpu->attachHardware (lem1802);
	dcpu->attachHardware (lem1803);
	dcpu->attachHardware (gclock);
    dcpu->reset();
    dcpu->loadProgram (data, size);
	
	sf::Sprite sprite; //sprite of the screen
	sf::Clock clock; 
	
	
    while (/*window1802.isOpen() || */window1803.isOpen()) //Because non mainthread event are forbidden in OSX
    {
        // Process events
        sf::Event event;
        /*while (window1802.pollEvent(event)) 
        {
            // Close window : exit
            if (event.type == sf::Event::Closed) {
                window1802.close();
				//dcpu->detachHardware (0);
				//dcpu->reset();
				//dcpu->loadProgram (data, size);
			}
        }*/
		while (window1803.pollEvent(event)) 
        {
            // Close window : exit
            if (event.type == sf::Event::Closed) {
                window1803.close();
				//dcpu->detachHardware (1);
				//dcpu->reset();
				//dcpu->loadProgram (data, size);
		    }
        }
		
		///DCPU emulation stuff
		//lem1802->prepareRender();
		lem1803->prepareRender();
		const float delta=clock.getElapsedTime().asSeconds();
		clock.restart();
	    unsigned int tick_needed=(float)dcpu->cpu_clock*delta;
		if (tick_needed > dcpu->cpu_clock/60)
		   tick_needed = dcpu->cpu_clock/60;
		//std::cout << "ticked :" << tick_needed << std::endl;
		dcpu->tick(tick_needed);
		   
		
		/*window1802.setActive(true);
		///Update 1802 screen stuff
		sprite.setTexture(lem1802->getTexture(),true);
        // Clear screen
        window1802.clear();
        // Draw the sprite
        window1802.draw(sprite);
        // Update the window
        window1802.display();
		window1802.setActive(false);*/
		
		//For emulation mode
		sf::FloatRect r(0,0,lem1803->getTexture().getSize().x,
				            lem1803->getTexture().getSize().y);
		window1803.setView(sf::View(r));
		
		window1803.setActive(true);
		///Update 1803 screen stuff
		sprite.setTexture(lem1803->getTexture(),true);
        // Clear screen
        window1803.clear();
        // Draw the sprite
        window1803.draw(sprite);
        // Update the window
        window1803.display();
		window1803.setActive(false);
    }
    return 0;
}


