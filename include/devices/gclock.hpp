#pragma once
#ifndef _G_CLOCK_HPP_
#define _G_CLOCK_HPP_ 1

#include <dcpu.hpp>

#include <cstdint>
#include <SFML/System.hpp>

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

private:

    uint_fast32_t ticks;        /// Clock ticks
    uint16_t msg;               /// Mesage to send when a interrupt hapens
    bool trigger;               /// Trigered
    
    sf::Clock clock;            /// Measures the time
    int64_t acum;               /// Acumulates delta time
    int64_t max_acum;           /// If accu > max_accu -> does a Tick
    uint_fast16_t divider;      /// Frac of 60 seconds
};

} // END OF NAMESPACE cpu

#endif // _G_CLOCK_HPP_
