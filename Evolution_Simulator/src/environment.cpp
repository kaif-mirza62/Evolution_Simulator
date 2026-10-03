#include "environment.hpp"

void Environment::generateFood(int amount, std::mt19937 &randomGenerator,double worldWidth, double worldHeight){
    std::uniform_real_distribution<float> xDistribution(10.0f, worldWidth - 10.0f);
    std::uniform_real_distribution<float> yDistribution(10.0f, worldHeight - 10.0f);
    
    for(int foodNumber = 0; foodNumber < amount; foodNumber++){
        Food food;
        food.position = {
            xDistribution(randomGenerator),
            yDistribution(randomGenerator)
        };
        foods.push_back(food);
    }
}

void Environment::clearFood(){
    foods.clear();
}
void Environment::draw() const {
    for(const Food &food : foods){
        DrawCircleV(food.position, 3.0f, GREEN);
    }
}