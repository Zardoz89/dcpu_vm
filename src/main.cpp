#include <iostream>
#include <fstream>
#include <cstdint>
#include <algorithm>
#include <memory>
#include <chrono>

#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp> 

//#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Texture.hpp>

#include "dcpu.hpp"
#include "disassembler.hpp"

#include "gclock.hpp"
#include "gkeyboard.hpp"
//#include "fake_lem1802.hpp"
#include "lem1802.hpp"
#include "lem1803.hpp"
#include "cgm.hpp"

using namespace cpu;

/*
const long TOTALCPUS    = 215*16;
const long PERTHREAD    = 16;  // At 16 looks that is the ideal for the FX-4100
#define THREADS           (TOTALCPUS/PERTHREAD)
const long CYCLES       = 1000*1000;

const int BATCH         = 10;
*/

//std::vector<std::vector<std::shared_ptr<DCPU>>> threads;
uint16_t* data;
size_t size = 0;

bool running = true;

//void benchmark();
void step();
void run();

void renderGuy(sf::RenderWindow* win, std::shared_ptr<cpu::AbstractMonitor> mon);

//void cpu_in_thread(int n);


int main (int argc, char **argv)
{

    char* filename;
    std::ifstream binfile;
    
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
   
badchar:
    std::cout << "Select what to do :" << std::endl;
    std::cout << "\ts -> step execution\n\tr-> run";
    std::cout << std::endl << std::endl;
    char choose;
    std::cin >> choose;
    
/*    if (choose == 'b' || choose == 'B') {
        benchmark();
    } else*/ if ( choose == 's' || choose == 'S') {
        step();
    } else if ( choose == 'r' || choose == 'R') {
        run();
    } else {
        goto badchar; /// HATE ME!!!!
    }

    delete[] data;

    return 0;
}

/*
void benchmark() 
{
    // Load program to all CPUs
    for (int u=0; u< THREADS; u++) {
        std::vector<std::shared_ptr<DCPU>> cpus;
        cpus.reserve (PERTHREAD);
        for (int i = 0; i< PERTHREAD; i++) {
            auto cpu = std::make_shared<DCPU>();   
            //auto screen = std::make_shared<Fake_Lem1802>();
            //screen->setEnable(false); // We not desire to write to stdout
            //cpu->attachHardware (screen);
            cpu->reset();
            cpu->loadProgram (data, size);
            
            cpus.push_back(cpu);
        }

        threads.push_back(cpus);
        
    }
    
    sf::Thread* tds[THREADS];

    std::cout << "Threads " << THREADS << "\t CPU PerThread " << PERTHREAD;
    std::cout << "\t N cpus " << PERTHREAD * THREADS << std::endl;
    std::cout << "Cycles " << CYCLES << std::endl;
    
    auto start = std::chrono::high_resolution_clock::now(); 
    
    for (int i=0; i< THREADS; i++) {
        tds[i] = new sf::Thread(cpu_in_thread, i);
        tds[i]->launch();
    }
    
    for (int i=0; i< THREADS; i++) {
        delete tds[i]; //wait() is automatically called
    }

	auto end = std::chrono::high_resolution_clock::now();
    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(end - start); 
	std::cout << "Measured time: " << dur.count() << "ms" << std::endl;
    
}
*/
void step() {
    using namespace std;
    auto cpu = make_shared<DCPU>();
    
    auto screen = make_shared<lem::Lem1803>();
    cpu->attachHardware (screen);
   
    sf::RenderWindow win(sf::VideoMode(
                                screen->phyWidth()  + screen->borderSize()*2,
                                screen->phyHeight() + screen->borderSize()*2),
                            "DCPU-16");
    win.setFramerateLimit(30); 
    
    auto clock = make_shared<Generic_Clock>();
    cpu->attachHardware (clock);
    
    cpu->reset();
    cpu->loadProgram (data, size);
    
    
    char c = getchar();
    while (1) {
        c = getchar();
        if (c == 'f' || c == 'q' || c == '\n' )
            break;
    }
    
    win.setActive(false);
    boost::thread thr_render (renderGuy, &win, 
            std::static_pointer_cast<cpu::AbstractMonitor>(screen));
    
    while (running) {
        cout << cpu->dumpRegisters() << endl;
        cout << "T cycles " << dec << cpu->getTotCycles() << endl;
        cout << "> " << cpu->dumpRam() << " - ";
        string s = disassembly(cpu->getMem() + cpu->GetPC(), 3);
        cout << s << endl;
        
        if (cpu->GetSP() != 0x0000)
            cout << "STACK : "<< cpu->dumpRam(cpu->GetSP(), 0xFFFF) << endl;
        
        if (c == 'f') {
            for (int i = 0; i < 100; i++)
                cpu->step();
                //cpu->tick();
        } else {
            cpu->step();
            /*if (cpu->tick())
                cout << "Execute! ";*/
        }
        cout << endl;

        while (1) {
            c = getchar();
            if (c == 'f' || c == 'q' || c == '\n' )
                break;
        }

        if (c == 'q')
            running = false;
    }


    if (thr_render.joinable())
        thr_render.join();

    
}


