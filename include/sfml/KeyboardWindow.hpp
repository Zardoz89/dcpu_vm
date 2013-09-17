#ifndef KEYBOARDWINDOW_HPP_
#define KEYBOARDWINDOW_HPP_

#include <devices/gkeyboard.hpp>

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include <memory>

namespace windows {

typedef std::shared_ptr<cpu::keyboard::GKeyboard> sptr_GKeyboard;

/**
 * @brief Class does the visual representation of a keyboard device
 */
class KeyboardWindow: public sf::RenderWindow {
public:
    KeyboardWindow(unsigned devId, sptr_GKeyboard keyboard);
    virtual ~KeyboardWindow();

    /**
     * Displays the Keyboard Window
     */
    void display();

    /**
     * Manages his own events, and send keyboard events to the keyboard device
     */
    void handleEvents();

private:
    sptr_GKeyboard keyboard;    /// Genetic Keyboard that represents
    sf::Texture keyb_tx;        /// Texture that fills the window
    sf::Sprite keyb_sprite;     /// Sprite that fills the window

    bool keyb_image_loaded;
    bool focus;                  /// Have the focus ?
};

} // END OF NAMESPACE windows

#endif /* KEYBOARDWINDOW_HPP_ */
