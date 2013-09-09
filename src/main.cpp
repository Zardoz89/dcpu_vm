#include <iostream>
#include <cstdint>
#include <algorithm>
#include <memory>
#include <stdio.h>
#include <chrono>
#include <cmath>

#include "config.hpp"

// Machine core
#include <dcpu_opcodes.hpp>
#include <dcpu.hpp>

// Devices
#include <devices/gclock.hpp>
#include <devices/gkeyboard.hpp>

#include <devices/lem1802.hpp>
#include <devices/lem1803.hpp>
#include <devices/cgm.hpp>

#include <devices/speaker.hpp>

// Audio
#include <audio/square_gen.hpp>

// Util
#include <disassembler.hpp>
#include <binasm.hpp>


using namespace cpu;

void print_help(std::string program_name)
{
    std::cout << "usage : " << program_name << " [-options] <dcpu16-exe>\n";
    std::cout << "--------------------------------------------------------\n";
    std::cout << "  options:" << std::endl;
    std::cout << "    -assemble (-a) : assemble before load (experimental)\n";
    std::cout << "    -debug (-d) : start in debug mode\n";
    std::cout << "            F1  : next step" << std::endl;
    std::cout << "            F2  : print registers" << std::endl;
    std::cout << "            F3  : reset (no need debug mode)" << std::endl;
    std::cout << "            F12 : switch debug/run" << std::endl;
    std::cout << "    --monitor=<monitor_name> : use the following monitor\n";
    std::cout << "            1802 -> Lem1802 (default) [c] (-1802)\n";
    std::cout << "            1803 -> Lem1803 [c] (-1803)" << std::endl;
    std::cout << "            cgm -> Colour Graphics Monitor (-cgm)\n";
    std::cout << "            [c] : compatible with Lem1802 0x10c programs\n";
    std::cout << "    -output <filename> (-o) : output assembled filename\n";
    std::cout << "    -time (-t) : use timed emulation (else refresh based)\n";
    std::cout << "    -vsync (-v) : use vertical synchronisation\n";
    std::cout << "                    (more accurate but may bug)\n";
}

