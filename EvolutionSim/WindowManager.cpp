#include "WindowManager.h"

WindowManager::WindowManager()
{
    window_ = new sf::RenderWindow();
    window_->create(sf::VideoMode(
        static_cast<unsigned int>(world_extent.x), static_cast<unsigned int>(world_extent.y)),
        "Evolution Sim",
        sf::Style::Close);
}

WindowManager::~WindowManager()
{
    window_->close();
    delete window_;
}

void WindowManager::draw(const std::vector<Thing*>& things_in_world) const
{
    window_->clear();
    for (const auto thing : things_in_world)
        thing->draw(*window_);
    static sf::Clock clock;
    sf::Text stat_text;
    stat_text.setCharacterSize(18);
    sf::String text_to_display = std::to_string(static_cast<unsigned int>(1.0f / clock.restart().asSeconds())) + " fps\n";
    text_to_display += std::to_string(Plant::plant_count) + " plants\n";
    text_to_display += std::to_string(Creature::creatures_count) + " creatures\n";
    stat_text.setString(text_to_display);
    stat_text.setFont(global_font);
    window_->draw(stat_text);

    window_->display();
}
