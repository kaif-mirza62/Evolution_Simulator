#pragma once

#include "types.hpp"
#include "brain.hpp"
//initial creature traits

struct Creature{
    
    int id = 0;
    CreatureType type = CreatureType::Prey;

    float x = 0.0f;
    float y = 0.0f;

    float speed = 2.0f;
    float vision = 70.0f;
    float stamina = 100.0f;
    float hunting = 50.0f;
    float escape = 50.0f;

    float energy = 100.0f;
    float age = 0.0f;
    float maxAge = 25.0f;

    bool alive = true;

    Brain brain;

    DeathCause deathCause = DeathCause::None;

    int foodEaten = 0;
    int kills = 0;
    LearningState learningState{};

   Action learningInstinct = Action::Wander;
   Action learningAction = Action::Wander;
   float learningReward = 0.0f;
   bool learningTransitionPending = false;
};
