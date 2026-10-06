#include "environment.hpp"

void Environment::generateFood(
    int amount,
    float worldWidth,
    float worldHeight,
    std::mt19937& rng
)
{
    if (amount <= 0)
    {
        return;
    }

    float padding = 15.0f;

    std::uniform_real_distribution<float> xDist(
        padding,
        worldWidth - padding
    );

    std::uniform_real_distribution<float> yDist(
        padding,
        worldHeight - padding
    );

    for (int i = 0; i < amount; i++){
        Food food;

        food.position = {
            xDist(rng),
            yDist(rng)
        };

        foods.push_back(food);
    }
}

void Environment::loadBackground()
{
    background = LoadTexture("assets/environment.png");
}

void Environment::unloadBackground()
{
    if (background.id != 0)
    {
        UnloadTexture(background);
        background = {};
    }
}

void Environment::draw(
    float worldWidth,
    float worldHeight
) const
{

    if (background.id != 0)
    {
        Rectangle source{
            0.0f,
            0.0f,
            static_cast<float>(background.width),
            static_cast<float>(background.height)
        };

        Rectangle destination{
            0.0f,
            0.0f,
            worldWidth,
            worldHeight
        };

        DrawTexturePro(
            background,
            source,
            destination,
            Vector2{0.0f, 0.0f},
            0.0f,
            WHITE
        );
    }
}