#include <devices/gclock.hpp>

#include <iostream>

namespace cpu {

Generic_Clock::Generic_Clock() : 
    ticks(0), msg(0), trigger(false), acum(0), max_acum(0), divider(0) 
{
    clock.restart();
}

Generic_Clock::~Generic_Clock() 
{ 
}

unsigned Generic_Clock::handleInterrupt() 
{
    if (this->cpu == NULL)
        return 0;

    switch (cpu->getA() ) {
    case 0:
        divider = cpu->getB();
        if (cpu->getB() > 0) {
            this->max_acum =  sf::seconds(60.0f / divider).asMicroseconds();
            clock.restart();
        } 
        ticks = 0;
        break;

    case 1:
        cpu->setC(ticks);
        break;

    case 2:
        this->msg = cpu->getB();
        break;

    default:
        ;
    }

    return 0;
}


void Generic_Clock::tick() 
{
    if (divider >0) {

        acum += clock.getElapsedTime().asMicroseconds();
        clock.restart();

        if (acum > max_acum ) {
            //std::cerr << "\t\tMax: " << this->max_acum;
            //std::cerr << "  preacum "<< acum << std::endl;
            acum -= max_acum;
            //std::cerr << "  postacum "<< acum << std::endl;
            ticks++;
            trigger = msg > 0;
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
