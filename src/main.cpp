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
//#include <devices/lem1803.hpp>
//#include <devices/cgm.hpp>

#include <devices/m35fd.hpp>

#include <devices/speaker.hpp>

// Modules that sues SFML
#include <sfml/square_gen.hpp>
#include <sfml/KeyboardWindow.hpp>
#include <sfml/MonitorWindow.hpp>

// Util
#include <disassembler.hpp>
#include <binasm.hpp>

#define FRAMERATE   50

using namespace cpu;

void print_help(std::string program_name)
{
    std::cout << _PRG_NAME << " " << _VERSION << std::endl;
    std::cout << "usage : " << program_name << " [-options] <dcpu16-exe>\n";
    std::cout << "--------------------------------------------------------\n";
    std::cout << "  options:" << std::endl;
    std::cout << "    -assemble (-a) : assemble before load (experimental)\n";
    std::cout << "    -debug (-d) : start in debug mode\n";
    std::cout << "            F1  : next step" << std::endl;
    std::cout << "            F2  : print CPU status" << std::endl;
    std::cout << "            F3  : reset (no need debug mode)" << std::endl;
    std::cout << "            F9  : ejects/inserts floppy" << std::endl;
    std::cout << "            System + F12 : switch debug/run" << std::endl;
    std::cout << "    --monitor=<monitor_name> : use the following monitor\n";
    std::cout << "            1802 -> Lem1802 (default) [c] (-1802)\n";
    std::cout << "            1803 -> Lem1803 [c] (-1803)" << std::endl;
    std::cout << "            cgm -> Colour Graphics Monitor (-cgm)\n";
    std::cout << "            [c] : compatible with Lem1802 0x10c programs\n";
    std::cout << "    --no-sound : disable the sound speaker device\n";
    std::cout << "    -output <filename> (-o) : output assembled filename\n";
    std::cout << "    -floppy <filename> (-fd) : floppy image file\n";
    std::cout << "    -time (-t) : use timed emulation (else refresh based)\n";
    std::cout << "    -vsync (-v) : use vertical synchronisation\n";
    std::cout << "                    (more accurate but may bug)\n";
}

