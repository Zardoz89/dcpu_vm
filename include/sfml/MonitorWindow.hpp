#ifndef MONITORWINDOWS_HPP_
#define MONITORWINDOWS_HPP_

#include <sfml/AbstractWindow.hpp>
#include <devices/monitor.hpp>

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

private:
    double aspect_ratio;
    sf::Vector2u old_size;

};

} /* namespace windows */
#endif /* MONITORWINDOWS_HPP_ */
