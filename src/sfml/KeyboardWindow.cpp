/*
 * KeyboardWindow.cpp
 *
 *  Created on: 17/09/2013
 *      Author: luis
 */

#include <sfml/KeyboardWindow.hpp>
#include <config.hpp>

#include <cstdio>

namespace windows {

KeyboardWindow::KeyboardWindow(sptr_GKeyboard keyboard) : keyboard(keyboard)

{
    char tmp[32];
    snprintf(tmp, 32, "Keyboard DevId=%zu", keyboard->getDevIndex());

    this->create(sf::VideoMode(484, 196), tmp, sf::Style::Titlebar |
                  sf::Style::Close);

    this->setFramerateLimit(25);
    //this->setVerticalSyncEnabled(true);

    this->setKeyRepeatEnabled(false);
    this->setActive(false);

    // TODO not be hard-code here, the asset file that uses
    keyb_image_loaded = keyb_tx.loadFromFile("assets/keyb_img.png");
    if (keyb_image_loaded)
        keyb_sprite.setTexture(keyb_tx);
    else
        LOG_WARN << "assets/keyb_img.png not found !";
}

KeyboardWindow::~KeyboardWindow()
{
    // TODO Auto-generated destructor stub
}

void KeyboardWindow::display()
{
    this->setActive(true);
    this->clear();

    if (keyb_image_loaded) {
        this->draw(keyb_sprite);
    }

    this->AbstractWindow::display();
    this->setActive(false);
}

void KeyboardWindow::handleEvents()
{
    if (! this->isOpen())
        return;

    using namespace cpu::keyboard;

    // Process events
    sf::Event event;
    while (this->pollEvent(event)) {
        // This window capture key events to the VM window if have focus
        if (event.type == sf::Event::Closed) {
            this->close();

        } else if (event.type == sf::Event::GainedFocus) {
            focus = true;

        } else if (event.type == sf::Event::LostFocus) {
            focus = false;

        } else if (focus && ( event.type == sf::Event::KeyPressed ||
                    event.type == sf::Event::KeyReleased ||
                    event.type == sf::Event::TextEntered )) {
            // Process VM keyboard input
            bool pressed = (event.type == sf::Event::KeyPressed);
            unsigned char keycode=0;

            // Note this works because Unicode maps ASCII 7 bit in his
            // first 128 codes
            if (event.type == sf::Event::TextEntered &&
                   ((event.text.unicode >= '!' &&
                     event.text.unicode <= '/') ||
                    (event.text.unicode >= ':' &&
                         event.text.unicode <= '@') ||
                    (event.text.unicode >= '[' &&
                         event.text.unicode <= 0x60) ||
                    (event.text.unicode >= '{' &&
                         event.text.unicode <= 0x7F) )) {

                    keyboard->pushKeyEvent(true,
                            (unsigned char) event.text.unicode);
                    keyboard->pushKeyEvent(false,
                            (unsigned char) event.text.unicode);
            }

            // Ignore silenty any other TextEntered events
            // because SFML generate at same time KeyEvent and TextEntered
            if (event.type == sf::Event::TextEntered)
                continue;

            if (event.key.code>=sf::Keyboard::A &&
                event.key.code<=sf::Keyboard::Z) {
                if (event.key.shift)
                    keycode=event.key.code+'A';
                else
                    keycode=event.key.code+'a';
            } else if (event.key.code>=sf::Keyboard::Num0 &&
                    event.key.code<=sf::Keyboard::Num9) {

                keycode=event.key.code-sf::Keyboard::Num0+'0';
            } else {
                switch (event.key.code) {
                    case sf::Keyboard::Space:
                        keycode= ' ';
                        break;
                    case sf::Keyboard::BackSpace:
                        keycode= BACKSPACE;
                        break;
                    case sf::Keyboard::Return:
                        keycode= RETURN;
                        break;
                    case sf::Keyboard::Insert:
                        keycode= INSERT;
                        break;
                    case sf::Keyboard::Delete:
                        keycode= DELETE;
                        break;
                    case sf::Keyboard::Up:
                        keycode= ARROW_UP;
                        break;
                    case sf::Keyboard::Down:
                        keycode= ARROW_DOWN;
                        break;
                    case sf::Keyboard::Left:
                        keycode= ARROW_LEFT;
                        break;
                    case sf::Keyboard::Right:
                        keycode= ARROW_RIGHT;
                        break;
                    case sf::Keyboard::RShift:
                    case sf::Keyboard::LShift:
                        keycode= SHIFT;
                        break;
                    case sf::Keyboard::RControl:
                    case sf::Keyboard::LControl:
                        keycode= CONTROL;
                        break;
                    case sf::Keyboard::Escape:
                        keycode= ESC;
                        break;


                    default:
                        break;
                }
            }
            if (keycode)
                keyboard->pushKeyEvent(pressed,keycode);
        }
    }

}

} // END OF NAMESPACE windows
