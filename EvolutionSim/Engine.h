#pragma once
#include "Common.h"
#include "WindowManager.h"

class Engine
{
private:
    WindowManager* window_manager;
    
public:
    Engine();
    bool tick();
    void process_events();
};
