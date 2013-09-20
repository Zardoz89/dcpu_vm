#ifndef MONITORWINDOWS_HPP_
#define MONITORWINDOWS_HPP_

#include <sfml/AbstractWindow.hpp>
#include <dcpu/devices/amonitor.hpp>

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include <memory>

namespace windows {

typedef std::shared_ptr<cpu::AbstractMonitor> sptr_AbstractMonitor;

/**
 * @brief Window that shows a virtual monitor on screen
 */
class MonitorWindow: public AbstractWindow {
public:
    MonitorWindow(sptr_AbstractMonitor monitor, const std::string title,
                    unsigned framerate = 50);
    virtual ~MonitorWindow();

    /**
     * Displays the Keyboard Window
     */
    void display();

    /**
     * Manages his own events, and send keyboard events to the keyboard device
     */
    void handleEvents();

    /**
     * @brief Sets the asset files with the splash image
     * If the filename is "" or isn't called, then uses a black screen for
     * splash
     * @param filename Asset file
     */
    void setSplashImage (const std::string filename);


private:
    sptr_AbstractMonitor monitor;
    double aspect_ratio;
    sf::Vector2u old_size;
    unsigned int old_width;
    unsigned int old_height;

    float border_add;

    sf::Texture splash;         /// Splash image
    sf::Sprite splash_sprite;   /// Splash sprite
    sf::Color powering_fx;      /// Sprite color to imitate gradual power on

    sf::Texture texture;        /// Texture of the screen
    sf::Sprite sprite;          /// Sprite of the screen

};

} /* namespace windows */
#endif /* MONITORWINDOWS_HPP_ */
