#include "gclock.hpp"

//#include <iostream>

namespace cpu {

    
    Generic_Clock::Generic_Clock() : 
        cpu_ticks(0), max_ticks(0), ticks(0), msg(0), trigger(false), 
        acum(0), max_acum(0), divider(0) 
    {
        clock.restart();
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
            divider = cpu->GetB();
            if (cpu->GetB() > 0) {
                this->max_acum =  sf::seconds(60.0 / divider).asMicroseconds();
                clock.restart();
                //max_ticks = (cpu->cpu_clock * 60) / cpu->GetB();
                // std::cerr << "Max :" << max_ticks << "B= " << cpu->GetB();
                // std::cerr << std::endl; 
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
        //if (max_ticks >0 && msg > 0) {
        if (divider >0) {
            /*cpu_ticks++;
            if (cpu_ticks >= max_ticks) {
                cpu_ticks -= max_ticks;
            }*/

            acum += clock.getElapsedTime().asMicroseconds();
            clock.restart();

            if (acum >= max_acum ) {
                acum -= max_acum;
                ticks++;
                trigger = msg > 0;
                //std::cerr << "TICK! " << acum << std::endl;
            }
        }
    }

    bool Generic_Clock::checkInterrupt (uint16_t &msg)
    {
        if (trigger && this->msg > 0) {
            //b = std::chrono::high_resolution_clock::now();
            //auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(b -e);
            //std::cerr << delta.count() << " ms" << std::endl;
            //e = std::chrono::high_resolution_clock::now();

            trigger = false;
            msg = this->msg;
            return true;
        }
        return false;
    }

} // END OF NAMESPACE cpu
