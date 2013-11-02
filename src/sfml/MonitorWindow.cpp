#include <sfml/MonitorWindow.hpp>
#include <config.hpp>

#include <cstdio>

namespace windows {

MonitorWindow::MonitorWindow(sptr_AbstractMonitor monitor,
                             const std::string title, unsigned framerate) :
                               monitor(monitor)
{

    border_add = monitor->borderSize()*2;
    char tmp[48];
    snprintf(tmp, 48, "%s DevId=%u", title.c_str(),
                   monitor->getDevIndex());

    this->create(sf::VideoMode(monitor->phyWidth()+border_add,
            monitor->phyHeight() +border_add),
            tmp);
    this->setFramerateLimit(framerate);

    aspect_ratio = (double)(monitor->phyWidth() + border_add) /
                   (monitor->phyHeight() + border_add);
    old_size = this->getSize();

    texture.create(monitor->width(), monitor->height());
    old_width = monitor->width();
    old_height = monitor->height();

    splash.create(monitor->width(), monitor->height());

    powering_fx = sf::Color(0x01, 0x01, 0x01, 0xFF);
}

MonitorWindow::~MonitorWindow()
{
    // TODO Auto-generated destructor stub
}

void MonitorWindow::display()
{
    this->setActive(true);

    if (monitor->isPowered() && monitor->isSplash()) {
        // Show splash if there is any splash image

        splash_sprite.setScale(  //Warning setScale and scale are different !!
          (float)(getSize().x) / (float)(splash.getSize().x),
          (float)(getSize().y) / (float)(splash.getSize().y) );

        if (powering_fx.r < 0xFF) {
            powering_fx.r += 0x02;
            powering_fx.b += 0x02;
            powering_fx.g += 0x02;
            splash_sprite.setColor(powering_fx);
        }

        clear(sf::Color::Black);
        draw(splash_sprite);

    } else if (monitor->isPowered()) {

        // Working resizing code
        border_add = monitor->borderSize();

        if (old_width != monitor->width() || old_height != monitor->height()) {
            texture.create(monitor->width(), monitor->height());
            old_width = monitor->width();
            old_height = monitor->height();
        }

        texture.update(monitor->getPixels()); //Slow function
        sprite.setTexture(texture);

        sprite.setScale(  //Warning setScale and scale are different !!
          (float)(getSize().x - border_add*2) / (float)(monitor->width()),
          (float)(getSize().y - border_add*2) / (float)(monitor->height()) );
        sprite.setPosition(sf::Vector2f(border_add,border_add));

        if (powering_fx.r < 0xFF) {
            powering_fx.r += 0x02;
            powering_fx.b += 0x02;
            powering_fx.g += 0x02;
            sprite.setColor(powering_fx);
        }
        uint32_t c = monitor->getBorder();
        // Draws border and screen state
        clear(sf::Color(c >> 24,(c >> 16) & 0xFF,(c >> 8) & 0xFF ,0xFF)); 
        draw(sprite);

    } else { // No power, no image
        clear(sf::Color::Black);
    }

    this->AbstractWindow::display();
    this->setActive(false);
}

void MonitorWindow::handleEvents()
{
    if (! this->isOpen())
        return;

    // Handle events
    sf::Event event;

    bool resized = false;
    sf::Vector2u newsize;

    while (pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            close();
        } else if (event.type == sf::Event::GainedFocus) {
            focus = true;
        } else if (event.type == sf::Event::LostFocus) {
            focus = false;

        } else if (event.type == sf::Event::Resized) {
            double diffWidth = std::abs((double) event.size.width - old_size.x);
            double diffHeight = std::abs((double) event.size.height - old_size.y);

            if (diffWidth > diffHeight) { // Enforces aspect ratio
                newsize.x = event.size.width;
                newsize.y = (unsigned)(event.size.width / aspect_ratio);
            } else {
                newsize.x = (unsigned)(event.size.height * aspect_ratio);
                newsize.y = event.size.height;
            }


            resized = true;
        }
    }

    if (resized) { // Avoid SFML behavior that floods of resize events
        setSize(newsize);
        //Re-wrap opengl camera
        float r_width = newsize.x;
        float r_height = newsize.y;
        sf::FloatRect r(0,0,r_width,r_height);
        setView(sf::View(r));
        old_size = newsize;
    }
}


void MonitorWindow::setSplashImage (const std::string filename)
{
    if (filename.size() == 0) {
        splash.create(monitor->width(), monitor->height());
        return;
    }

    splash.loadFromFile(filename);
    splash_sprite.setTexture(splash);
    splash_sprite.setPosition(0,0);

}

} /* namespace windows */
