#pragma once
#include "Common.h"
#include "Creature.h"

class WindowManager
{
    friend class Engine;
public:
    WindowManager();
    ~WindowManager();
    inline bool is_window_open() const{ return window_->isOpen(); }
    void draw(const std::vector<Thing*>& things_in_world) const;
protected:
    sf::RenderWindow* window_;
};
