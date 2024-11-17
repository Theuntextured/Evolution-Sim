#include "NeuralNetwork.h"

float ActivationFunctions::binary_step(const float x)
{
    return x >= 0.0f ? 1.0f : 0.0f;
}

float ActivationFunctions::sign(const float x)
{
    if (x > 0) return 1;
    if (x < 0) return -1;
    return 0;
}

float ActivationFunctions::linear(const float x)
{
    return x;
}

float ActivationFunctions::sigmoid(const float x)
{
    return 1.0f / (1.0f + exp(-x));
}

float ActivationFunctions::tanh(const float x)
{
    return tanhf(x);
}

float ActivationFunctions::re_lu(const float x)
{
    return std::max(0.0f, x);
}

float ActivationFunctions::leaky_re_lu(const float x)
{
    return std::max(0.1f * x, x);
}

float ActivationFunctions::elu(const float x)
{
    return x >= 0 ? x : alpha * (exp(x) - 1);
}

ActivationFunctions::ActivationFunction::ActivationFunction(float func(const float x), const float complexity)
{
    this->func_ = func;
    this->complexity_ = complexity;
}

inline float ActivationFunctions::ActivationFunction::parse_value(const float x) const
{
    return func_(x);
}

float ActivationFunctions::ActivationFunction::get_complexity() const
{
    return complexity_;
}

Node::Node(const bool is_input)
{
    is_input_node = is_input;
    activation_function_ = &ActivationFunctions::functions[random() % ActivationFunctions::functions.size()];
    bias_ = 0.0f;
}

Node::Node(const Node& other)
{
    activation_function_ = other.activation_function_;
    bias_ = other.bias_;
    is_input_node = other.is_input_node;
    if(random_chance(NeuralNetworkSettings::node_bias_mutation_chance))
        bias_ = bias_ *
            random_float(1 - NeuralNetworkSettings::node_bias_mutation_delta,
                1 + NeuralNetworkSettings::node_bias_mutation_delta) +
            random_float(-NeuralNetworkSettings::node_bias_mutation_delta,
                NeuralNetworkSettings::node_bias_mutation_delta);

    if(random_chance(NeuralNetworkSettings::node_activation_function_mutation_chance))
        activation_function_ = &ActivationFunctions::functions[random() % ActivationFunctions::functions.size()];
    
}

Node::~Node()
{
    for(auto& connection : connections_)
        delete connection;
    connections_.clear();
}

void Node::calculate_output()
{
    output_ = bias_;
    for(const auto i : connections_)
        output_ += i->from->get_value(last_request_id_) * i->weight;
    output_ = activation_function_->parse_value(output_);
}

float Node::get_value(const sf::Uint8 request_id)
{
    if(request_id == last_request_id_ || is_input_node)
        return output_;
    last_request_id_ = request_id;
    calculate_output();
    return output_;
}

void Node::add_connection(Node* other, const float weight)
{
    connections_.push_back(new Connection(other, weight));
}

Connection::Connection(Node* from, const float weight)
{
    this->from = from;
    this->weight = weight;
}

NeuralNetwork::NeuralNetwork()
{
    for (auto& input : inputs)
        input = new Node(true);
    
    auto internal_node_count = NeuralNetworkSettings::width * NeuralNetworkSettings::depth;
    internal_nodes.reserve(internal_node_count);
    for(size_t i = 0; i < internal_node_count; i++)
    {
        auto new_node = new Node(false);
        internal_nodes.emplace_back(new_node);
        if(i / NeuralNetworkSettings::width == 0)
        {
            for (auto& input : inputs)
                new_node->add_connection(input, random_float(-2.0f, 2.0f));
        }
        else
        {
            for(size_t j = 0; j < NeuralNetworkSettings::width; j++)
                new_node->add_connection(internal_nodes[(i / NeuralNetworkSettings::width - 1) * NeuralNetworkSettings::width + j], random_float(-2.0f, 2.0f));
        }
        complexity_ += new_node->get_complexity();
    }

    for (auto& output : outputs)
    {
        const auto new_node = new Node(false);
        output = new_node;
        complexity_ += new_node->get_complexity();
        
        for(size_t j = 0; j < NeuralNetworkSettings::width; j++)
            new_node->add_connection(internal_nodes[internal_nodes.size() - 1 - j], random_float(-2.0f, 2.0f));
    }
}

NeuralNetwork::NeuralNetwork(const NeuralNetwork& other)
{
    for(size_t i = 0; i < static_cast<size_t>(InputNode::Num); i++)
        inputs[i] = new Node(*other.inputs[i]);

    internal_nodes.reserve(other.internal_nodes.size());
    for(size_t i = 0; i < other.internal_nodes.size(); i++)
    {
        auto new_node =new Node(*other.internal_nodes[i]);
        internal_nodes.push_back(new_node);
        
        if(i / NeuralNetworkSettings::width == 0)
        {
            for(size_t j = 0; j < static_cast<size_t>(InputNode::Num); j++)
                new_node->add_connection(inputs[j],
                    mutate_connection_weight(other.internal_nodes[i]->get_connections()[j]->weight));
        }
        else
        {
            for(size_t j = 0; j < NeuralNetworkSettings::width; j++)
                new_node->add_connection(internal_nodes[(i / NeuralNetworkSettings::width - 1) * NeuralNetworkSettings::width + j],
                    mutate_connection_weight(other.internal_nodes[i]->get_connections()[j]->weight));
        }
        complexity_ += new_node->get_complexity();
    }

    for(size_t i = 0; i < static_cast<size_t>(OutputNode::Num); i++)
    {
        const auto new_node = new Node(other.outputs[i]);
        outputs[i] = new_node;
        complexity_ += new_node->get_complexity();
        
        for(size_t j = 0; j < NeuralNetworkSettings::width; j++)
            new_node->add_connection(internal_nodes[internal_nodes.size() - 1 - j],
                mutate_connection_weight(other.outputs[i]->get_connections()[j]->weight));
    }
}

NeuralNetwork::~NeuralNetwork()
{
    for(const auto& input : inputs)
        delete input;
    for(const auto& output : outputs)
        delete output;
    for(const auto& internal_node : internal_nodes)
        delete internal_node;
    internal_nodes.clear();
}

void NeuralNetwork::get_values(const float* in, float* out)
{
    last_request_id++;
    for(size_t i = 0; i < static_cast<size_t>(InputNode::Num); i++)
        inputs[i]->set_value(in[i]);
    for(size_t i = 0; i < static_cast<size_t>(OutputNode::Num); i++)
        out[i] = outputs[i]->get_value(last_request_id);
}

float NeuralNetwork::mutate_connection_weight(const float weight)
{
    if(!random_chance(NeuralNetworkSettings::connection_weight_mutation_chance))
        return weight;
    return weight *
        random_float(1 - NeuralNetworkSettings::connection_weight_mutation_delta,
            1 + NeuralNetworkSettings::connection_weight_mutation_delta) +
        random_float(-NeuralNetworkSettings::connection_weight_mutation_delta,
            NeuralNetworkSettings::connection_weight_mutation_delta);    
}