void run() {

    using namespace std;
    using namespace std::chrono;

    
    auto cpu = make_shared<DCPU>();
    
    auto screen = make_shared<cgm::CGM>();
    cpu->attachHardware (screen);
   
    sf::RenderWindow win(sf::VideoMode(
                                screen->phyWidth()  + screen->borderSize()*2,
                                screen->phyHeight() + screen->borderSize()*2),
                            "DCPU-16 CGM1084");
    win.setFramerateLimit(30); 

    auto screen2 = make_shared<lem::Lem1803>();
    cpu->attachHardware (screen2);
    
    sf::RenderWindow win2(sf::VideoMode(
                                screen2->phyWidth()  + screen2->borderSize()*2,
                                screen2->phyHeight() + screen2->borderSize()*2),
                            "DCPU-16 LEM1803");
    win2.setFramerateLimit(30); 
    
    auto clock = make_shared<Generic_Clock>();
    cpu->attachHardware (clock);

    auto keyboard = make_shared<keyboard::GKeyboard>();
    cpu->attachHardware (keyboard);
    keyboard->pushKeyEvent(true, 'h');
    keyboard->pushKeyEvent(false, 'h');
    keyboard->pushKeyEvent(true, 'o');
    keyboard->pushKeyEvent(false, 'o');
    keyboard->pushKeyEvent(true, 'l');
    keyboard->pushKeyEvent(false, 'l');
    keyboard->pushKeyEvent(true, 'a');
    keyboard->pushKeyEvent(false, 'a');

    cpu->reset();
    cpu->loadProgram (data, size);
    
    high_resolution_clock::time_point t = high_resolution_clock::now();
    high_resolution_clock::time_point t2; 
  
    win.setActive(false);
    boost::thread thr_render (renderGuy, &win, 
            std::static_pointer_cast<cpu::AbstractMonitor>(screen));

    win2.setActive(false);
    boost::thread thr_render2 (renderGuy, &win2, 
            std::static_pointer_cast<cpu::AbstractMonitor>(screen2));
    
    while (running ) { //&& wincgm.isOpen()) {
        t2 =  high_resolution_clock::now(); 
        
        
        cpu->tick();
        
        auto delta = duration_cast<chrono::nanoseconds>(t2 - t);
        auto rest = nanoseconds(1000000000/cpu->cpu_clock)-delta; 

        if ((cpu->getTotCycles() % 100000) == 0) { 
            // Not show running speed every clock tick 
            double p = nanoseconds(1000000000/cpu->cpu_clock).count() /
                (double)(delta.count() );//+ rest.count());
            cerr << "Delta :" << delta.count() << " ns \t";
            cerr << "Rest :" << rest.count() << " ns ";
            cerr << " Running at "<< p*100.0 << " % speed." << endl;
            cerr << keyboard->bufferSize() << endl;
        }
        t = t2;
    }

    if (thr_render.joinable())
        thr_render.join();

    if (thr_render2.joinable())
        thr_render.join();

    cout << "Finish" << endl;

}



/*

// Runs PERTHREAD cpus, doing CYCLES cycles
void cpu_in_thread(int n) {
    auto cpus = threads[n];
    for (long i=0; i < CYCLES; i+= BATCH) {
        for (auto c = cpus.begin(); c != cpus.end(); c++) {
            for ( int j=0; j < BATCH; j++)
                (*c)->tick();
        }
    }
}
*/

void renderGuy(sf::RenderWindow* win, std::shared_ptr<cpu::AbstractMonitor> mon)
{
    sf::Texture texture;
    
    while (running) {
        win->setActive(true);
        sf::Event event;
        while (win->pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                win->close();
                running = false;
                continue;
            }
        }

        win->clear(mon->getBorder());

        sf::Image* scr = mon->updateScreen();
        texture.create(mon->width(), mon->height());
        texture.loadFromImage(*scr);
        
        sf::Sprite sprite(texture);
        sprite.scale(
                mon->phyWidth()  / (float)(mon->width() ) , 
                mon->phyHeight() / (float)(mon->height()) ); 
        sprite.setPosition(mon->borderSize(), mon->borderSize());

        win->draw(sprite);
        win->display();
        win->setActive(false);
        
        delete scr;
        boost::this_thread::yield();
    }
    
    if (win->isOpen())
        win->close();

}

