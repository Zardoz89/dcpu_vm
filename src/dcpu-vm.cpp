#include <iostream>
#include <cstdint>
#include <algorithm>
#include <memory>
#include <vector>
#include <cstdio>
#include <chrono>
#include <cmath>

#include "config.hpp"

// Machine core
#include <dcpu/dcpu_opcodes.hpp>
#include <dcpu/dcpu.hpp>

// Devices
#include <dcpu/devices.hpp>

// Modules that sues SFML
#include <sfml/square_gen.hpp>
#include <sfml/KeyboardWindow.hpp>
#include <sfml/MonitorWindow.hpp>

// Util
#include <dcpu/disassembler.hpp>
#include <binasm.hpp>

#include <tinyxml2/tinyxml2.h>

#define FRAMERATE   50

using namespace cpu;
    
// Containers of devices and windows
std::vector<std::shared_ptr<AbstractMonitor>> monitors;
std::vector<std::shared_ptr<windows::AbstractWindow>> wins;
std::vector<std::shared_ptr<m35fd::M35FD>> fdrives;
std::vector<std::shared_ptr<m35fd::M35_Floppy>> floppies;
std::vector<std::string> floppy_files;

std::vector<std::shared_ptr<sf::SoundStream>> sound_streams;

auto dcpu = std::make_shared<DCPU>();

bool custom_loadout = false; /// Use a custom loadout
bool no_sound=false;        /// Disable audio/Speaker

// Process loadout from a XML document
void process_loadout_xml(tinyxml2::XMLDocument& doc, bool devkit = false);

void print_help(std::string program_name)
{
    using namespace std;
    cout << _PRG_NAME << " " << _VERSION << std::endl;
    cout << "usage : " << program_name << " [-options] <dcpu16-exe>";
    cout << endl;
    cout << "--------------------------------------------------------";
    cout << endl;
    cout << "  options:";
    cout << endl;
    cout << "    -assemble (-a) : assemble before load (experimental)";
    cout << endl;
    cout << "    -output <filename> (-o) : output assembled filename";
    cout << endl;
    cout << "    -debug (-d) : start in debug mode";
    cout << endl;
    cout << "            F1  : next step";
    cout << endl;
    cout << "            F2  : print CPU status";
    cout << endl;
    cout << "            F3  : reset (no need debug mode)";
    cout << endl;
    cout << "            F4  : dump the ram";
    cout << endl;
    cout << "            F9  : ejects/inserts floppy";
    cout << endl;
    cout << "            F10 : switch debug/run";
    cout << endl;
    cout << "    -time (-t) : use timed emulation (else refresh based)";
    cout << endl;
    cout << "    -vsync (-v) : use vertical synchronisation";
    cout << endl;
    cout << "                    (more accurate but may bug)";
    cout << endl;
    cout << "  Hardware loadout options:";
    cout << endl;
    cout << 
"    -loadout (-l) : XML file that descrives hardware loadout. If isn't using";
    cout << endl <<
"            a loadout file, then a default loadout will be used. The default";
    cout << endl <<
"            loadout contains a generic clock, a generic keyboard, a monitor,";
    cout << endl <<
"            a floppy drive and a sound speaker.";
    cout << endl << 
"            The next options will be used to change the default loadout of";
    cout << endl << 
"            the virtual machine.";
    cout << endl;
    cout << "    --monitor=<monitor_name> : use the following monitor";
    cout << endl;
    cout << "            1802 -> Lem1802 (default) [c] (-1802)";
    cout << endl;
    cout << "            1803 -> Lem1803 [c] (-1803)";
    cout << endl;
    cout << "            cgm -> Colour Graphics Monitor (-cgm)";
    cout << endl;
    cout << "            [c] : compatible with Lem1802 0x10c programs";
    cout << endl;
    cout << "    --no-sound : disable the sound speaker device";
    cout << endl;
    cout << "    -floppy <filenames...> (-fd) : floppy image files";
    cout << endl;
}

