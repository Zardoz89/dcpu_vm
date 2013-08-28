#include "gclock.hpp"

#include <iostream>

namespace cpu {

    Generic_Clock::Generic_Clock() : 
        cpu_ticks(0), max_ticks(0), ticks(0), msg(0) 
    { 
    }

    Generic_Clock::~Generic_Clock() 
    { 
    }

    void Generic_Clock::handleInterrupt() 
    {
        if (this->cpu == NULL)
            return;

        switch (cpu->GetA() ) {
        case 0:
            if (cpu->GetB() > 0) {
                max_ticks = (cpu->GetB() * cpu->cpu_clock) / 60;
            } else {
                max_ticks = 0;
            }
            cpu_ticks = ticks = 0;
            break;

        case 1:
            cpu->SetC(ticks);
            break;

        case 2:
            this->msg = cpu->GetB();
            break;

        default:
            ;
        }
    }
   

    void Generic_Clock::tick() 
    {
        // We use CPU clock ticks to measure time relative to DCPU core
        if (max_ticks >0 && msg > 0) {
            cpu_ticks++;
        }
    }

    bool Generic_Clock::checkInterrupt (uint16_t &msg)
    {
        if (cpu_ticks >= max_ticks && this->msg > 0) {
            cpu_ticks -= max_ticks;
            ticks++;
            msg = this->msg;
            return true;
        }
        return false;
    }

} // END OF NAMESPACE cpu
