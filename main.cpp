#include <iostream>
#include <fstream>
#include <cstdint>
#include <algorithm>
#include <memory>
#include <thread>

#include <stdio.h>
#include <time.h>

#include "dcpu.hpp"
#include "fake_lem1802.hpp"

using namespace cpu;

#define THREADS           (4)
const int PERTHREAD     = 250000;
const long CYCLES       = 1000000;

std::vector<std::shared_ptr<DCPU>>* threads = 
        new std::vector<std::shared_ptr<DCPU>>[THREADS];

void hello(){
    std::cout << "Hello from thread " << std::endl;
}

// Runs PERTHREAD cpus, doing CYCLES cycles
void cpu_in_thread(int n) {
    auto cpus = threads[n];
    for (long i=0; i < CYCLES; i++) {
        for (auto c = cpus.begin(); c != cpus.end(); c++) {
            (*c)->tick();
        }
    }
}

int main (int argc, char **argv)
{

    char* filename;
    std::ifstream binfile;
    uint16_t* data;
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
    
    // Load program to all CPUs
    for (int u=0; u<< THREADS; u++) {
        std::vector<std::shared_ptr<DCPU>> cpus;
        cpus.reserve (PERTHREAD);
        for (int i = 0; i< PERTHREAD; i++) {
            auto cpu = std::make_shared<DCPU>();   
            auto screen = std::make_shared<Fake_Lem1802>();
            cpu->attachHardware (screen);
            cpu->reset();
            cpu->loadProgram (data, size);
            
            cpus.push_back(cpu);
        }

        threads[i] = cpus;
    }
    
    
    //std::cout << cpu->dumpRegisters() << "\n";
    
    /*while (getchar() != 'q') {
        //std::cout << "RAM[PC] = " << cpu->dumpRam() << "\n";
        for (int i = 0; i < 100; i++)
            cpu->tick();
            
        //std::cout << cpu->dumpRegisters() << "\n";
        //std::cout << "T cycles " << cpu->getTotCycles() << "\n";
        //if (cpu->GetSP() != 0x0000)
        //    std::cout << "STACK : "<< cpu->dumpRam(cpu->GetSP(), 0xFFFF) << "\n";
        
        
    }*/
    
    long int start_time, finish_time;
    struct timespec gettime_now;

    std::thread tds[THREADS];

    printf("Threads %d\t CPU PerThread %d\t", THREADS, PERTHREAD);
    printf("N cpus %d\n", PERTHREAD * THREADS);
    printf("Cycles %ld\n", CYCLES);
    clock_gettime(CLOCK_REALTIME, &gettime_now);
    start_time = gettime_now.tv_nsec + 1000000000 * gettime_now.tv_sec;
    
    for (int i=0; i< THREADS; i++) {
        tds[i] = std::thread(cpu_in_thread, i);
    }
    
    for (int i=0; i< THREADS; i++) {
        tds[i].join();
    }

    
    clock_gettime(CLOCK_REALTIME, &gettime_now);
    finish_time = gettime_now.tv_nsec + 1000000000 * gettime_now.tv_sec;
    printf("Running took %fms.\n", (float)(finish_time - start_time) / 1000000.0);
    
    delete[] data;
    return 0;
}
