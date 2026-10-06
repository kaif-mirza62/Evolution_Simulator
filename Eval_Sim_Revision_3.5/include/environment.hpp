#pragma once

#include "raylib.h"
#include <random>
#include <vector>

struct Food{
    Vector2 position{0.0f, 0.0f};
    float energy = 25.0f;
};

class Environment{
public:
   
    std::vector<Food> foods;
    Texture2D background{};

    void generateFood(
        int amount,
        float worldWidth,
        float worldHeight,
        std::mt19937& rng
    );

    void loadBackground();
    void unloadBackground();

    void draw(float worldWidth,float worldHeight) const;
};