#pragma once
#include "Common.h"
#include "WindowManager.h"
#include "Creature.h"

class Engine
{
    
public:
    Engine();
    bool tick();
    void process_events();

private:
    std::vector<Thing*> things_;
    WindowManager* window_manager_;
    float time_until_plant_spawn_;
    sf::Clock clock_;
};

namespace EngineSettings
{
    static constexpr float plant_spawn_interval = 5.f;
    static constexpr size_t initial_plant_count = 50;
    
    static constexpr size_t initial_creature_count = DEBUG_VALUE_SWITCH(100, 1000);
}