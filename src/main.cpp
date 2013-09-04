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
	int monitor_type=0; 
	size_t size = 0;
	uint16_t* data;
    std::ifstream binfile;
    
    
    if (argc <= 1) {
        std::cerr << "Missing input file\n";
        return 0;
    }
	for (int k=1; k < argc; k++) //parse arguments
	{
		if (argv[k][0] == '-')
		{
			std::string opt = argv[k];
			if (opt.find("--monitor") != std::string::npos)
			{
				if (opt == "--monitor=1802") monitor_type=0;
				else if (opt == "--monitor=1803") monitor_type=1; 
				else if (opt == "--monitor=cgm") monitor_type=2;
				else { 
					std::cout << "Warning unknow monitor type "; 
					std::cout << opt << std::endl;
				}
			}
			else
			{
				std::cout << "Warning unknow option ";
				std::cout << opt << std::endl;
			}
		}
		else
		{
			filename = argv[k];
		}
	
    }
    
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
    
    while (! binfile.eof() ) { 
        uint16_t word = 0;
        binfile.read ( (char*) &word, 2);
        uint16_t tmp = ( (word & 0xFF00) >> 8) & 0x00FF;
        word = ( (word & 0x00FF) << 8) | tmp;
        data[i] = word;
        i++;
    }
    
    binfile.close();
	
    
    std::cout << "Readed " << size << " bytes - " << size / 2 << " words\n";
    size /= 2;
    
	
	
    auto dcpu = std::make_shared<DCPU>();
	auto gclock = std::make_shared<Generic_Clock>();
	std::shared_ptr<AbstractMonitor> monitor;
	switch (monitor_type)
	{
		case 1:
			monitor=std::make_shared<Lem1803>();
			std::cout << "Use Lem1803 Monitor" << std::endl;
			break;
		case 2:
			monitor=std::make_shared<Lem1803>();
			std::cout << "Use CGM Monitor" << std::endl;
			break;
		default :
			monitor=std::make_shared<Lem1802>();
			std::cout << "Use Lem1802 Monitor" << std::endl;
			break;
	}
	
    dcpu->attachHardware (monitor);
	dcpu->attachHardware (gclock);
    dcpu->reset();
    dcpu->loadProgram (data, size);
	
	sf::Sprite sprite; //sprite of the screen
	sf::RectangleShape border; //Screen border
	sf::Clock clock; 
	sf::RenderWindow window;
	float border_add = monitor->borderSize()*2;
    window.create(sf::VideoMode(monitor->phyWidth()+border_add,
								monitor->phyHeight()+border_add),
								"dcpu_vm");
    window.setFramerateLimit(60);
	
	
    while (window.isOpen()) //Because non mainthread event are forbidden in OSX
    {
        // Process events
        sf::Event event;
		while (window.pollEvent(event)) 
        {
            // Close window : exit
            if (event.type == sf::Event::Closed) {
                window.close();
		    }
        }
		
		///DCPU emulation stuff
		monitor->prepareRender();
		const float delta=clock.getElapsedTime().asSeconds();
		clock.restart();
	    unsigned int tick_needed=(float)dcpu->cpu_clock*delta;
		
		if (tick_needed > dcpu->cpu_clock/60)
		   tick_needed = dcpu->cpu_clock/60;
		dcpu->tick(tick_needed);
		
		border_add = monitor->borderSize();
		sprite.setPosition(sf::Vector2f(border_add,border_add));
		border_add *=2;
		
		float r_width = border_add + monitor->getScreen().getSize().x;
		float r_height = border_add + monitor->getScreen().getSize().y;
		border.setSize(sf::Vector2f(r_width,r_height));
		border.setFillColor(monitor->getBorder());
		
		//For emulations modes
		sf::FloatRect r(0,0,r_width,r_height);
		window.setView(sf::View(r));
		window.setActive(true);
		sprite.setTexture(monitor->getScreen(),true);
        window.clear();
		window.draw(border);
        window.draw(sprite);
        window.display();
		window.setActive(false);
    }
    return 0;
}


