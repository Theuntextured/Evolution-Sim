#include "Creature.h"

#include <assert.h>

size_t Creature::creatures_count = 0;
size_t Plant::plant_count = 0;

Thing::Thing()
{
    position = sf::Vector2f(random_float(world_extent.x), random_float(world_extent.y));
    shape_ = nullptr;
}

Thing::~Thing()
{
    delete shape_;
}

Plant::Plant()
{
    size = 0.0f;
    shape_ = new sf::CircleShape(size, DRAW_RESOLUTION);
    shape_->setPosition(position);
    shape_->setFillColor(sf::Color::White);
    shape_->setOutlineColor(color_);
    shape_->setOutlineThickness(1.0f);
    
    plant_count++;
}

Plant::~Plant()
{
    plant_count--;
}

void Plant::tick(const float dt, std::vector<Thing*>& things_in_world)
{
    if(is_overlapping_plant(things_in_world))
    {
        if(size == 0.0f)
            alive = false;
        return;
    }
    size += plant_growth_rate * dt;
}

void Plant::draw(sf::RenderWindow& window)
{
    shape_->setRadius(size);
    shape_->setOrigin(size / 2, size / 2);
    shape_->setOutlineThickness(size * -0.25f);
    window.draw(*shape_);
}

Plant* Thing::is_overlapping_plant(const std::vector<Thing*>& things_in_world) const
{
    for(Thing* thing : things_in_world)
    {
        const auto p = dynamic_cast<Plant*>(thing);
        
        if (!p || p == this)
            continue;
        
        if (is_overlapping_other(p))
            return p;
    }
    return nullptr;
}

bool Thing::is_overlapping_other(const Thing* other) const
{
    return other && vector_length_squared(position - other->position) < square(size + other->size);
}

Creature::Creature()
{
    creatures_count++;
    energy_ = energy_storage;
    neural_network = new NeuralNetwork();
    calculate_energy_consumptions();
    gene = static_cast<Gene>(random() % 0xFFFF);
    color_ = sf::Color(
        static_cast<sf::Uint8>(random() % 256),
        static_cast<sf::Uint8>(random() % 256),
        static_cast<sf::Uint8>(random() % 256));

    setup_shape();
    reproduction_clock_.restart();
}

Creature::Creature(const Creature& other)
 : Thing(other)
{
 creatures_count++;
    
    neural_network = new NeuralNetwork(*other.neural_network);
    speed = mutate_property(other.speed);
    color_ = mutate_color(other.color_);
    gene = mutate_gene(other.gene);
    size = mutate_property(other.size);
    vision_angle = mutate_property(other.vision_angle);
    strength = mutate_property(other.strength);
    energy_storage = mutate_property(other.energy_storage);
    diet = mutate_gene(other.diet);
    average_offspring_count = mutate_property(other.average_offspring_count);
    max_offspring_offset = mutate_property(other.max_offspring_offset);
    age_to_reproduce = mutate_property(other.age_to_reproduce);
    
    position = other.position;
    
    alive = true;
    energy_ = energy_storage;
    calculate_energy_consumptions();

    setup_shape();
    reproduction_clock_.restart();
}

