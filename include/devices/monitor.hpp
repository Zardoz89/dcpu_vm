#pragma once
#ifndef _MONITOR_HPP_
#define _MONITOR_HPP_ 1
#include <dcpu.hpp>

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Image.hpp>

#include <cstdint>
#include <cassert>

namespace cpu {

class AbstractMonitor : public cpu::IHardware{
public:
    AbstractMonitor() : need_render(false), pixels(NULL), _width(0), _height(0)
    { }

    /**
     * Return Actual screen resolution Width without border
     */
    inline unsigned int width() const
    {
        return _width;
    }
    
    /**
     * Return Actual screen resolution Height without border
     */
    inline unsigned int height() const
    {
        return _height;
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
     * TODO do a typedef to hide sf:Color
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
     * @brief ptr with the pixels in RGBA format
     */
    inline const uint8_t* getPixels() const
    {
        return pixels;
    }
    

protected:
    /**
     * Generates a sf::Image with the actual screen state
     */
    virtual void updateScreen() = 0;

    bool need_render; ///Do we need a render (Improve the speed)

    uint8_t* pixels;    /// Pixels matrix in RGBA format
    unsigned _width;     /// Screen width in pixels
    unsigned _height;    /// Screen height in pixels

    /**
     * Sets a pixel value
     * @param x X coord must be < width
     * @param y Y coord must be < height
     * @param r Red channel
     * @param g Green channel
     * @param b Blue channel
     * @param a Alpha channel
     */
    inline void setPixel(unsigned x, unsigned y,
            uint8_t r, uint8_t g, uint8_t b, uint8_t a =255)
    {
        assert(pixels != NULL);
        //assert(x < width());
        //assert(y < height());

        pixels[4*(x*height() + y)  ] = r; // R
        pixels[4*(x*height() + y)+1] = g; // G
        pixels[4*(x*height() + y)+2] = b; // B
        pixels[4*(x*height() + y)+3] = a; // A
    }

    /**
     * Sets a pixel value
     * @param x X coord must be < width
     * @param y Y coord must be < height
     * @param color RGBA color
     */
    inline void setPixel(unsigned x, unsigned y, sf::Color color)
    {
        assert(pixels != NULL);
        assert(x < width());
        assert(y < height());

        pixels[4*(x + y*width())  ] = color.r; // R
        pixels[4*(x + y*width())+1] = color.g; // G
        pixels[4*(x + y*width())+2] = color.b; // B
        pixels[4*(x + y*width())+3] = color.a; // A
    }

};
} // END OF NAMESPACE cpu

#endif // Endif _MONITOR_HPP_
