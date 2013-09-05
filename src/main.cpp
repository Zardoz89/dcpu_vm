#include <iostream>
#include <fstream>
#include <cstdint>
#include <algorithm>
#include <memory>
#include <stdio.h>
#include <chrono>

#include "config.hpp"
#include "dcpu_opcodes.hpp"
#include "dcpu.hpp"
#include "disassembler.hpp"
#include "gclock.hpp"
#include "gkeyboard.hpp"
#include "lem1802.hpp"
#include "lem1803.hpp"
#include "cgm.hpp"



using namespace cpu;

void print_help(std::string program_name)
{
    std::cout << "usage : " << program_name << " [--options] <dcpu16-exe>\n";
    std::cout << "--------------------------------------------------------\n";
    std::cout << "  options:" << std::endl;
    std::cout << "    --debug                  : start in debug mode\n";
    std::cout << "            F1  : next step" << std::endl;
    std::cout << "            F2  : print registers" << std::endl;
    std::cout << "            F3  : reset (no need debug mode)" << std::endl;
    std::cout << "            F12 : switch debug/run" << std::endl;
    std::cout << "    --monitor=<monitor_name> : use the following monitor\n";
    std::cout << "            1802 -> Lem1802 (default) [c]" << std::endl;
    std::cout << "            1803 -> Lem1803 [c]" << std::endl;
    std::cout << "            cgm -> Colour Graphics Monitor" << std::endl;
    std::cout << "            [c] : compatible with Lem1802 0x10c programs\n";
}