Creature::~Creature()
{
    creatures_count--;
    delete neural_network;
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

void Creature::setup_shape()
{
    shape_ = new sf::CircleShape(size, DRAW_RESOLUTION);
    shape_->setFillColor(color_);
    shape_->setOrigin(size / 2.0f, size / 2.0f);
    shape_->setOutlineColor(sf::Color::White);
    shape_->setOutlineThickness(size * -0.25f);

    direction_shape_.setSize(sf::Vector2f(size * 4.5f, size * 0.25f));
    direction_shape_.setFillColor(color_);
    direction_shape_.setOrigin(direction_shape_.getSize().x / 2.0f, 0.0f);

#if DRAW_DEBUG_DATA
    debug_text_.setFont(global_font);
    debug_text_.setString("Creature");
    debug_text_.setFillColor(sf::Color::Magenta);
    debug_text_.setOutlineColor(sf::Color::Black);
    debug_text_.setOutlineThickness(0.25f);
    debug_text_.setCharacterSize(11);
#endif
}

void Creature::tick(const float dt, std::vector<Thing*>& things_in_world)
{
    attackable_creature_ = nullptr;
    nearby_plant_ = nullptr;
    
    float params[static_cast<size_t>(OutputNode::Num)];
    get_neural_network_outputs(params, things_in_world);

    const sf::Vector2f current_speed = clamp_vec_size(
        sf::Vector2f(
            params[static_cast<size_t>(OutputNode::MoveRight)],
            params[static_cast<size_t>(OutputNode::MoveUp)]),
        1.0f) * speed;
    position += dt * current_speed;
    position = {clamp(position.x, 0.0f, world_extent.x),
        clamp(position.y, 0.0f, world_extent.y)};

    if(params[static_cast<size_t>(OutputNode::Reproduce)] > 0.0f)
        reproduce(things_in_world);
    
    if(params[static_cast<size_t>(OutputNode::Attack)] > 0.0f)
        attempt_attack();

    if(!alive) return;
    
    energy_ -= dt * (vector_length(current_speed) * movement_energy_consumption_ + idle_energy_consumption_);

    if(nearby_plant_)
        if(can_eat(nearby_plant_) && nearby_plant_->alive)
        {
            auto previous_energy = energy_;
            energy_ = std::min(energy_ + (plant_energy_per_unit_size * nearby_plant_->size), energy_storage);
            
            nearby_plant_->size -= (energy_ - previous_energy) / plant_energy_per_unit_size;
            if(nearby_plant_->size <= 0.0f)
                nearby_plant_->alive = false;
        }

    orientation_ = normalize(current_speed, {1.0f, 0.0f});

    if(energy_ <= 0)
        alive = false;
}

void Creature::draw(sf::RenderWindow& window)
{
#if DRAW_DEBUG_DATA
    direction_shape_.setPosition(position);
    direction_shape_.setRotation(atan2f(-orientation_.y, orientation_.x) / PI_F * 180.0f);
    window.draw(direction_shape_);
#endif
    
    shape_->setPosition(position);
    window.draw(*shape_);
    
#if DRAW_DEBUG_DATA
    debug_text_.setPosition(position);
    debug_text_.setString(sf::String(
        "Energy: " + std::to_string(energy_) +
        "\nIs Overlapping Plant: " + (static_cast<bool>(nearby_plant_) ? "True" : "False") +
        "\nGene: " + std::to_string(gene) +
        "\nDiet: " + std::to_string(diet)
    ));
    window.draw(debug_text_);
#endif
}

template <typename T>
T Creature::mutate_property(const T& property)
{
    if(!random_chance(0.01f))
        return property;

    return static_cast<T>(static_cast<float>(property) * random_float(0.9f, 1.1f));
}

Gene Creature::mutate_gene(const Gene g)
{
    Gene mod = 0;
    for(size_t i = 0; i < sizeof(gene); i++)
    {
        mod <<= 1;
        if(!random_chance(0.01f))
            continue;
        mod += 1;
    }
    return g ^ mod;
}

sf::Color Creature::mutate_color(const sf::Color& color)
{
    return {
        mutate_property(color.r),
        mutate_property(color.g),
        mutate_property(color.b)
    };
}

void Creature::calculate_energy_consumptions()
{
    idle_energy_consumption_ = energy_storage * 0.01f +
        vision_distance * 0.015f +
        vision_angle * 0.001f +
        strength * 0.01f +
        size * 0.02f +
        neural_network->get_complexity_factor();
    movement_energy_consumption_ = 0.1f * (size * 0.1f +
        speed * 0.05f);
    energy_per_offspring_ = size * 1.5f;
}

void Creature::get_neural_network_parameters(float* out, const std::vector<Thing*>& things_in_world)
{
    float closest_pray_s = FLT_MAX;
    float closest_attacker_s = FLT_MAX;
    Thing* attacker = nullptr;
    Thing* pray = nullptr;
    
    for(const auto i : things_in_world)
    {
        const bool is_pray = can_eat(i);
        const bool is_predator = can_be_eaten(i);

        if(is_pray || is_predator)
        {
            const float n = vector_length_squared(position - i->position) - i->size;

            if(is_predator && can_see_thing(i) && (closest_attacker_s > n))
            {
                closest_attacker_s = n;
                attacker = i;
            }
            if(is_pray && can_see_thing(i) && (closest_pray_s > n))
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
    }
    nearby_plant_ = is_overlapping_other(pray) ? dynamic_cast<Plant*>(pray) : nullptr;
    //assert(static_cast<bool>(is_overlapping_plant(things_in_world)) == static_cast<bool>(nearby_plant_));
    //nearby_plant_ = is_overlapping_plant(things_in_world);

    out[static_cast<size_t>(InputNode::CanSeeAttacker)] = attacker ? 1.f : 0.f;
    out[static_cast<size_t>(InputNode::CanSeePray)] = pray ? 1.f : 0.f;
    
    out[static_cast<size_t>(InputNode::NearestAttackerXOffset)] = attacker ? attacker->position.x - position.x : 0;
    out[static_cast<size_t>(InputNode::NearestAttackerYOffset)] = attacker ? attacker->position.y - position.y : 0;

    out[static_cast<size_t>(InputNode::NearestPrayXOffset)] = pray ? pray->position.x - position.x : 0;
    out[static_cast<size_t>(InputNode::NearestPrayYOffset)] = pray ? pray->position.y - position.y : 0;

    out[static_cast<size_t>(InputNode::CurrentEnergy)] = energy_;
    
    //out[static_cast<size_t>(InputNode::RandomA)] = random_float();
    //out[static_cast<size_t>(InputNode::RandomB)] = random_float();

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
    if(reproduction_clock_.getElapsedTime().asSeconds() < age_to_reproduce / time_speed_modifier)
        return;
    unsigned int offspring_count = static_cast<unsigned int>(std::max(0.0f, random_float(
        average_offspring_count - max_offspring_offset,
        average_offspring_count + max_offspring_offset)));

    const float energy_required = energy_per_offspring_ * static_cast<float>(offspring_count);

    if(energy_ < energy_required)
        return;

    energy_ -= energy_required;

    for(unsigned int i = 0; i < offspring_count; i++)
        things_in_world.push_back(new Creature(*this));

    reproduction_clock_.restart();
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
    if(is_overlapping_other(thing))
        return true;
    const auto offset = thing->position - position;

    if (vector_length_squared(offset) > vision_distance * vision_distance)
        return false;

    const auto offset_len = vector_length(offset);
    return vision_angle >= acos_deg(dot(offset / offset_len, orientation_)) - atan_deg(thing->size / offset_len);
}