int main (int argc, char **argv)
{
    // TODO Debug level should be set at compile time, or have a hidden program
    // option to set this, like the -v -vv -vvv flags
    logger::LOG_level = logger::LogLevel::INFO; 

    std::string filename;
    std::string loadout_filename;
    std::string outname="a.out"; //output assembled filename
    int monitor_type=0; 
    bool debug=false;
    //use vsync for refresh the screen (more accurate than setFrameLimit)
    bool use_vsync=false; 
    //use time emulation based on sf::Clock; 
    bool use_time=false; 
    //need assemble the file
    bool assemble=false; 
    
    //TODO make a function that parse argument into a program struct
    for (int k=1; k < argc; k++) { //parse arguments
        std::string disk_filename;

        if (argv[k][0] == '-') {
            std::string opt = argv[k];
            
            if (opt=="--help"||opt=="-help"||opt=="-h") {
                std::string pn = argv[0];
                pn.erase(0,pn.find_last_of('\\')+1); //windows
                pn.erase(0,pn.find_last_of('/')+1); //linux
                print_help(pn);
                return 0;
            } else if (opt=="-debug") debug=true;
              else if (opt=="-1802"||opt=="--monitor=1802") monitor_type=0;
              else if (opt=="-1803"||opt=="--monitor=1803") monitor_type=1; 
              else if (opt=="-cgm"||opt=="--monitor=cgm") monitor_type=2;
              else if (opt.find("--monitor") != std::string::npos) {
                LOG_WARN << "Unknow monitor type " + opt;
            } else if (opt=="--no-sound") {
                no_sound=true;
            } else if (opt == "-vsync" || opt == "-v") use_vsync=true;
              else if (opt == "-time" || opt == "-t") use_time=true;
              else if (opt == "-assemble" || opt == "-a") assemble=true;
              else if ((opt == "-output" || opt == "-o") && argc > k+1) {
                assemble=true;
                outname = argv[++k];
            } else if (opt == "-output" || opt == "-o") {
                LOG_WARN << "Option " + opt +
                        " requierd another argument it will be ignored here";
            } else if ((opt == "-floppy" || opt == "-fd") && argc >= k+1) {
                k++;
                do {
                    disk_filename = argv[k];
                    if (disk_filename[0] == '-')
                        break;

                    floppy_files.push_back(disk_filename);
                } while (++k < argc);
            } else if (opt == "-floppy" || opt == "-fd") {
                LOG_WARN << "Option " + opt + 
                        " requierd another argument it will be ignored here";
            } else if ((opt == "-loadout" || opt == "-l") && argc > k+1) {
                loadout_filename = argv[++k];
            } else if (opt == "-loadout" || opt == "-l") {
                LOG_WARN << "Option " + opt + 
                        " requierd another argument it will be ignored here";
            } else {
                LOG_WARN << "Unknow option " + opt + " it will be ignored !";
            }

        } else {
            filename = argv[k];
        }
    }
  

    // Find extension of input file
    auto ext_pos= filename.rfind(".");
    if (ext_pos != std::string::npos) {
        auto extension = filename.substr(ext_pos);
        if (extension == ".dasm" || extension == ".asm" || 
                extension == ".dasm16" || extension == ".10c") {
            // Assembly files
            assemble = true;
        }
    }

    tinyxml2::XMLDocument xml; // Loadout from XML file
    if (loadout_filename.size() > 0) {
        auto ext_pos= loadout_filename.rfind(".");
        bool devkit = false;
        if (ext_pos != std::string::npos) {
            auto extension = loadout_filename.substr(ext_pos);
            if (extension == ".10csln") {
                devkit = true;
            }
        } 
        
        // XML loadout of devices
        if (xml.LoadFile(loadout_filename.c_str()) 
                == tinyxml2::XML_NO_ERROR) {
            process_loadout_xml(xml, devkit);
        }
    }

    if (assemble) {
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
   
    // Default laodout
    if (!custom_loadout) {
        LOG << "Loading devices. Using default loadout";
        std::string window_title="dcpu_vm";
        auto gclock = std::make_shared<Generic_Clock>();
        dcpu->attachHardware (gclock);
       
        // Generic Keyboard
        auto gkeyboard = std::make_shared<keyboard::GKeyboard>();
        dcpu->attachHardware (gkeyboard);
        auto keyb_win = std::make_shared<windows::KeyboardWindow>(gkeyboard);
        wins.push_back(keyb_win);

        // Prepare the speaker device
        auto speaker = std::make_shared<speaker::Speaker>();
        if (!no_sound) {
            dcpu->attachHardware (speaker);
            auto gen = std::make_shared<audio::SquareGenerator>();
            sound_streams.push_back(gen);
            gen->prepare();
            gen->play();
            speaker->setFreqCallback(audio::SquareGenerator::WrappeCallback,
                (void*)gen.get());
        }

        // Floppy drive
        auto fd = std::make_shared<m35fd::M35FD>();
        fdrives.push_back(fd);
        dcpu->attachHardware (fd);
        std::string fname = "disk.dsk";
        if (!floppy_files.empty())
            fname = floppy_files.front();
        auto floppy = std::make_shared<m35fd::M35_Floppy>(fname);
        floppies.push_back(floppy);

        // Sets appropriated monitor
        std::shared_ptr<AbstractMonitor> monitor;
        auto splash_file = "assets/lem_splash.png";
        switch (monitor_type) {
            case 1:
                monitor=std::make_shared<lem::Lem1803>();
                LOG << "use Lem1803 Monitor";
                window_title = "Lem 1803";
                break;
            case 2:
                monitor=std::make_shared<cgm::CGM>();
                LOG << "use CGM Monitor";
                window_title = "CGM";
                splash_file = "assets/cgm_splash.png";
                break;
            default :
                monitor=std::make_shared<lem::Lem1802>();
                LOG << "use Lem1802 Monitor";
                window_title = "Lem 1802";
                break;
        }
        
        dcpu->attachHardware (monitor);
        monitors.push_back(monitor);
        auto window = std::make_shared<windows::MonitorWindow>(monitor, 
                window_title, FRAMERATE);
        wins.push_back(window);
        window->setSplashImage(splash_file);
    }
    
    LOG << "Number of floppy images files: " << floppies.size();

    if (use_vsync) {
        LOG_WARN << "vsync activated may bug";
        for (auto it = wins.begin(); it != wins.end(); ++it) {
            (*it)->setVerticalSyncEnabled(true);
        }
    }
    
    dcpu->reset();
    dcpu->loadProgramFromFile(filename);

    LOG << "Entering main loop";
    sf::Clock clock; 
    unsigned long ticks_counter = 0;
    bool pressed_key = false; // Used to emulate keyDown event
    bool is_open = true;
    bool have_focus;
    while (is_open) {
        have_focus = false;
        // Does all windows stuff
        for (auto it = wins.begin(); it != wins.end(); ++it) {
            is_open = is_open && (*it)->isOpen();
            have_focus = have_focus || (*it)->haveFocus();
            (*it)->handleEvents();
            (*it)->display();
        }
        
        // Checks general keyboard events
        if (have_focus) {
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
            } else if (sf::Keyboard::isKeyPressed (sf::Keyboard::F4)) {
                if (!pressed_key ) {
                    LOG << "Ram dumped in \"ram_dump:\"";
                    FILE* f=fopen("ram_dump","wb");
                    fwrite((const char*) dcpu->getMem(),1,cpu::RAM_SIZE*2,f);
                    fclose(f);
                }

                pressed_key = true;
            } else if (sf::Keyboard::isKeyPressed (sf::Keyboard::F9)) {
                if (!pressed_key ) {
                    int fd_count = 0;
                    // TODO: We need a more powerfull way to handle this
                    // A manager or similar
                    for (auto it = fdrives.begin(); it != fdrives.end(); ++it){
                        if ((*it)->getState() == m35fd::STATE_CODES::NO_MEDIA 
                                && !floppies.empty()) {
                            (*it)->insertFloppy( floppies.at(fd_count++)); 
                        } else {
                            (*it)->eject();
                        }
                    }
                }

                pressed_key = true;
            } else if (sf::Keyboard::isKeyPressed (sf::Keyboard::F10)) {
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

        
        // DCPU emulation stuff -----------------------------------------------
        for (auto it = monitors.begin(); it != monitors.end(); ++it) {
            (*it)->prepareRender();
        }

        // T period of a 100KHz signal = 10 microseconds
        const auto delta=clock.getElapsedTime().asMicroseconds(); 
        clock.restart();
        if (!debug)
        {
            unsigned int tick_needed;
			double tmp;
            if (use_time) {
                tmp = delta / 10.0f; //trash fix Visual don't know the function round lol!
            } else {
                tmp = dcpu->getClock() / (double)(FRAMERATE);
            }
			tick_needed= (unsigned int)(tmp+0.5); 
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

    }

    // Stops audio generator/streams
    for (auto it = sound_streams.begin(); it != sound_streams.end(); ++it){
        (*it)->stop();
    }
    
    return 0;
}


void process_loadout_xml(tinyxml2::XMLDocument& doc, bool devkit) 
{
    using namespace tinyxml2;
    const XMLNode* node = NULL;

    LOG << "Parsing XML file. DevKit solution: " << devkit ;

    if (devkit) {
        // Finds the first loadout element
        node = doc.FirstChildElement("solution");
        if (node == NULL) {
            LOG_WARN << "Invalid DevKit solution file." 
                     << "Trying parse as XML loadout file";
            devkit = false;

            // Try to parse as normal XML
            node = doc.FirstChildElement("loadout");
            if (node == NULL) {
                LOG_WARN << "Invalid loadout file."; 
                return;
            }
            goto end_devkit; // Try to parse as normal XML
        } 

        node = node->FirstChildElement("loadout");
        if (node == NULL) {
            LOG_WARN << "Invalid DevKit solution file.";
            return;
        }

        node = node->FirstChildElement("loadout");
        if (node == NULL) {
            LOG_WARN << "Invalid DevKit solution file."; 
            return;
        }

        end_devkit:
            ;
    } else {
        node = doc.FirstChildElement("loadout");
        if (node == NULL) {
            LOG_WARN << "Invalid loadout file.";
            return;
        }
    }
    // Node points to <loadout>

    node = node->FirstChildElement("hardware");
    if (node == NULL) {
        LOG_WARN << "Invalid loadout file.";
        return;
    }

    std::string defname = "disk";
    unsigned fd_counter = 0;

    for ( auto dev = node->FirstChildElement(); dev != NULL; 
            dev = dev->NextSiblingElement()) {
        std::string name = dev->Name();
        if (name != "device")
            continue;

        if (dev->Attribute("type", "GenericClock")) {
            auto gclock = std::make_shared<Generic_Clock>();
            dcpu->attachHardware (gclock);
        } else if (dev->Attribute("type", "GenericKeyboard")) {
            auto gkeyboard = std::make_shared<keyboard::GKeyboard>();
            dcpu->attachHardware (gkeyboard);
            auto keyb_win = 
                std::make_shared<windows::KeyboardWindow>(gkeyboard);
            wins.push_back(keyb_win);
        // Monitors -----------------------------------------------------------
        } else if (dev->Attribute("type", "LEM1802")) {
            auto monitor = std::make_shared<lem::Lem1802>();
            dcpu->attachHardware (monitor);
            monitors.push_back(monitor);
            
            auto window = std::make_shared<windows::MonitorWindow>(monitor, 
                "LEM 1802", FRAMERATE);
            wins.push_back(window);
            window->setSplashImage("assets/lem_splash.png");
        } else if (dev->Attribute("type", "LEM1803")) {
            auto monitor = std::make_shared<lem::Lem1803>();
            dcpu->attachHardware (monitor);
            monitors.push_back(monitor);
            
            auto window = std::make_shared<windows::MonitorWindow>(monitor, 
                "LEM 1803", FRAMERATE);
            wins.push_back(window);
            window->setSplashImage("assets/lem_splash.png");
        } else if (dev->Attribute("type", "CGM1084")) {
            auto monitor = std::make_shared<cgm::CGM >();
            dcpu->attachHardware (monitor);
            monitors.push_back(monitor);
            
            auto window = std::make_shared<windows::MonitorWindow>(monitor, 
                "CGM 1084", FRAMERATE);
            wins.push_back(window);
            window->setSplashImage("assets/cgm_splash.png");

        // Audio --------------------------------------------------------------
        } else if (dev->Attribute("type", "SimpleSpeaker")) {
            auto speaker = std::make_shared<speaker::Speaker>();
            dcpu->attachHardware (speaker);
            if (!no_sound) {
                auto gen = std::make_shared<audio::SquareGenerator>();
                sound_streams.push_back(gen);
                gen->prepare();
                gen->play();
                speaker->setFreqCallback(
                        audio::SquareGenerator::WrappeCallback, 
                            (void*)gen.get() );
            }

        // Storage ------------------------------------------------------------
        } else if (dev->Attribute("type", "M35FD")) {
            auto fd = std::make_shared<m35fd::M35FD>();
            fdrives.push_back(fd);
            dcpu->attachHardware (fd);

            std::string fname;
            if (floppy_files.size() <= fd_counter) {
                char tmp[10];
                // Generates filename "diskX.dsk"
                std::string fname = defname;
                snprintf(tmp, 10, "%u", fd_counter);
                fname.append(tmp);
                fname.append(".dsk");
                LOG << fname;
                auto floppy = std::make_shared<m35fd::M35_Floppy>(fname);
                floppies.push_back(floppy);
            } else {
                fname = floppy_files.front();
                auto floppy = std::make_shared<m35fd::M35_Floppy>(fname);
                floppies.push_back(floppy);
            }
            fd_counter++;

        }
        

    }

    LOG << "Custom loadout";
    custom_loadout = true;
}

