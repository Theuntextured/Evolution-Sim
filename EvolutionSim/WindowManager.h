#pragma once
#include <SFML/Graphics/RenderWindow.hpp>

class WindowManager
{
    friend class Engine;
public:
    WindowManager();
    ~WindowManager();
    inline bool is_window_open() const{ return window->isOpen(); }
protected:
    sf::RenderWindow* window;
};
