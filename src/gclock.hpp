#pragma once
#ifndef _G_CLOCK_HPP_
#define _G_CLOCK_HPP_ 1

#include <cstdint>

#include "dcpu.hpp"

namespace cpu {

class Generic_Clock : public cpu::IHardware {

public:
    Generic_Clock();
    ~Generic_Clock();

    static const uint32_t id            = 0x12d0b402;
    static const uint16_t revision      = 0x1;
    static const uint32_t manufacturer  = 0x0;
    
    uint32_t getId() {
        return id;
    }
    uint16_t getRevision() {
        return revision;
    }
    uint32_t getManufacturer() {
        return manufacturer;
    }
    
    bool checkInterrupt (uint16_t &msg);
    void handleInterrupt();
    void tick();

private:

    uint_fast32_t cpu_ticks; /// CPU Tick counter
    uint_fast32_t max_ticks; /// How many ticks need to count to launch a interrupt
    uint_fast32_t ticks;     /// Clock ticks
    uint16_t msg;       /// Mesage to send when a interrupt hapens
};

} // END OF NAMESPACE cpu

#endif // _G_CLOCK_HPP_
