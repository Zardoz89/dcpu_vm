#pragma once
#ifndef _MONITOR_HPP_
#define _MONITOR_HPP_ 1

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Image.hpp>

#include <cstdint>

namespace cpu {

class AbstractMonitor {
public:
    /**
     * Return Actual screen resolution Width without border
     */
    virtual unsigned int width() const = 0;
    
    /**
     * Return Actual screen resolution Height without border
     */
    virtual unsigned int height() const = 0;

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
     * Generates a sf::Image with the actual screen state
     */
    virtual sf::Image* updateScreen() const = 0;

    /**
     * Returns the Border color
     */
    virtual sf::Color getBorder() const = 0;

};

} // END OF NAMESPACE cpu

#endif // Endif _MONITOR_HPP_
