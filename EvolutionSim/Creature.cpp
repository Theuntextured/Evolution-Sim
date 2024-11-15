#include "Creature.h"

Thing::Thing()
{
    position = sf::Vector2f(random_float(world_extent.x), random_float(world_extent.y));
}

Plant::Plant()
: Thing()
{
    size = 0.0f;
}

void Plant::tick(const float dt, std::vector<Thing*>& things_in_world)
{
    size += plant_growth_rate * dt;
}

Creature::Creature()
: Thing()
{
    energy_ = energy_storage;
    calculate_energy_consumptions();
    neural_network = new NeuralNetwork();
    gene = static_cast<Gene>(random() % 0xFFFF);
    color = sf::Color(
        static_cast<sf::Uint8>(random() % 256),
        static_cast<sf::Uint8>(random() % 256),
        static_cast<sf::Uint8>(random() % 256));
    cos_vision_angle_ = cos(vision_angle * 0.0174533f);    
}

Creature::Creature(const Creature& other)
 : Thing(other)
{
 energy_ = energy_storage;
    neural_network = new NeuralNetwork(*other.neural_network);
    speed = other.speed;
    color = other.color;
    gene = other.gene;
    size = other.size;
    position = other.position;
    alive = true;
    vision_angle = other.vision_angle;
    strength = other.strength;
    energy_storage = other.energy_storage;
    diet = other.diet;
    average_offspring_count = other.average_offspring_count;
    max_offspring_offset = other.max_offspring_offset;

    
    cos_vision_angle_ = cos(vision_angle * 0.0174533f);
    calculate_energy_consumptions();
}

bool Creature::can_eat(const Thing* other) const
{
    return other->gene & diet;
}

bool Creature::can_be_eaten(const Thing* other) const
{
    auto o = dynamic_cast<const Creature*>(other);
    if(!o) return false;
    return o->diet & gene;
}

void Creature::tick(const float dt, std::vector<Thing*>& things_in_world)
{
    float params[static_cast<size_t>(OutputNode::Num)];
    get_neural_network_parameters(params, things_in_world);

    sf::Vector2f current_speed = clamp_vec_size(
        sf::Vector2f(
            params[static_cast<size_t>(OutputNode::MoveRight)],
            params[static_cast<size_t>(OutputNode::MoveUp)]),
        1.0f);
    position += dt * current_speed;

    if(params[static_cast<size_t>(OutputNode::Reproduce)] > 0.0f)
        reproduce(things_in_world);

    if(!alive) return;

    if(params[static_cast<size_t>(OutputNode::Attack)] > 0.0f)
        attempt_attack();

    energy_ -= dt * (vector_length(current_speed) * movement_energy_consumption_ + idle_energy_consumption_);

    if(nearby_plant_)
        if(can_eat(nearby_plant_) && nearby_plant_->alive)
        {
            nearby_plant_->alive = false;
            energy_ = std::min(energy_ + (plant_energy_per_unit_size * nearby_plant_->size), energy_storage);
        }
}

template <typename T>
T Creature::mutate_property(const T& property)
{
    if(!random_chance(0.01f))
        return property;

    return property * random_float(0.9, 1.1);
}

Gene Creature::mutate_gene(const Gene g)
{
    Gene mod = 0;
    for(size_t i = 0; i < sizeof(Gene); i++)
    {
        mod <<= 1;
        if(!random_chance(0.01f))
            continue;
        mod += 1;
    }
    return g ^ mod;
}

void Creature::calculate_energy_consumptions()
{
    idle_energy_consumption_ = energy_storage * 0.01f +
        vision_distance * 0.015f +
        vision_angle * 0.001f +
        strength * 0.01f +
        size * 0.1f +
        neural_network->get_complexity_factor();
    movement_energy_consumption_ = size * 0.5f +
        speed * 0.05f;
    energy_per_offspring_ = size * 1.5f;        
}

