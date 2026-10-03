#pragma once

#include "raylib.h"

enum class CreatureType{
    Prey,Predator
};

struct Creature{
    public:
    float speed = 2;
    float x = 0;
    float y = 0;
    int foodEaten = 0;
    float vision = 0;
    float energy = 100;

    CreatureType type = CreatureType::Prey;
};