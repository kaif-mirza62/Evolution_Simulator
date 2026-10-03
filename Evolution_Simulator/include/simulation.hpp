#pragma once

#include "creature.hpp"
#include "environment.hpp"
#include "raylib.h"
#include <random>
#include <vector>

const float WORLD_WIDTH = 760.0f;
const float WORLD_HEIGHT = 600.0f;
const float GENERATION_LENGTH = 8.0f;
const float PREY_START_ENERGY = 100.0f;
const float PREDATOR_START_ENERGY = 120.0f;
const float FOOD_ENERGY = 40.0f;
const float PREY_ENERGY_COST = 4.0f;
const float PREDATOR_ENERGY_COST = 6.0f;
const float PREDATOR_FOOD_ENERGY = 80.0f;
const float PREDATOR_ATTACK_RANGE = 10.0f;
const float FOOD_EAT_RANGE = 8.0f;
const float SECOND_CHILD_ENERGY = 170.0f;

class Simulation{
    public:
    std::vector<Creature> creatures;
    Environment environment;

    int generation = 1;
    int maxGenerations = 20;
    float generationTimer = 0.0f;
    bool finished = false;
    
    int startingPrey = 0;
    int startingPredators = 0;
    int startingFood = 0;
    
    int preyBorn = 0;
    int predatorBorn = 0;

    int preyDied = 0;
    int predatorDied = 0;
    int preyEaten = 0;
    int foodEaten = 0;

    int peakPopulation = 0;
    int peakPrey = 0;
    int peakPredators = 0;

    int generationsCompleted = 0;

    float initialPreySpeed = 0;
    float finalPreySpeed = 0;

    float initialPredatorSpeed = 0;
    float finalPredatorSpeed = 0;

    float initialPreyVision = 0;
    float finalPreyVision = 0;

    float initialPredatorVision = 0;
    float finalPredatorVision = 0;

    Simulation();

    void start(int preyCount,int predatorCount, int foodCount, int generationCount);
    void update(float deltaTime);
    void draw() const;
    
    private:
    std::mt19937 randomGenerator;
    float randomFloat(float minimum, float maximum);
    float distanceBetween(float firstX, float firstY,float secondX,float secondY);

    void createStartingCreature(CreatureType type);
    void movePrey(float deltaTime);
    void movePredators(float deltaTime);
    void preyEatFood();
    void predatorsEatPrey();
    void updateEnergy(float deltaTime);
    void removeDeadCreatures();
    void finishGeneration();
    void createNextGeneration();
    void mutate(Creature &child);
    void updatePeakStatistics();
    void saveFinalAverages();
    float getAverageTrait(CreatureType type, bool getSpeed) const;
};