int main (int argc, char **argv)
{

    std::string filename;
    std::string outname="a.out"; //output assembled filename
    int monitor_type=0; 
    bool debug=false;
    //use vsync for refresh the screen (more accuracte than setFrameLimit)
    bool use_vsync=false; 
    //use time emulation based on sf::Clock; 
    bool use_time=false; 
    //need asssemble the file 
    bool assemble=false; 
    
    //TODO make a fonction that parse argument into a program struct
    for (int k=1; k < argc; k++) //parse arguments
    {
        if (argv[k][0] == '-')
        {
            std::string opt = argv[k];
            
            if (opt=="--help"||opt=="-help"||opt=="-h")
            {
                std::string pn = argv[0];
                pn.erase(0,pn.find_last_of('\\')+1); //windows
                pn.erase(0,pn.find_last_of('/')+1); //linux
                print_help(pn);
                return 0;
            }
            else if (opt=="-debug") debug=true;
            else if (opt=="-1802"||opt=="--monitor=1802") monitor_type=0;
            else if (opt=="-1803"||opt=="--monitor=1803") monitor_type=1; 
            else if (opt=="-cgm"||opt=="--monitor=cgm") monitor_type=2;
            else if (opt.find("--monitor") != std::string::npos)
            {
                std::cout << "warning: unknow monitor type "; 
                std::cout << opt << std::endl;
            }
            else if (opt == "-vsync" || opt == "-v") use_vsync=true;
            else if (opt == "-time" || opt == "-t") use_time=true;
            else if (opt == "-assemble" || opt == "-a") assemble=true;
            else if ((opt == "-output" || opt == "-o") && argc > k+1)
            {
                assemble=true;
                outname = argv[k+1];
                k++;
            }
            else if (opt == "-output" || opt == "-o")
            {
                std::cout << "warning: option " << opt << " requiert";
                std::cout << " another argument it will be ignored here";
            }
            else
            {
                std::cout << "warning: unknow option ";
                std::cout << opt << " it will be ignored !" << std::endl;
            }
        }
        else
        {
            filename = argv[k];
        }
    
    }
    
    if (assemble)
    {
        BinAsm assembler;
        if (!assembler.load(filename)) return 0xdead;
        if (!assembler.assemble()) return 0xdead;
        if (!assembler.resolve_labels()) return 0xdead;
        if (!assembler.save(outname)) return 0xdead;
        filename = outname;
    }
    
    if (argc <= 1 || !filename.size()) {
        std::cerr <<"error: missing input file, type --help for list options\n";
        return 0;
    }
    
    sf::String window_title="dcpu_vm";
    auto dcpu = std::make_shared<DCPU>();
    auto gclock = std::make_shared<Generic_Clock>();
    auto gkeyboard = std::make_shared<keyboard::GKeyboard>();

    // Prepare the speaker device
    auto speaker = std::make_shared<speaker::Speaker>();
    audio::SquareGenerator gen;
    gen.prepare(44100);
    gen.play();
    speaker->setFreqCallback(audio::SquareGenerator::WrappeCallback,
            (void *)(&gen));

    // Sets apropiated monitor
    std::shared_ptr<AbstractMonitor> monitor;
    switch (monitor_type)
    {
        case 1:
            monitor=std::make_shared<lem::Lem1803>();
            std::cout << "use Lem1803 Monitor" << std::endl;
            window_title = "Lem 1803";
            break;
        case 2:
            monitor=std::make_shared<cgm::CGM>();
            std::cout << "use CGM Monitor" << std::endl;
            window_title = "CGM";
            break;
        default :
            monitor=std::make_shared<lem::Lem1802>();
            std::cout << "use Lem1802 Monitor" << std::endl;
            window_title = "Lem 1802";
            break;
    }
    
    dcpu->attachHardware (monitor);
    dcpu->attachHardware (gclock);
    dcpu->attachHardware (gkeyboard);
    dcpu->attachHardware (speaker);
    dcpu->reset();
    dcpu->loadProgramFromFile(filename);
   
    sf::Texture texture; // texture of the screen
    sf::Sprite sprite;   //sprite of the screen
    const sf::Image* screen = monitor->getScreen();
    sf::Clock clock; 
    sf::RenderWindow window;
    float border_add = monitor->borderSize()*2;
    window.create(sf::VideoMode(monitor->phyWidth()+border_add,
                                monitor->phyHeight()+border_add),
                                window_title);
    if (use_vsync)
    {
        std::cout << "warning: vsync activated may bug" << std::endl;
        window.setVerticalSyncEnabled(true);
    }
    else
        window.setFramerateLimit(50);
    
    // We try to use a window to show a fake keyboard and capture keyboard
    // events if it have focus
    sf::RenderWindow keyb_win;
    keyb_win.create(sf::VideoMode(484, 196), "Keyboard", 
            sf::Style::Titlebar | sf::Style::Close);
    keyb_win.setKeyRepeatEnabled(false);
    keyb_win.setActive(false);
    sf::Texture keyb_tx;
    bool keyb_image_loaded = keyb_tx.loadFromFile("assets/keyb_img.png");
    sf::Sprite keyb_sprite;
    if (keyb_image_loaded)
        keyb_sprite.setTexture(keyb_tx);
    else
        std::cerr << "Waring: assets/keyb_img.png not found !" << std::endl;
    bool keyb_focus;
    

    unsigned long ticks_counter = 0;
    while (window.isOpen() && keyb_win.isOpen()) 
    {   //Because non mainthread event are forbidden in OSX
    
        // Process events
        sf::Event event;
        while (keyb_win.pollEvent(event)) {
            // This window capture key events to the VM window if have focus
            if (event.type == sf::Event::Closed) {
                keyb_win.close();

            } else if (event.type == sf::Event::GainedFocus) {
                keyb_focus = true;

            } else if (event.type == sf::Event::LostFocus) {
                keyb_focus = false;

            } else if (keyb_focus && ( event.type == sf::Event::KeyPressed || 
                        event.type == sf::Event::KeyReleased || 
                        event.type == sf::Event::TextEntered )) {
                // Process VM keyboard input
                bool pressed = (event.type == sf::Event::KeyPressed);
                unsigned char keycode=0;

                // Note this works becuase Unicode maps ASCII 7 bit in his
                // first 128 codes
                if (event.type == sf::Event::TextEntered &&
                       ((event.text.unicode >= '!' && 
                         event.text.unicode <= '/') ||
                        (event.text.unicode >= ':' && 
                             event.text.unicode <= '@') ||
                        (event.text.unicode >= '[' && 
                             event.text.unicode <= 0x60) ||
                        (event.text.unicode >= '{' && 
                             event.text.unicode <= 0x7F) )) {
                        
                        gkeyboard->pushKeyEvent(true, 
                                (unsigned char) event.text.unicode);
                        gkeyboard->pushKeyEvent(false, 
                                (unsigned char) event.text.unicode);
                }
                // Ignore silenty any other TextEntered events
                // because SFML generate at same time KeyEvent and TextEntered
                if (event.type == sf::Event::TextEntered) 
                    continue; 
                
                if (event.key.code>=sf::Keyboard::A && 
                    event.key.code<=sf::Keyboard::Z) {
                    if (event.key.shift)
                        keycode=event.key.code+'A';
                    else
                        keycode=event.key.code+'a';
                } else if (event.key.code>=sf::Keyboard::Num0 && 
                        event.key.code<=sf::Keyboard::Num9) {

                    keycode=event.key.code-sf::Keyboard::Num0+'0';
                } else {
                    switch (event.key.code) {
                        case sf::Keyboard::Space:
                            keycode=' ';
                            break;
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
                        case sf::Keyboard::Escape:
                            keycode=keyboard::ESC;
                            break;


                        default:
                            break;
                    }
                }
                if (keycode)
                    gkeyboard->pushKeyEvent(pressed,keycode);
            }
        }

        while (window.pollEvent(event)) 
        {
            // Close window : exit
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            else if (event.type == sf::Event::Resized)
            {
                //Rewrap opengl camera
                float r_width = window.getSize().x;
                float r_height = window.getSize().y;
                sf::FloatRect r(0,0,r_width,r_height);
                window.setView(sf::View(r));
            }
            // NOTE This keyboard events should be handled by sf:Keyboard
            // direclty and only works if any VM window have focus.
            // Actually only wokrs if the monitor window have focus
            else if (event.type == sf::Event::KeyPressed 
                    || event.type == sf::Event::KeyReleased)
            {
                bool pressed = (event.type == sf::Event::KeyPressed);
                switch (event.key.code)
                {
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
                            std::cout << "reset dcpu" << std::endl;
                            dcpu->reset();
                            dcpu->loadProgramFromFile(filename);
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
        }
        
        ///DCPU emulation stuff
        monitor->prepareRender();
        // T period of a 100KHz signal = 10 microseconds
        const auto delta=clock.getElapsedTime().asMicroseconds(); 
        clock.restart();
        
        if (!debug)
        {
            unsigned int tick_needed;
            if (use_time) {
                double tmp = delta / 10.0f;
                tick_needed= std::round(tmp);
            } else {
                double tmp = dcpu->cpu_clock/50;
                tick_needed= std::round(tmp);
            }
            ticks_counter += tick_needed;

            // Outputs every second (if runs to ~100% of speed)
            if (ticks_counter > 100000) {
                ticks_counter -= 100000;
                std::cerr << "Delta: " << delta << " ms ";
                std::cerr << "Ticks: " << tick_needed << "   ";
                double tmp = tick_needed*(1 / (double)(dcpu->cpu_clock));
                int64_t rtime = 1000000 * tmp;
                std::cerr << "Speed: " << (float)(100*delta)/rtime << std::endl;
            }

            dcpu->tick(tick_needed);
        }
        
        
        window.setActive(true);
        
        //Working resizing code
        border_add = monitor->borderSize();
        texture.loadFromImage(*screen); //Slow function
        sprite.setTexture(texture);
        sprite.setScale(  //Warning setScale and scale are different !!
          (float)(window.getSize().x-border_add*2)/(float)(monitor->width()),
          (float)(window.getSize().y-border_add*2)/(float)(monitor->height()));
        sprite.setPosition(sf::Vector2f(border_add,border_add));

        window.clear(monitor->getBorder()); //good idea
        window.draw(sprite);
        window.display();
        window.setActive(false);

        // Updates Keyboard window
        if (keyb_image_loaded) {
            keyb_win.setActive(true);
            keyb_win.clear();
            keyb_win.draw(keyb_sprite);
            keyb_win.display();
            keyb_win.setActive(false);
        }
    }

    gen.stop();
    return 0;
}


