#include "Engine.h"

#include <cassert>

Engine::Engine()
{
    auto font_loaded = global_font.loadFromFile("arial.ttf");
    assert(font_loaded);
    window_manager_ = new WindowManager();
    time_until_plant_spawn_ = EngineSettings::plant_spawn_interval;
    clock_.restart();
    things_.reserve(EngineSettings::initial_plant_count +
        EngineSettings::initial_creature_count);
    
    for(size_t i = 0; i < EngineSettings::initial_plant_count; i++)
        things_.push_back(new Plant());

    for(size_t i = 0; i < EngineSettings::initial_creature_count; i++)
        things_.push_back(new Creature());
}

bool Engine::tick()
{
    const auto dt = std::min(1.0f, clock_.restart().asSeconds() * time_speed_modifier);
    process_events();

    if(Creature::creatures_count == 0)
        for(size_t i = 0; i < EngineSettings::initial_creature_count; i++)
            things_.push_back(new Creature());

    time_until_plant_spawn_ -= dt;

    if(time_until_plant_spawn_ <= 0)
    {
        time_until_plant_spawn_ += EngineSettings::plant_spawn_interval;
        things_.push_back(new Plant());
    }
    
    for(size_t i = 0; i < things_.size(); i++)
        things_[i]->tick(dt, things_);

    for(int i = static_cast<int>(things_.size()) - 1; i >= 0; i--)
        if(!things_[i]->alive)
            delete remove_at_swap(things_, i);

    window_manager_->draw(things_);
    
    return window_manager_->is_window_open();
}

void Engine::process_events()
{
    sf::Event event;
    while (window_manager_->window_->pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
            window_manager_->window_->close();
    }
}
