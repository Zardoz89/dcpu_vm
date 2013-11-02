#include <dcpu/devices/gclock.hpp>

#include <iostream>

namespace cpu {

Generic_Clock::Generic_Clock() : 
    clock_ticks(0), ticks(0), msg(0), trigger(0), acum(0), max_acum(0),
    divider(0)
{
    start = std::chrono::system_clock::now();
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
            this->max_acum =  60000 / divider;
            //this->max_acum =  sf::seconds(60.0f / divider).asMicroseconds();
            start = std::chrono::system_clock::now();
        } 
        clock_ticks=0;
        ticks=0;
        acum=0;
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

    update();
}

void Generic_Clock::update()
{
    if (divider >0) {
        //why not ? (it's slow operation)
        end = std::chrono::system_clock::now();
        int64_t delta=std::chrono::duration_cast<std::chrono::milliseconds>
                             (end-start).count();
        acum = (delta)%max_acum;
        
        int64_t now_tick = delta/max_acum;
        if (now_tick > clock_ticks)
        {
            trigger = msg > 0 ? (int) (now_tick - clock_ticks) : 0; //no ticks lost !
            clock_ticks = now_tick;
            ticks++; //Ticks the number of events since the last call command 0
        }
        
        
    }
}

bool Generic_Clock::checkInterrupt (uint16_t &msg)
{
    if (trigger && this->msg > 0) {
        trigger--;
        msg = this->msg;
        return true;
    }
    return false;
}

} // END OF NAMESPACE cpu