void Creature::get_neural_network_parameters(float* out, const std::vector<Thing*>& things_in_world)
{
    float closest_pray_s = vision_distance * vision_distance + 1;
    float closest_attacker_s = vision_distance * vision_distance + 1;
    Thing* attacker = nullptr;
    Thing* pray = nullptr;
    
    for(auto i : things_in_world)
    {
        if(can_be_eaten(i))
        {
            auto n = vector_length_squared(position - i->position);
            if(can_see_thing(i) && (closest_attacker_s > n))
            {
                closest_attacker_s = n;
                attacker = i;
            }
        }
        if(can_eat(i))
        {
            auto n = vector_length_squared(position - i->position);
            if(can_see_thing(i) && (closest_pray_s > n))
            {
                closest_pray_s = n;
                pray = i;
            }
        }
    }

    sf::Vector2f distance_to_border = position;
    if(distance_to_border.x > world_extent.x / 2)
        distance_to_border.x = position.x - world_extent.x;
    if(distance_to_border.y > world_extent.y / 2)
        distance_to_border.y = position.y - world_extent.y;

    if(attacker)
    {
        attackable_creature_ = dynamic_cast<Creature*>(attacker);
    }
    else if(pray)
    {
        attackable_creature_ = dynamic_cast<Creature*>(pray);
        nearby_plant_ = dynamic_cast<Plant*>(pray);
    }

    out[static_cast<size_t>(InputNode::CanSeeAttacker)] = attacker ? 1 : 0;
    out[static_cast<size_t>(InputNode::CanSeePray)] = pray ? 1 : 0;
    
    out[static_cast<size_t>(InputNode::NearestAttackerXOffset)] = attacker ? attacker->position.x - position.x : 0;
    out[static_cast<size_t>(InputNode::NearestAttackerYOffset)] = attacker ? attacker->position.y - position.y : 0;

    out[static_cast<size_t>(InputNode::NearestPrayXOffset)] = pray ? pray->position.x - position.x : 0;
    out[static_cast<size_t>(InputNode::NearestPrayYOffset)] = pray ? pray->position.y - position.y : 0;

    out[static_cast<size_t>(InputNode::CurrentEnergy)] = energy_;
    
    out[static_cast<size_t>(InputNode::RandomA)] = random_float();
    out[static_cast<size_t>(InputNode::RandomB)] = random_float();

    out[static_cast<size_t>(InputNode::DistanceToNearestBorderX)] = distance_to_border.x;
    out[static_cast<size_t>(InputNode::DistanceToNearestBorderY)] = distance_to_border.y;
}

void Creature::get_neural_network_outputs(float* out, const std::vector<Thing*>& things_in_world)
{
    float params[static_cast<size_t>(InputNode::Num)];
    get_neural_network_parameters(params, things_in_world);

    neural_network->get_values(params, out);
}

void Creature::reproduce(std::vector<Thing*>& things_in_world)
{
    unsigned int offspring_count = static_cast<unsigned int>(std::min(0.0f, random_float(
        average_offspring_count - max_offspring_offset,
        average_offspring_count + max_offspring_offset)));

    float energy_required = energy_per_offspring_ * static_cast<float>(offspring_count);

    if(energy_ < energy_required)
        return;

    energy_ -= energy_required;

    for(unsigned int i = 0; i < offspring_count; i++)
        things_in_world.push_back(new Creature(*this));
}

void Creature::attempt_attack()
{
    if(!attackable_creature_ || !alive)
        return;
    if(vector_length_squared(position - attackable_creature_->position) > size || !attackable_creature_->alive)
        return;
    
    auto energy_usage = std::min(energy_, strength * size);
    auto other_energy_usage = std::min(
        attackable_creature_->energy_,
        attackable_creature_->strength * attackable_creature_->size);

    if(energy_usage == other_energy_usage)  // NOLINT(clang-diagnostic-float-equal)
        return;

    Creature* winner;
    Creature* loser;
    
    if(energy_usage < other_energy_usage)
    {
        winner = attackable_creature_;
        loser = this;
    }
    else
    {
        winner = this;
        loser = attackable_creature_;
    }

    loser->alive = false;
    winner->energy_ = std::min(winner->energy_storage,
        winner->energy_ + loser->energy_ - std::min(energy_usage, other_energy_usage));
    
}

bool Creature::can_see_thing(const Thing* thing) const
{
    auto offset = normalize(thing->position - position);
    if(dot(offset, orientation_) > cos_vision_angle_ && len)
        
}
