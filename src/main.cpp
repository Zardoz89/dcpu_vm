#include <iostream>
#include <fstream>
#include <cstdint>
#include <algorithm>
#include <memory>
#include <thread>

#include <stdio.h>
#include <chrono>

#include "dcpu.hpp"
#include "disassembler.hpp"
#include "fake_lem1802.hpp"

using namespace cpu;

const long TOTALCPUS    = 215*16;
const long PERTHREAD    = 16;  // At 16 looks that is the ideal for the FX-4100
#define THREADS           (TOTALCPUS/PERTHREAD)
const long CYCLES       = 1000*1000;

const int BATCH         = 10;


std::vector<std::vector<std::shared_ptr<DCPU>>> threads;
uint16_t* data;
size_t size = 0;

void benchmark();
void step();
void one_bench();

void cpu_in_thread(int n);


int main (int argc, char **argv)
{

    char* filename;
    std::ifstream binfile;
    
   /* std::cout << "cpu " << sizeof(DCPU) << " IHardware " << sizeof(IHardware);
    std::cout << " fake_LEM1802 " << sizeof(Fake_Lem1802) << std::endl;*/
    
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
    std::cout << "\tb -> benchmark  s -> step execution o-> benchmark one VM";
    std::cout << std::endl << std::endl;
    char choose;
    std::cin >> choose;
    
    if (choose == 'b' || choose == 'B') {
        benchmark();
    } else if ( choose == 's' || choose == 'S') {
        step();
    } else if ( choose == 'o' || choose == 'O') {
        one_bench();
    } else {
        goto badchar; /// HATE ME!!!!
    }

    delete[] data;

    return 0;
}


void benchmark() 
{
    // Load program to all CPUs
    for (int u=0; u< THREADS; u++) {
        std::vector<std::shared_ptr<DCPU>> cpus;
        cpus.reserve (PERTHREAD);
        for (int i = 0; i< PERTHREAD; i++) {
            auto cpu = std::make_shared<DCPU>();   
            auto screen = std::make_shared<Fake_Lem1802>();
            screen->setEnable(false); // We not desire to write to stdout
            cpu->attachHardware (screen);
            cpu->reset();
            cpu->loadProgram (data, size);
            
            cpus.push_back(cpu);
        }

        threads.push_back(cpus);
        
    }
    
    std::thread tds[THREADS];

    printf("Threads %ld\t CPU PerThread %ld\t", THREADS, PERTHREAD);
    printf("N cpus %ld\n", PERTHREAD * THREADS);
    printf("Cycles %ld\n", CYCLES);
    
    auto start = std::chrono::high_resolution_clock::now(); 
    
    for (int i=0; i< THREADS; i++) {
        tds[i] = std::thread(cpu_in_thread, i);
    }
    
    for (int i=0; i< THREADS; i++) {
        tds[i].join();
    }

	auto end = std::chrono::high_resolution_clock::now();
    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(end - start); 
	std::cout << "Measured time: " << dur.count() << "ms" << std::endl;
    
}

void step() {
    using namespace std;
    auto cpu = make_shared<DCPU>();
    auto screen = make_shared<Fake_Lem1802>();
    cpu->attachHardware (screen);
    cpu->reset();
    cpu->loadProgram (data, size);
    
    
    char c = getchar();
    while (1) {
        c = getchar();
        if (c == 'f' || c == 'q' || c == '\n' )
            break;
    }
    
    while (c != 'q') {
   
        cout << cpu->dumpRegisters() << endl;
        cout << "T cycles " << dec << cpu->getTotCycles() << endl;
        cout << "> " << cpu->dumpRam() << " - ";
        string s = disassembly(cpu->getMem() + cpu->GetPC(), 3);
        cout << s << endl;
        
        if (cpu->GetSP() != 0x0000)
            cout << "STACK : "<< cpu->dumpRam(cpu->GetSP(), 0xFFFF) << endl;
        
        if (c == 'f') {
            for (int i = 0; i < 100; i++)
                cpu->tick();
        } else {
            if (cpu->tick())
                cout << "Execute! ";
        }
        cout << endl;
            
        
        
        while (1) {
            c = getchar();
            if (c == 'f' || c == 'q' || c == '\n' )
                break;
        }
        
    }
    
}


void one_bench() {
    using namespace std;
    using namespace std::chrono;
    
    const int times = 200;

    high_resolution_clock::time_point  starts[times];
    high_resolution_clock::time_point  creates[times];
    high_resolution_clock::time_point  loadstarts[times];
    high_resolution_clock::time_point  loads[times];
    high_resolution_clock::time_point  finishs[times];
   
    for (int x=0; x < times; x++) {
        using namespace std::chrono;
        starts[x] = high_resolution_clock::now(); 
        auto cpu = make_shared<DCPU>();

        creates[x] = high_resolution_clock::now(); 
        
        auto screen = make_shared<Fake_Lem1802>();
        screen->setEnable(false); // We not desire to write to stdout
        cpu->attachHardware (screen);
        
        loadstarts[x] = high_resolution_clock::now(); 
        
        cpu->reset();
        cpu->loadProgram (data, size);
        
        loads[x] = high_resolution_clock::now(); 
        
        for (int i=0; i < 10000; i++) {
            cpu->tick();
        }
        finishs[x] = high_resolution_clock::now(); 
        
    }


    double d_create, d_load, d_execute;
    d_create = d_load = d_execute = 0;
    
    for (int x=0; x < times; x++) {

        auto tmp = duration_cast<chrono::microseconds> 
            (creates[x] - starts[x]);
        d_create += tmp.count();
        
        tmp = duration_cast<chrono::microseconds> 
            (loads[x] - loadstarts[x]); 
        d_load += tmp.count();
        
        tmp = duration_cast<chrono::microseconds> 
            (finishs[x] - loads[x]); 
        
        d_execute += tmp.count();
        
    }
    d_create /= times;
    d_load /= times;
    d_execute /= times;

    cout << "Measured time: " << endl;
    cout << "\tCreating time "<< d_create << "us" << endl;
    cout << "\tLoad time "<< d_load << "us" << endl;
    cout << "\tExecute 10k cycles time "<< d_execute << "us" << endl;

}


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



