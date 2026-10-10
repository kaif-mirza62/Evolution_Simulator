#pragma once

#include "creature.hpp"
#include "environment.hpp"
#include "raylib.h"
#include <random>
#include <vector>
#include "rl_agent.hpp"

struct SimulationConfig
{
    int initialPrey = 30;
    int initialPredators = 8;
    int initialFood = 180;
};

struct TraitMeans
{
    float speed = 0.0f;
    float vision = 0.0f;
    float stamina = 0.0f;
    float hunting = 0.0f;
    float escape = 0.0f;
};

struct SimulationStats
{
    int initialPrey = 0;
    int initialPredators = 0;

    int finalPrey = 0;
    int finalPredators = 0;

    int peakPrey = 0;
    int peakPredators = 0;

    int preyBorn = 0;
    int predatorBorn = 0;

    int preyStarved = 0;
    int preyPredated = 0;
    int preyOldAge = 0;

    int predatorStarved = 0;
    int predatorOldAge = 0;

    int preyFoodEaten = 0;

    int successfulHunts = 0;
    int failedHunts = 0;

    int foodCreated = 0;
    int foodConsumed = 0;

    int completedGenerations = 0;

    TraitMeans initialPreyTraits;
    TraitMeans finalPreyTraits;

    TraitMeans initialPredatorTraits;
    TraitMeans finalPredatorTraits;
};

class Simulation{
public:
    Simulation();

    void loadTextures();
    void unloadTextures();

    void start(const SimulationConfig& newConfig);

    void update(float deltaTime);

    void draw(float worldWidth,float worldHeight) const;

    void nextGeneration();
    void finishExperiment();

    void setWorldSize(float width, float height);

    bool isRunning() const;
    bool isFinished() const;

    int getGeneration() const;
    int getGenerationCount() const;

    float getGenerationProgress() const;

    const SimulationConfig& getConfig() const;
    const SimulationStats& getStats() const;
    const std::vector<Creature>& getCreatures() const;

    int getPreyCount() const;
    int getPredatorCount() const;
    int getFoodCount() const;
    float getPreyEpsilon() const;

   float getPredatorEpsilon() const;

  std::size_t getPreyMemorySize() const;
  std::size_t getPredatorMemorySize() const;
  std::size_t getPreyTrainingSteps() const;
  std::size_t getPredatorTrainingSteps() const;

float getPreyLoss() const;

float getPredatorLoss() const;

    const Creature* findCreature(int id) const;

private:
    struct Perception{
    int nearestFood = -1;

    float nearestFoodDistance = 0.0f;

    Vector2 nearestFoodOffset{};

    int nearestPredator = -1;

    float nearestPredatorDistance = 0.0f;

    Vector2 nearestPredatorOffset{};

    int nearestPrey = -1;

    float nearestPreyDistance = 0.0f;

    Vector2 nearestPreyOffset{};

    int nearbyPredatorCount = 0;

    int nearbyPreyCount = 0;
};
Perception perceive(const Creature& creature) const;

LearningState buildLearningState(
    const Creature& creature,
    const Perception& perception,
    Action instinct
) const;

    void createInitialPopulation();
    void createFoodIfNeeded();
    
    Creature makeCreature(
        CreatureType type,
        int id
    );

    Creature makeChild(
        const Creature& firstParent,
        const Creature& secondParent,
        int id
    );

    void applyDeath(
        Creature& creature,
        DeathCause cause
    );

    void removeDeadCreatures();
    void recordFinalTraitMeans();

    float calculateFitness(
        const Creature& creature
    ) const;

    bool isInsideWorld(
        const Creature& creature
    ) const;

    SimulationConfig config;
    SimulationStats stats;

    std::vector<Creature> creatures;
    Environment environment;
    std::mt19937 rng;
    RLAgent preyRL;
    RLAgent predatorRL;

    int nextCreatureId = 1;
    int generation = 1;

    float generationTime = 0.0f;

    float worldWidth = 1000.0f;
    float worldHeight = 700.0f;
    float GENERATION_LENGTH = 6.0f;

    bool running = false;
    bool finished = false;

    Texture2D preyTexture{};
    Texture2D predatorTexture{};
};