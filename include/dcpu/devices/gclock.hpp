#pragma once
#ifndef _G_CLOCK_HPP_
#define _G_CLOCK_HPP_ 1

#include <dcpu/dcpu.hpp>

#include <cstdint>
#include <chrono>
// TODO Is using SFML clock for timming. Remplace for something not SFML
// dependent

namespace cpu {

class Generic_Clock : public cpu::IHardware {

public:
    Generic_Clock();
    ~Generic_Clock();

    uint32_t getId() const {
        return 0x12d0b402;
    }
    uint16_t getRevision() const {
        return 0x0001;
    }
    uint32_t getManufacturer() const {
        return 0x90099009;
    }
    
    bool checkInterrupt (uint16_t &msg);
    unsigned handleInterrupt();
    void tick();

    inline void update(); //test for windows
    
private:

    int64_t clock_ticks;        /// Clock ticks
    uint16_t ticks;             /// Ticks readed to DCPU (read the clock specs!)
    uint16_t msg;               /// Message to send when a interrupt hapens
    int trigger;                /// Trigered
    
    //why std can't make little name ?
    std::chrono::time_point<std::chrono::system_clock> 
    start, end;                 /// Measures the time
    int64_t acum;               /// Accumulates delta time
    int64_t max_acum;           /// If accu > max_accu -> does a Tick
    uint_fast16_t divider;      /// Frac of 60 seconds
};

} // END OF NAMESPACE cpu

#endif // _G_CLOCK_HPP_
