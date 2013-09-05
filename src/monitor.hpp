#pragma once
#ifndef _MONITOR_HPP_
#define _MONITOR_HPP_ 1
#include "dcpu.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Image.hpp>

#include <cstdint>

namespace cpu {

class AbstractMonitor : public cpu::IHardware{
public:
        AbstractMonitor() : need_render(false) {} 
    /**
     * Return Actual screen resolution Width without border
     */
    inline unsigned int width() const
    {
        return screen.getSize().x;
    }
    
    /**
     * Return Actual screen resolution Height without border
     */
    inline unsigned int height() const
    {
        return screen.getSize().y;
    }

    /**
     * Return Physical monitor Width without border
     */
    virtual unsigned int phyWidth() const = 0;
    
    /**
     * Return Physical monitor Height without border
     */
    virtual unsigned int phyHeight() const = 0;

    /**
     * Return Physical monitor outside border
     */
    virtual unsigned int borderSize() const {return 10;}


    /**
     * Returns the Border color
     */
    virtual sf::Color getBorder() const = 0;

    /**
      * @Call before render 1 frame to each frames
      */
    inline void prepareRender()
    {
        need_render = true;
    }
    
    /**
      * @brief ptr value will NEVER change
      */
    inline const sf::Image* getScreen() const
    {
        return &screen;
    }
    
    
protected:
    /**
     * Generates a sf::Image with the actual screen state
     */
    virtual void updateScreen() = 0;

    bool need_render; ///Do we need a render (Improve the speed)
    sf::Image screen; 

};
} // END OF NAMESPACE cpu

#endif // Endif _MONITOR_HPP_
