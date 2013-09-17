#include <sfml/MonitorWindow.hpp>
#include <config.hpp>

#include <cstdio>

namespace windows {

MonitorWindow::MonitorWindow(sptr_AbstractMonitor monitor,
                                 const std::string title,
                                 unsigned framerate) : monitor(monitor)
{

    border_add = monitor->borderSize()*2;
    char tmp[48];
    std::snprintf(tmp, 48, "%s DevId=%zu", title.c_str(),
                   monitor->getDevIndex());

    this->create(sf::VideoMode(monitor->phyWidth()+border_add,
            monitor->phyHeight() +border_add),
            tmp);
    this->setFramerateLimit(framerate);

    aspect_ratio = (double)(monitor->phyWidth() + border_add) /
                   (monitor->phyHeight() + border_add);
    old_size = this->getSize();

    screen = monitor->getScreen();
}

MonitorWindow::~MonitorWindow()
{
    // TODO Auto-generated destructor stub
}

void MonitorWindow::display()
{
    this->setActive(true);

    //Working resizing code
    border_add = monitor->borderSize();

    texture.loadFromImage(*screen); //Slow function
    sprite.setTexture(texture);
    sprite.setScale(  //Warning setScale and scale are different !!
      (float)(getSize().x-border_add*2)/(float)(monitor->width()),
      (float)(getSize().y-border_add*2)/(float)(monitor->height()));
    sprite.setPosition(sf::Vector2f(border_add,border_add));

    clear(monitor->getBorder()); // Draws border and screen state
    draw(sprite);

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
            double diffWidth = std::abs(event.size.width - old_size.x);
            double diffHeight = std::abs(event.size.height - old_size.y);

            if (diffWidth > diffHeight) { // Enforces aspect ratio
                newsize.x = event.size.width;
                newsize.y = (unsigned)(event.size.width * aspect_ratio);
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

} /* namespace windows */
