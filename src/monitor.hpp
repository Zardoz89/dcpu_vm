
#ifndef _MONITOR_HPP_
#define _MONITOR_HPP_ 1
#include "dcpu.hpp"
#include <SFML/Graphics.hpp>
#include <cstdint>

namespace cpu {

class AbstractMonitor : public cpu::IHardware{
	public:
		AbstractMonitor() : need_render(false) {} 
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
		 * Generates a sf::Texture with the actual screen state
		 */
		virtual const sf::Texture& getScreen() const = 0;

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
	
	protected:
		bool need_render; ///Do we need a render (Improve the speed)
};

} // END OF NAMESPACE cpu

#endif // Endif _MONITOR_HPP_
