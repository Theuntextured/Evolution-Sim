#pragma once
#include "Common.h"
#include "NeuralNetwork.h"

class NeuralNetwork;

typedef sf::Uint16 Gene;
#define DRAW_RESOLUTION 32

#define DRAW_DEBUG_DATA 1 && _DEBUG

class Plant;

class Thing
{
public:
    Thing();
    virtual ~Thing();
    Thing(const Thing&) = default;
    Thing(Thing&&) = default;

    Plant* is_overlapping_plant(const std::vector<Thing*>& things_in_world) const;
    bool is_overlapping_other(const Thing* other) const;

    
    Thing& operator=(const Thing&) = default;
    Thing& operator=(Thing&&) = default;
    Gene gene = 1;    
    float size = 5.0f;
    sf::Vector2f position;
    bool alive = true;
    
protected:
    sf::CircleShape* shape_;
    sf::Color color_ = sf::Color::Green;
    
public:
    virtual void tick(const float dt, std::vector<Thing*>& things_in_world) {}
    virtual void draw(sf::RenderWindow& window) {}
};

constexpr float plant_growth_rate = DEBUG_VALUE_SWITCH(10.0f, 0.1f);
constexpr float plant_energy_per_unit_size = 3.0f;
class Plant : public Thing
{
public:
    Plant();
    ~Plant() override;
    void tick(const float dt, std::vector<Thing*>& things_in_world) override;
    void draw(sf::RenderWindow& window) override;

    static size_t plant_count;
};

class Creature : public Thing
{
public:
    Creature();
    Creature(const Creature &other);
    ~Creature() override;
    bool can_eat(const Thing* other) const;
    bool can_be_eaten(const Thing* other) const;
    
    void tick(const float dt, std::vector<Thing*>& things_in_world) override;
    void draw(sf::RenderWindow& window) override;

    template <typename T>
    static T mutate_property(const T& property);
    static Gene mutate_gene(const Gene g);
    static sf::Color mutate_color(const sf::Color& color);
    
    float speed = 10.0f;
    float vision_angle = 30.0f;
    float vision_distance = 10.0f;
    float strength = 1.0f;
    float energy_storage = 20.0f;
    Gene diet = 1;
    float average_offspring_count = 3.f;
    float max_offspring_offset = 1.f;
    float age_to_reproduce = 10.0f;
    NeuralNetwork* neural_network;

    static size_t creatures_count;
private:
    void calculate_energy_consumptions();
    void get_neural_network_parameters(float* out, const std::vector<Thing*>& things_in_world);
    void get_neural_network_outputs(float* out, const std::vector<Thing*>& things_in_world);
    void reproduce(std::vector<Thing*>& things_in_world);
    void attempt_attack();
    bool can_see_thing(const Thing* thing) const;
    void setup_shape();
    
    
    float energy_;
    sf::Vector2f orientation_;
    float idle_energy_consumption_ = 0.1f;
    float movement_energy_consumption_ = 1.0f;
    float energy_per_offspring_ = 1.0f;
    Creature* attackable_creature_ = nullptr;
    Plant* nearby_plant_ = nullptr;
    sf::Clock reproduction_clock_;
    sf::RectangleShape direction_shape_;
    
#if DRAW_DEBUG_DATA
    sf::Text debug_text_;    
#endif
};
