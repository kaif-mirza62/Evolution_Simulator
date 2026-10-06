#pragma once

#include "types.hpp"
#include <random>

class Brain{

public:
    Action currentAction = Action::Wander;

    float wanderAngle = 0.0f;
    float decisionTimer = 0.0f;

    Action decide(
        CreatureType type,
        bool hungry,
        bool seesFood,
        bool seesDanger,
        bool seesPrey,
        std::mt19937& rng,
        float deltaTime
    );
};