int main (int argc, char **argv)
{
    // TODO Debug level should be set at compile time, or have a hidden program
    // option to set this, like the -v -vv -vvv flags
    logger::LOG_level = logger::LogLevel::INFO; 

    std::string filename;
    std::string disk_filename="disk.dsk"; // Floppy disk image
    std::string outname="a.out"; //output assembled filename
    int monitor_type=0; 
    bool debug=false;
    //use vsync for refresh the screen (more accurate than setFrameLimit)
    bool use_vsync=false; 
    //use time emulation based on sf::Clock; 
    bool use_time=false; 
    //need assemble the file
    bool assemble=false; 
    //disable speaker
    bool no_sound=false;
    
    //TODO make a function that parse argument into a program struct
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
                LOG_WARN << "Unknow monitor type " + opt;
            }
            else if (opt=="--no-sound")
            {
                no_sound=true;
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
                LOG_WARN << "Option " + opt +
                        " requiert another argument it will be ignored here";
            }
            else if ((opt == "-floppy" || opt == "-fd") && argc > k+1)
            {
                disk_filename = argv[k+1];
                k++;
            }
            else if (opt == "-floppy" || opt == "-fd")
            {
                LOG_WARN << "Option " + opt + 
                        " requiert another argument it will be ignored here";
            }
            else
            {
                LOG_WARN << "Unknow option " + opt + " it will be ignored !";
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
        LOG_ERROR << "Missing input file, type --help for list options";
        return 0;
    }
   
    
    LOG << "Loading devices";
    sf::String window_title="dcpu_vm";
    auto dcpu = std::make_shared<DCPU>();
    auto gclock = std::make_shared<Generic_Clock>();
    auto gkeyboard = std::make_shared<keyboard::GKeyboard>();

    // Prepare the speaker device
    auto speaker = std::make_shared<speaker::Speaker>();
    audio::SquareGenerator gen;
    gen.prepare();
    gen.play();
    speaker->setFreqCallback(audio::SquareGenerator::WrappeCallback,
            (void *)(&gen));

    // Floppy drive
    auto fd = std::make_shared<m35fd::M35FD>();
    auto floppy = std::make_shared<m35fd::M35_Floppy>(disk_filename);
    fd->insertFloppy(floppy);

    // Sets appropriated monitor
    std::shared_ptr<AbstractMonitor> monitor;
    switch (monitor_type)
    {
//        case 1:
//            monitor=std::make_shared<lem::Lem1803>();
//            LOG << "use Lem1803 Monitor";
//            window_title = "Lem 1803";
//            break;
//        case 2:
//            monitor=std::make_shared<cgm::CGM>();
//            LOG << "use CGM Monitor";
//            window_title = "CGM";
//            break;
        default :
            monitor=std::make_shared<lem::Lem1802>();
            LOG << "use Lem1802 Monitor";
            window_title = "Lem 1802";
            break;
    }
    
    dcpu->attachHardware (gclock);
    dcpu->attachHardware (monitor);
    dcpu->attachHardware (gkeyboard);
    dcpu->attachHardware (fd);
    if (!no_sound)
        dcpu->attachHardware (speaker);
    dcpu->reset();
    dcpu->loadProgramFromFile(filename);

    sf::Clock clock; 
    windows::MonitorWindow window(monitor, window_title, FRAMERATE);
    if (use_vsync) {
        LOG_WARN << "vsync activated may bug";
        window.setVerticalSyncEnabled(true);
    }

    
    // We use a window to show a fake keyboard and capture keyboard
    // events if it have focus
    windows::KeyboardWindow keyb_win(gkeyboard);

    LOG << "Entering main loop";
    unsigned long ticks_counter = 0;
    bool pressed_key = false; // Used to emulate keyDown event
    while (window.isOpen() && keyb_win.isOpen()) {
    
        // Process events
        keyb_win.handleEvents();
        window.handleEvents();

        // Checks general keyboard events
        if (keyb_win.haveFocus() || window.haveFocus()) {
            if (debug && sf::Keyboard::isKeyPressed (sf::Keyboard::F1)) {
                if (!pressed_key ) {
                    std::cout << disassembly(dcpu->getMem()
                                          +dcpu->getPC(),3);
                    std::cout << std::endl;
                    dcpu->step();
                }

                pressed_key = true;
            } else if (debug && sf::Keyboard::isKeyPressed
                                        (sf::Keyboard::F2)) {
                if (!pressed_key ) {
                    printf("A : 0x%04X | B : 0x%04X | C : 0x%04X\n",
                                      dcpu->getA(),dcpu->getB(),dcpu->getC());
                    printf("X : 0x%04X | Y : 0x%04X | Z : 0x%04X\n",
                                      dcpu->getX(),dcpu->getY(),dcpu->getZ());
                    printf("I : 0x%04X | J : 0x%04X | IA: 0x%04X\n",
                                      dcpu->getI(),dcpu->getJ(),dcpu->getIA());
                    printf("PC: 0x%04X | SP: 0x%04X | EX: 0x%04X\n",
                                      dcpu->getPC(),dcpu->getSP(),dcpu->getEX());
                    if (dcpu->isQueueing())
                        printf("Interrupts being push to the queue\n");
                    if (dcpu->getOnFire())
                        printf("Catch Fire!\n");
                }

                pressed_key = true;
            } else if (sf::Keyboard::isKeyPressed (sf::Keyboard::F3)) {
                if (!pressed_key ) {
                    LOG << "Reset dcpu";
                    dcpu->reset();
                    dcpu->loadProgramFromFile(filename);
                }

                pressed_key = true;
            } else if (sf::Keyboard::isKeyPressed (sf::Keyboard::F9)) {
                if (!pressed_key ) {
                    if (fd->getState() == m35fd::STATE_CODES::NO_MEDIA) {
                        fd->insertFloppy(floppy);
                    } else {
                        fd->eject();
                    }
                }

                pressed_key = true;
            } else if (sf::Keyboard::isKeyPressed (sf::Keyboard::F12) &&
                    (sf::Keyboard::isKeyPressed (sf::Keyboard::LSystem) ||
                     sf::Keyboard::isKeyPressed (sf::Keyboard::RSystem) )) {
                if (!pressed_key ) {
                    debug = !debug;
                    if (debug)
                        LOG << "Debug mode activated";
                    else
                        LOG << "Debug mode deactivated";
                }

                pressed_key = true;
            } else {
                pressed_key = false;
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
                tick_needed= tmp+0.5; //trash fix Visual don't know the function round lol!
            } else {
                double tmp = dcpu->getClock() / (double)(FRAMERATE);
                tick_needed= tmp+0.5;
            }
            ticks_counter += tick_needed;

            // Outputs every second (if runs to ~100% of speed)
            if (ticks_counter > 100000) {
                ticks_counter -= 100000;
                //std::cerr << "Delta: " << delta << " ms ";
                //std::cerr << "Ticks: " << tick_needed << "   ";
                //double tmp = tick_needed*(1 / (double)(dcpu->getClock()));
                //int64_t rtime = 1000000 * tmp;
                //std::cerr << "Speed: " << (float)(100*delta)/rtime << std::endl;
            }

            dcpu->tick(tick_needed);
        }

        
        window.display();
        keyb_win.display();

    }

    gen.stop();
    return 0;
}


