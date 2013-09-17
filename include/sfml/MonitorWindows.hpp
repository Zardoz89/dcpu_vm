#ifndef MONITORWINDOWS_HPP_
#define MONITORWINDOWS_HPP_

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include <memory>

namespace windows {

/**
 * @brief Window that shows a virtual monitor on screen
 */
class MonitorWindows: public sf::RenderWindow {
public:
    MonitorWindows();
    virtual ~MonitorWindows();
};

} /* namespace windows */
#endif /* MONITORWINDOWS_HPP_ */
