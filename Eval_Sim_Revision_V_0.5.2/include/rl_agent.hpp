#pragma once
#include "neural_network.hpp"
#include "replay_buffer.hpp"
#include <cstddef>
#include <random>

class RLAgent
{
public:

    RLAgent(
        CreatureType type,
        std::mt19937& rng
    );

    Action chooseAction(
        const LearningState& state,
        Action instinct
    );

    void remember(const Experience& experience );

    float train();

    void reset();

    float getEpsilon() const;

    std::size_t getMemorySize() const;

    std::size_t getTrainingSteps() const;

    float getLastLoss() const;

private:

    static constexpr std::size_t BUFFER_CAPACITY = 100000;

    static constexpr std::size_t BATCH_SIZE = 100;

    static constexpr std::size_t WARMUP_EXPERIENCES = 512;

    static constexpr std::size_t TARGET_UPDATE_STEPS = 250;

    static constexpr std::size_t EPSILON_DECAY_STEPS = 20000;

    static constexpr float LEARNING_RATE = 0.5;

    static constexpr float GAMMA = 0.97f;

    static constexpr float INITIAL_EPSILON = 1.0f;

    static constexpr float MIN_EPSILON = 0.05f;

    static constexpr float INSTINCT_BIAS = 0.20f;

    CreatureType type;

    std::mt19937& rng;

    NeuralNetwork onlineNetwork;

    NeuralNetwork targetNetwork;

    ReplayBuffer replayBuffer;

    float epsilon = INITIAL_EPSILON;

    std::size_t trainingSteps = 0;

    float lastLoss = 0.0f;

    bool isValidAction(
        Action action
    ) const;

    Action randomValidAction();

    int bestValidAction(const NeuralNetwork::Output& qValues,Action instinct ) const;
};
