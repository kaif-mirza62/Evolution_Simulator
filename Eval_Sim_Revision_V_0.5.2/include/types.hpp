#pragma once

#include <array>
#include <cstddef>

//defines the type of the creature

enum class CreatureType{
    Prey,
    Predator
};
//defines what action the creature can do
enum class Action{
    Wander,
    FindFood,
    Escape,
    Hunt,
    Rest
};
//the ways the creature can die
enum class DeathCause{
    None,
    Starvation,
    Predation,
    OldAge
};

constexpr std::size_t LEARNING_STATE_SIZE = 30;

using LearningState = std::array<float,LEARNING_STATE_SIZE>;

struct Experience{
    LearningState state{};
    Action action = Action::Wander;
    float reward = 0.0f;
    LearningState nextState{};
    bool done = false;

};
