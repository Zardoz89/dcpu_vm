#ifndef ABSTRACTWINDOW_HPP_
#define ABSTRACTWINDOW_HPP_ 1

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

namespace windows {

/**
 * @brief Abstract class that are the base of windows used by the VM
 */
class AbstractWindow: public sf::RenderWindow {
public:
    AbstractWindow() : focus(false) {};
    virtual ~AbstractWindow() {};

    /**
     * Displays the Keyboard Window
     */
    virtual void display()
    {
        this->RenderWindow::display();
    }

    /**
     * Manages his own events, and send keyboard events to the keyboard device
     */
    virtual void handleEvents() = 0;

    /**
     * Return if have focus
     */
    bool haveFocus() const {return focus;}

protected:
    bool focus;

};

} /* namespace windows */

#endif // ABSTRACTWINDOW_HPP_
