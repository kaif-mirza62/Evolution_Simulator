#pragma once

#include "raylib.h"
#include <random>
#include <vector>

struct Food{
    public:
    double energy = 40;
    Vector2 position;
};

class Environment{
    public:
    std::vector<Food> foods;

    void generateFood(int amount , std::mt19937 &randomGenerator, double worldWidth, double worldHeight);

    void clearFood();

    void draw() const;
};
