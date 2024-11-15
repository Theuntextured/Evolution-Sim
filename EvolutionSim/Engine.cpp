#include "Engine.h"

Engine::Engine()
{
    window_manager = new WindowManager();
}

bool Engine::tick()
{
    process_events();
    return window_manager->is_window_open();
}

void Engine::process_events()
{
    sf::Event event;
    while (window_manager->window->pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
            window_manager->window->close();
    }

    
}
