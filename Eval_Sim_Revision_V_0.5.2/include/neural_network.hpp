#pragma once

#include "types.hpp"

#include <array>
#include <cstddef>
#include <random>
#include <utility>
#include <vector>

class NeuralNetwork
{
public:
    
    static constexpr std::size_t HIDDEN_1 = 32;

    static constexpr std::size_t HIDDEN_2 = 32;

    static constexpr std::size_t OUTPUTS = 5;

    using Output = std::array<float, OUTPUTS>;

    using TrainingExample = std::pair<LearningState, Output>;

    explicit NeuralNetwork( std::mt19937& rng );

    Output predict( const LearningState& state) const;

    float trainBatch(const std::vector<TrainingExample>& batch,float learningRate );

    void copyFrom( const NeuralNetwork& other);

private:

    std::array<float, LEARNING_STATE_SIZE * HIDDEN_1> w1{};

    std::array<float, HIDDEN_1> b1{};

    std::array<float, HIDDEN_1 * HIDDEN_2> w2{};

    std::array<float, HIDDEN_2> b2{};

    std::array<float, HIDDEN_2 * OUTPUTS> w3{};

    std::array<float, OUTPUTS> b3{};
};