int main (int argc, char **argv)
{

    std::string filename;
    int monitor_type=0; 
    bool debug=false;
    size_t size = 0;
    uint16_t* data;
    std::ifstream binfile;
    
    if (argc <= 1) {
        std::cerr << "Missing input file, type --help for list options\n";
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
            else if (opt=="--help"||opt=="-help"||opt=="-h")
            {
                std::string pn = argv[0];
                pn.erase(0,pn.find_last_of('\\')+1); //windows
                pn.erase(0,pn.find_last_of('/')+1); //linux
                print_help(pn);
                return 0;
            }
            else if (opt=="--debug")
            {
                debug=true;
            }
            else
            {
                std::cout << "Warning: unknow option ";
                std::cout << opt << " it will be ignored !" << std::endl;
            }
        }
        else
        {
            filename = argv[k];
        }
    
    }
    
    
    //TODO make a function which do that but fast
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
        //need improvement read (read whole file and then switch endianess
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
    
    
    sf::String window_title="dcpu_vm";
    auto dcpu = std::make_shared<DCPU>();
    auto gclock = std::make_shared<Generic_Clock>();
    auto gkeyboard = std::make_shared<keyboard::GKeyboard>();
    std::shared_ptr<AbstractMonitor> monitor;
    switch (monitor_type)
    {
        case 1:
            monitor=std::make_shared<lem::Lem1803>();
            std::cout << "Use Lem1803 Monitor" << std::endl;
            window_title = "Lem 1803";
            break;
        case 2:
            monitor=std::make_shared<cgm::CGM>();
            std::cout << "Use CGM Monitor" << std::endl;
            window_title = "CGM";
            break;
        default :
            monitor=std::make_shared<lem::Lem1802>();
            std::cout << "Use Lem1802 Monitor" << std::endl;
            window_title = "Lem 1802";
            break;
    }
    
    dcpu->attachHardware (monitor);
    dcpu->attachHardware (gclock);
    dcpu->attachHardware (gkeyboard);
    dcpu->reset();
    dcpu->loadProgram (data, size);
   
    sf::Texture texture; // texture of the screen
    sf::Sprite sprite;   //sprite of the screen
    const sf::Image* screen = monitor->getScreen();
    sf::Clock clock; 
    sf::RenderWindow window;
    float border_add = monitor->borderSize()*2;
    window.create(sf::VideoMode(monitor->phyWidth()+border_add,
                                monitor->phyHeight()+border_add),
                                window_title);
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
            else if (event.type == sf::Event::KeyPressed 
                    || event.type == sf::Event::KeyReleased)
            {
                bool pressed = false;
                unsigned char keycode=0;
                if (event.type == sf::Event::KeyPressed) pressed = true;
                if (event.key.code>=sf::Keyboard::A && 
                    event.key.code<=sf::Keyboard::Z)
                {
                    if (event.key.shift)
                        keycode=event.key.code+'A';
                    else
                        keycode=event.key.code+'a';
                }
                else if (event.key.code>=sf::Keyboard::Num0 && 
                        event.key.code<=sf::Keyboard::Num9)
                {
                    keycode=event.key.code-sf::Keyboard::Num0+'0';
                }
                else 
                {
                    switch (event.key.code)
                    {
                        // NOTE: Changes between SFML 2.0 and 2.1
                        case sf::Keyboard::BackSpace: 
                            keycode=keyboard::BACKSPACE;
                            break;
                        case sf::Keyboard::Return:
                            keycode=keyboard::RETURN;
                            break;
                        case sf::Keyboard::Insert:
                            keycode=keyboard::INSERT;
                            break;
                        case sf::Keyboard::Delete:
                            keycode=keyboard::DELETE;
                            break;
                        case sf::Keyboard::Up:
                            keycode=keyboard::ARROW_UP;
                            break;
                        case sf::Keyboard::Down:
                            keycode=keyboard::ARROW_DOWN;
                            break;
                        case sf::Keyboard::Left:
                            keycode=keyboard::ARROW_LEFT;
                            break;
                        case sf::Keyboard::Right:
                            keycode=keyboard::ARROW_RIGHT;
                            break;
                        case sf::Keyboard::RShift:
                        case sf::Keyboard::LShift:
                            keycode=keyboard::SHIFT;
                            break;
                        case sf::Keyboard::RControl:
                        case sf::Keyboard::LControl:
                            keycode=keyboard::CONTROL;
                            break;
                        case sf::Keyboard::F1:
                            if (debug && pressed)
                            {
                              std::cout << disassembly(dcpu->getMem()
                                                    +dcpu->GetPC(),3);
                              std::cout << std::endl;
                              dcpu->step();
                            }
                            break;
                        case sf::Keyboard::F2:
                            if (debug && !pressed)
                            {
                              printf("A : 0x%04X | B : 0x%04X | C : 0x%04X\n",
                                                dcpu->ra,dcpu->rb,dcpu->rc);
                              printf("X : 0x%04X | Y : 0x%04X | Z : 0x%04X\n",
                                                dcpu->rx,dcpu->ry,dcpu->rz);
                              printf("I : 0x%04X | J : 0x%04X | IA: 0x%04X\n",
                                                dcpu->ri,dcpu->rj,dcpu->ria);
                              printf("PC: 0x%04X | SP: 0x%04X | EX: 0x%04X\n",
                                                dcpu->rpc,dcpu->rsp,dcpu->rex);
                            }
                            break;
                        case sf::Keyboard::F3: 
                            //No need to be in debug mode for this one
                            if (!pressed)
                            {
                                std::cout << "Reset dcpu" << std::endl;
                                dcpu->reset();
                                dcpu->loadProgram (data, size);
                            }
                            break;
                        case sf::Keyboard::F12:
                            if (!pressed)
                            {
                                debug = !debug;
                            }
                            break;
                            
                        default: break;
                    }
                }
                if (keycode)
                    gkeyboard->pushKeyEvent(pressed,keycode);
                
            }
        }
        
        ///DCPU emulation stuff
        monitor->prepareRender();
        const float delta=clock.getElapsedTime().asSeconds();
        clock.restart();
        unsigned int tick_needed=(float)dcpu->cpu_clock*delta;
        
        if (!debug)
        {
            if (tick_needed > dcpu->cpu_clock/60)
                tick_needed = dcpu->cpu_clock/60;
            dcpu->tick(tick_needed);
        }
        /*border_add = monitor->borderSize();
        sprite.setPosition(sf::Vector2f(border_add,border_add));
        border_add *=2;
        
        
        float r_width = border_add + monitor->getScreen().getSize().x;
        float r_height = border_add + monitor->getScreen().getSize().y;
        border.setSize(sf::Vector2f(r_width,r_height));
        border.setFillColor(monitor->getBorder());*/
        
        //For emulations modes and windows resizes
        //sf::FloatRect r(0,0,r_width,r_height);
        //window.setView(sf::View(r));
        window.setActive(true);
        
        
        texture.loadFromImage(*screen); //Slow function
        sprite.setTexture(texture);
        sprite.scale(
                monitor->phyWidth() / (float)(monitor->width() ) ,
                monitor->phyHeight() / (float)(monitor->height()) );
        sprite.setPosition(monitor->borderSize(), monitor->borderSize());

        window.clear(monitor->getBorder()); //good idea
        window.draw(sprite);
        window.display();
        window.setActive(false);
    }
    return 0;
}


