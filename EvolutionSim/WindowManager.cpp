#include "WindowManager.h"

WindowManager::WindowManager()
{
    window = new sf::RenderWindow();
    window->create(sf::VideoMode(800, 600), "Evolution Sim", sf::Style::Close);
    
}

WindowManager::~WindowManager()
{
    window->close();
    delete window;
}
