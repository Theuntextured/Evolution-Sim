#pragma once
#include "Common.h"

namespace ActivationFunctions
{
    constexpr float alpha = 0.1f;
    float binary_step(const float x);
    float sign(const float x);
    float linear(const float x);
    float sigmoid(const float x);
    float tanh(const float x);
    float reLU(const float x);
    float leaky_reLU(const float x);
    float elu(const float x);

    class ActivationFunction
    {
    private:
        float(*func)(const float x);
        float complexity;
    public:
        ActivationFunction(float(func)(const float x), const float complexity);
        inline float parse_value(const float x) const;
        inline float get_complexity() const;
    };
    
    static std::vector<ActivationFunction> functions = {
        ActivationFunction(binary_step, 0.1f),
        ActivationFunction(linear, 0.5f),
        ActivationFunction(sigmoid, 1.75f),
        ActivationFunction(tanh, 2.5f),
        ActivationFunction(reLU, 1.0f),
        ActivationFunction(leaky_reLU, 1.2f),
        ActivationFunction(elu, 3.0f)
    };
};

class Connection;

enum class OutputNode : size_t  // NOLINT(performance-enum-size)
{
    MoveUp,
    MoveRight,
    Reproduce, // >0 means reproduce
    Attack, // >0 means attack
    Num
};

enum class InputNode : size_t // NOLINT(performance-enum-size)
{
    CanSeeAttacker, // 1 or 0
    NearestAttackerXOffset,
    NearestAttackerYOffset,
    
    CanSeePray, // 1 or 0
    NearestPrayXOffset,
    NearestPrayYOffset,
    
    CurrentEnergy,
    
    RandomA, //0..1
    RandomB, //0..1
    
    DistanceToNearestBorderX, //signed
    DistanceToNearestBorderY, //signed
    
    Num
};

class Node
{
public:
    explicit Node(const bool is_input);
    Node(const Node& other);
    ~Node();
    void calculate_output();
    float get_value(const sf::Uint8 request_id);
    inline void set_value(const float value){output_ = value;}
    inline void add_connection(Node* other, const float weight);
    inline float get_complexity() const { return activation_function_->get_complexity(); }
    inline const std::vector<Connection*>& get_connections() const { return connections_; }

    bool is_input_node;
private:
    ActivationFunctions::ActivationFunction* activation_function_;
    std::vector<Connection*> connections_;
    sf::Uint8 last_request_id_ = 255;
    float output_ = 0.0f;
    float bias_;
};

class Connection
{
    public:
    Connection(Node* from, const float weight);
    Node* from;
    float weight;
};

class NeuralNetwork
{
public:
    NeuralNetwork();
    NeuralNetwork(const NeuralNetwork& other);
    ~NeuralNetwork();
    float get_complexity_factor() const{return complexity_;}
    void get_values(const float* in, float* out);
    static float mutate_connection_weight(const float weight);
    
    Node* inputs[size_t(InputNode::Num)];
    Node* outputs[size_t(OutputNode::Num)];
    std::vector<Node*> internal_nodes;
    sf::Uint8 last_request_id = 255;
private:
    float complexity_ = 0.0f;
};

namespace NeuralNetworkSettings
{
    static constexpr float node_bias_mutation_chance = 0.01f;
    static constexpr float node_bias_mutation_delta = 0.1f;
    static constexpr float connection_weight_mutation_chance = 0.01f;
    static constexpr float connection_weight_mutation_delta = 0.05f;
    static constexpr float node_activation_function_mutation_chance = 0.00125f;
    
    static constexpr size_t width = size_t(InputNode::Num) * 2;
    static constexpr size_t depth = 5;
}