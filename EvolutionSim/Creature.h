#pragma once
#include "Common.h"
#include "NeuralNetwork.h"

class NeuralNetwork;

using Gene = sf::Uint16;

class Thing
{
public:
    Thing();
    Gene gene = 1;    
    float size = 1.0f;
    sf::Vector2f position;
    sf::Color color = sf::Color::Green;
    bool alive = true;
    
    virtual void tick(const float dt, std::vector<Thing*>& things_in_world) {}
};

constexpr float plant_growth_rate = 0.1f;
constexpr float plant_energy_per_unit_size = 3.0f;
class Plant : public Thing
{
    Plant();
    void tick(const float dt, std::vector<Thing*>& things_in_world) override;
public:
};

class Creature : public Thing
{
public:
    Creature();
    Creature(const Creature &other);
    bool can_eat(const Thing* other) const;
    bool can_be_eaten(const Thing* other) const;
    void tick(const float dt, std::vector<Thing*>& things_in_world) override;

    template <typename T>
    static T mutate_property(const T& property);
    static Gene mutate_gene(const Gene g);
    
    float speed = 1.0f;
    float vision_angle = 30.0f;
    float vision_distance = 10.0f;
    float strength = 1.0f;
    float energy_storage = 20.0f;
    Gene diet = 1;
    float average_offspring_count = 3.f;
    float max_offspring_offset = 1.f;
    NeuralNetwork* neural_network;
private:
    void calculate_energy_consumptions();
    void get_neural_network_parameters(float* out, const std::vector<Thing*>& things_in_world);
    void get_neural_network_outputs(float* out, const std::vector<Thing*>& things_in_world);
    void reproduce(std::vector<Thing*>& things_in_world);
    void attempt_attack();
    bool can_see_thing(const Thing* thing) const;
    
    
    float energy_;
    sf::Vector2f orientation_;
    float idle_energy_consumption_ = 0.1f;
    float movement_energy_consumption_ = 1.0f;
    float energy_per_offspring_ = 1.0f;
    float cos_vision_angle_ = 0.0f;
    Creature* attackable_creature_ = nullptr;
    Plant* nearby_plant_ = nullptr;
};
