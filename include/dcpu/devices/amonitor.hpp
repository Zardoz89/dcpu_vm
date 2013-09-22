#pragma once
#ifndef _MONITOR_HPP_
#define _MONITOR_HPP_ 1
#include <dcpu/dcpu.hpp>


#include <cstdint>
#include <cassert>

namespace cpu {

class AbstractMonitor : public cpu::IHardware{
public:
    AbstractMonitor() : need_render(false), pixels(NULL), _width(0), _height(0),
                        powered(false), splash(false), splashtime(1000)
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
     * Returns the Border color as DWORD
     * @Return the color format is 0xRRGGBBAA
     */
    virtual uint32_t getBorder() const = 0;

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
    
    inline bool isPowered() const
    {
        return powered;
    }

    inline bool isSplash() const
    {
        return splash;
    }

protected:
    /**
     * Update the pixels array
     */
    virtual void updateScreen() = 0;

    bool need_render;       /// Do we need a render (Improve the speed)

    uint8_t* pixels;        /// Pixels matrix in RGBA format
    unsigned _width;        /// Screen width in pixels
    unsigned _height;       /// Screen height in pixels

    bool powered;           /// Is ON ?
    bool splash;            /// In splash mode ?
    uint32_t splashtime;    /// Time that shows the splash image in CPU cycles

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
        assert(x < width());
        assert(y < height());

        auto pos = 4*(x + y*width());
        pixels[pos   ] = r; // R
        pixels[pos +1] = g; // G
        pixels[pos +2] = b; // B
        pixels[pos +3] = a; // A
    }

    /**
     * Sets a pixel value
     * @param x X coord must be < width
     * @param y Y coord must be < height
     * @param The color in a DWORD format : 0xRRGGBBAA 
     */
    inline void setPixel(unsigned x, unsigned y, uint32_t color)
    {
        assert(pixels != NULL);
        assert(x < width());
        assert(y < height());

        auto pos = 4*(x + y*width());
        pixels[pos   ] = color >> 24;           // R
        pixels[pos +1] = (color >> 16) & 0xFF; // G
        pixels[pos +2] = (color >> 8) & 0xFF; // B
        pixels[pos +3] = color & 0xFF;     // A
    }

};
} // END OF NAMESPACE cpu

#endif // Endif _MONITOR_HPP_
