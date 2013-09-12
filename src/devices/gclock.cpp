#include <devices/gclock.hpp>

#include <iostream>

namespace cpu {

Generic_Clock::Generic_Clock() : 
    ticks(0), msg(0), trigger(0), acum(0), max_acum(0), divider(0) 
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
            this->max_acum =  60000 / divider;
            //this->max_acum =  sf::seconds(60.0f / divider).asMicroseconds();
            clock.restart();
        } 
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

    update(); //new
    /*if (divider >0) {

        acum += clock.getElapsedTime().asMicroseconds(); //not accurate 
        //We lose data on windows because clock.getElapsedTime() = 0 
        //refresh time is to speed ! so we cannot accumulate
        clock.restart();
       if (acum > max_acum ) {
            //std::cerr << "\t\tMax: " << this->max_acum;
            //std::cerr << "  preacum "<< acum << std::endl;
            acum -= max_acum;
            //std::cerr << "  postacum "<< acum << std::endl;
            ticks++;
            trigger = msg > 0 ? 1 : 0;
        }
        
    }*/
}

//instead of calling this in ticks we can call this in main loop gain more than
// 1600 call by loop !
void Generic_Clock::update()
{
    if (divider >0) {
        //why not ? (it's slow operation)
        acum = clock.getElapsedTime().asMilliseconds()%max_acum;
        
        //hope we have
        int64_t now = clock.getElapsedTime().asMilliseconds()/max_acum;
        if (now > ticks)
        {
            trigger = msg > 0 ? now - ticks : 0; //no ticks lost !
            ticks = clock.getElapsedTime().asMilliseconds()/max_acum; 
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

        trigger--;
        msg = this->msg;
        return true;
    }
    return false;
}

} // END OF NAMESPACE cpu
