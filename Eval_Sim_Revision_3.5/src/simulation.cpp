#define PI 3.14159265359f
#include "simulation.hpp"

#include "raylib.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace
{
    constexpr float PREY_SIZE = 5.0f;
    constexpr float PREDATOR_SIZE = 7.0f;
    constexpr float FOOD_SIZE = 4.0f;

    Texture2D environmentTexture{};
    Texture2D preyTexture{};
    Texture2D predatorTexture{};
    Texture2D foodTexture{};
}

Simulation::Simulation()
    : rng(std::random_device{}())
{
}

void Simulation::start(const SimulationConfig& newConfig)
{
    config = newConfig;

    if (environmentTexture.id != 0)
    {
        UnloadTexture(environmentTexture);
        environmentTexture = {};
    }

    if (preyTexture.id != 0)
    {
        UnloadTexture(preyTexture);
        preyTexture = {};
    }

    if (predatorTexture.id != 0)
    {
        UnloadTexture(predatorTexture);
        predatorTexture = {};
    }

    if (foodTexture.id != 0)
    {
        UnloadTexture(foodTexture);
        foodTexture = {};
    }

    environmentTexture = LoadTexture("assets/environment.png");
    preyTexture = LoadTexture("assets/predator.png");
    predatorTexture = LoadTexture("assets/prey.png");
    foodTexture = LoadTexture("assets/food.png");

    creatures.clear();
    environment.foods.clear();

    stats = SimulationStats{};

    nextCreatureId = 1;
    generation = 1;
    generationTime = 0.0f;

    running = true;
    finished = false;

    createInitialPopulation();

    environment.generateFood(
        config.initialFood,
        worldWidth,
        worldHeight,
        rng
    );

    stats.foodCreated += config.initialFood;

    stats.initialPrey = getPreyCount();
    stats.initialPredators = getPredatorCount();

    stats.peakPrey = stats.initialPrey;
    stats.peakPredators = stats.initialPredators;

    TraitMeans preyMeans{};
    TraitMeans predatorMeans{};

    int preyCount = 0;
    int predatorCount = 0;

    for (const Creature& creature : creatures)
    {
        if (creature.type == CreatureType::Prey)
        {
            preyMeans.speed += creature.speed;
            preyMeans.vision += creature.vision;
            preyMeans.stamina += creature.stamina;
            preyMeans.hunting += creature.hunting;
            preyMeans.escape += creature.escape;
            preyCount++;
        }
        else
        {
            predatorMeans.speed += creature.speed;
            predatorMeans.vision += creature.vision;
            predatorMeans.stamina += creature.stamina;
            predatorMeans.hunting += creature.hunting;
            predatorMeans.escape += creature.escape;
            predatorCount++;
        }
    }

    if (preyCount > 0)
    {
        preyMeans.speed /= preyCount;
        preyMeans.vision /= preyCount;
        preyMeans.stamina /= preyCount;
        preyMeans.hunting /= preyCount;
        preyMeans.escape /= preyCount;
    }

    if (predatorCount > 0)
    {
        predatorMeans.speed /= predatorCount;
        predatorMeans.vision /= predatorCount;
        predatorMeans.stamina /= predatorCount;
        predatorMeans.hunting /= predatorCount;
        predatorMeans.escape /= predatorCount;
    }

    stats.initialPreyTraits = preyMeans;
    stats.initialPredatorTraits = predatorMeans;
}

void Simulation::createInitialPopulation()
{
    for (int i = 0; i < config.initialPrey; i++)
    {
        creatures.push_back(
            makeCreature(CreatureType::Prey, nextCreatureId++)
        );
    }

    for (int i = 0; i < config.initialPredators; i++)
    {
        creatures.push_back(
            makeCreature(CreatureType::Predator, nextCreatureId++)
        );
    }
}

Creature Simulation::makeCreature(
    CreatureType type,
    int id
)
{
    Creature creature;

    creature.id = id;
    creature.type = type;

    std::uniform_real_distribution<float> xDist(
        20.0f,
        worldWidth - 20.0f
    );

    std::uniform_real_distribution<float> yDist(
        20.0f,
        worldHeight - 20.0f
    );

    creature.x = xDist(rng);
    creature.y = yDist(rng);

    if (type == CreatureType::Prey)
    {
        std::uniform_real_distribution<float> speedDist(1.5f, 4.0f);
        std::uniform_real_distribution<float> visionDist(45.0f, 100.0f);
        std::uniform_real_distribution<float> staminaDist(80.0f, 120.0f);
        std::uniform_real_distribution<float> escapeDist(35.0f, 75.0f);

        creature.speed = speedDist(rng);
        creature.vision = visionDist(rng);
        creature.stamina = staminaDist(rng);
        creature.escape = escapeDist(rng);
        creature.hunting = 0.0f;
    }
    else
    {
        std::uniform_real_distribution<float> speedDist(1.8f, 4.2f);
        std::uniform_real_distribution<float> visionDist(60.0f, 125.0f);
        std::uniform_real_distribution<float> staminaDist(90.0f, 125.0f);
        std::uniform_real_distribution<float> huntingDist(35.0f, 75.0f);

        creature.speed = speedDist(rng);
        creature.vision = visionDist(rng);
        creature.stamina = staminaDist(rng);
        creature.hunting = huntingDist(rng);
        creature.escape = 0.0f;
    }

    std::uniform_real_distribution<float> ageDist(10.0f, 20.0f);

    creature.maxAge = ageDist(rng);
    creature.energy = std::min(100.0f, creature.stamina);

    return creature;
}

void Simulation::update(float deltaTime)
{
    if (!running || finished)
    {
        return;
    }

    deltaTime = std::min(deltaTime, 0.05f);

    generationTime += deltaTime;

    for (Creature& creature : creatures)
    {
        if (!creature.alive)
        {
            continue;
        }

        creature.age += deltaTime;

        int nearestFood = -1;
        float nearestFoodDistance = creature.vision;

        for (int i = 0; i < static_cast<int>(environment.foods.size()); i++)
        {
            float dx = environment.foods[i].position.x - creature.x;
            float dy = environment.foods[i].position.y - creature.y;
            float distance = std::sqrt(dx * dx + dy * dy);

            if (distance < nearestFoodDistance)
            {
                nearestFoodDistance = distance;
                nearestFood = i;
            }
        }

        int nearestPredator = -1;
        float nearestPredatorDistance = creature.vision;

        int nearestPrey = -1;
        float nearestPreyDistance = creature.vision;

        for (int i = 0; i < static_cast<int>(creatures.size()); i++)
        {
            const Creature& other = creatures[i];

            if (!other.alive || other.id == creature.id)
            {
                continue;
            }

            float dx = other.x - creature.x;
            float dy = other.y - creature.y;
            float distance = std::sqrt(dx * dx + dy * dy);

            if (creature.type == CreatureType::Prey &&
                other.type == CreatureType::Predator &&
                distance < nearestPredatorDistance)
            {
                nearestPredatorDistance = distance;
                nearestPredator = i;
            }

            if (creature.type == CreatureType::Predator &&
                other.type == CreatureType::Prey &&
                distance < nearestPreyDistance)
            {
                nearestPreyDistance = distance;
                nearestPrey = i;
            }
        }

        bool hungry = creature.energy < 65.0f;
        bool seesFood = nearestFood != -1;
        bool seesDanger = nearestPredator != -1;
        bool seesPrey = nearestPrey != -1;

        Action action = creature.brain.decide(
            creature.type,
            hungry,
            seesFood,
            seesDanger,
            seesPrey,
            rng,
            deltaTime
        );

        float movementSpeed = creature.speed * 60.0f;

        if (action == Action::FindFood && nearestFood != -1)
        {
            const Food& food = environment.foods[nearestFood];

            float dx = food.position.x - creature.x;
            float dy = food.position.y - creature.y;
            float distance = std::sqrt(dx * dx + dy * dy);

            if (distance > 0.01f)
            {
                creature.x +=
                    (dx / distance) * movementSpeed * deltaTime;

                creature.y +=
                    (dy / distance) * movementSpeed * deltaTime;
            }

            float newDx = food.position.x - creature.x;
            float newDy = food.position.y - creature.y;
            float newDistance = std::sqrt(
                newDx * newDx +
                newDy * newDy
            );

            if (newDistance < 9.0f)
            {
                creature.energy = std::min(
                    creature.stamina,
                    creature.energy + food.energy
                );

                creature.foodEaten++;
                stats.preyFoodEaten++;
                stats.foodConsumed++;

                environment.foods.erase(
                    environment.foods.begin() + nearestFood
                );
            }
        }
        else if (action == Action::Escape && nearestPredator != -1)
        {
            Creature& predator = creatures[nearestPredator];

            float dx = creature.x - predator.x;
            float dy = creature.y - predator.y;
            float distance = std::sqrt(dx * dx + dy * dy);

            if (distance > 0.01f)
            {
                float escapeBoost =
                    1.0f + creature.escape / 200.0f;

                creature.x +=
                    (dx / distance) *
                    movementSpeed *
                    escapeBoost *
                    deltaTime;

                creature.y +=
                    (dy / distance) *
                    movementSpeed *
                    escapeBoost *
                    deltaTime;
            }
        }
        else if (action == Action::Hunt && nearestPrey != -1)
        {
            Creature& prey = creatures[nearestPrey];

            if (!prey.alive)
            {
                continue;
            }

            float dx = prey.x - creature.x;
            float dy = prey.y - creature.y;
            float distance = std::sqrt(dx * dx + dy * dy);

            if (distance > 0.01f)
            {
                creature.x +=
                    (dx / distance) *
                    movementSpeed *
                    deltaTime;

                creature.y +=
                    (dy / distance) *
                    movementSpeed *
                    deltaTime;
            }

            float newDx = prey.x - creature.x;
            float newDy = prey.y - creature.y;
            float newDistance = std::sqrt(
                newDx * newDx +
                newDy * newDy
            );

            if (newDistance < 12.0f)
            {
                std::uniform_real_distribution<float> chance(
                    0.0f,
                    100.0f
                );

                float hunterScore =
                    creature.speed * 10.0f +
                    creature.hunting +
                    creature.energy * 0.10f;

                float escapeScore =
                    prey.speed * 10.0f +
                    prey.escape +
                    prey.energy * 0.05f;

                float successChance =
                    50.0f +
                    (hunterScore - escapeScore) * 0.5f;

                successChance = std::clamp(
                    successChance,
                    10.0f,
                    90.0f
                );

                if (chance(rng) <= successChance)
                {
                    applyDeath(
                        prey,
                        DeathCause::Predation
                    );

                    creature.kills++;

                    creature.energy = std::min(
                        creature.stamina,
                        creature.energy + 45.0f
                    );

                    stats.successfulHunts++;
                }
                else
                {
                    stats.failedHunts++;
                }
            }
        }
        else if (action == Action::Rest)
        {
            creature.energy = std::min(
                creature.stamina,
                creature.energy + 4.0f * deltaTime
            );
        }
        else
        {
            float angle = creature.brain.wanderAngle;

            creature.x +=
                std::cos(angle) *
                movementSpeed *
                0.45f *
                deltaTime;

            creature.y +=
                std::sin(angle) *
                movementSpeed *
                0.45f *
                deltaTime;
        }

        float baseCost = 1.2f;
        float speedCost = creature.speed * 0.45f;

        creature.energy -=
            (baseCost + speedCost) *
            deltaTime;

        if (action == Action::Hunt ||
            action == Action::Escape)
        {
            creature.energy -=
                1.0f *
                deltaTime;
        }

        if (creature.age >= creature.maxAge &&
            creature.alive)
        {
            applyDeath(
                creature,
                DeathCause::OldAge
            );
        }

        if (creature.energy <= 0.0f &&
            creature.alive)
        {
            applyDeath(
                creature,
                DeathCause::Starvation
            );
        }

        if (creature.alive)
        {
            creature.x = std::clamp(
                creature.x,
                5.0f,
                worldWidth - 5.0f
            );

            creature.y = std::clamp(
                creature.y,
                5.0f,
                worldHeight - 5.0f
            );
        }
    }

    removeDeadCreatures();

    createFoodIfNeeded();

    stats.peakPrey =
        std::max(
            stats.peakPrey,
            getPreyCount()
        );

    stats.peakPredators =
        std::max(
            stats.peakPredators,
            getPredatorCount()
        );

    if (generationTime >= GENERATION_LENGTH)
    {
        nextGeneration();
    }

    if (getPreyCount() == 0){
        finishExperiment();
    }
}

void Simulation::applyDeath(
    Creature& creature,
    DeathCause cause
)
{
    if (!creature.alive)
    {
        return;
    }

    creature.alive = false;
    creature.deathCause = cause;

    if (creature.type == CreatureType::Prey)
    {
        if (cause == DeathCause::Starvation)
        {
            stats.preyStarved++;
        }
        else if (cause == DeathCause::Predation)
        {
            stats.preyPredated++;
        }
        else if (cause == DeathCause::OldAge)
        {
            stats.preyOldAge++;
        }
    }
    else
    {
        if (cause == DeathCause::Starvation)
        {
            stats.predatorStarved++;
        }
        else if (cause == DeathCause::OldAge)
        {
            stats.predatorOldAge++;
        }
    }
}

void Simulation::removeDeadCreatures()
{
    creatures.erase(
        std::remove_if(
            creatures.begin(),
            creatures.end(),
            [](const Creature& creature)
            {
                return !creature.alive;
            }
        ),
        creatures.end()
    );
}

void Simulation::createFoodIfNeeded()
{
    const int minimumFood = 30;

    if (static_cast<int>(environment.foods.size()) < minimumFood)
    {
        int amount = 45;

        environment.generateFood(
            amount,
            worldWidth,
            worldHeight,
            rng
        );

        stats.foodCreated += amount;
    }
}

float Simulation::calculateFitness(
    const Creature& creature
) const
{
    return creature.energy +
           creature.foodEaten * 15.0f +
           creature.kills * 30.0f +
           creature.age * 2.0f;
}

Creature Simulation::makeChild(
    const Creature& firstParent,
    const Creature& secondParent,
    int id
)
{
    Creature child;

    child.id = id;
    child.type = firstParent.type;

    std::uniform_real_distribution<float> xDist(
        20.0f,
        worldWidth - 20.0f
    );

    std::uniform_real_distribution<float> yDist(
        20.0f,
        worldHeight - 20.0f
    );

    child.x = xDist(rng);
    child.y = yDist(rng);

    child.speed =
        (firstParent.speed + secondParent.speed) / 2.0f;

    child.vision =
        (firstParent.vision + secondParent.vision) / 2.0f;

    child.stamina =
        (firstParent.stamina + secondParent.stamina) / 2.0f;

    child.hunting =
        (firstParent.hunting + secondParent.hunting) / 2.0f;

    child.escape =
        (firstParent.escape + secondParent.escape) / 2.0f;

    std::normal_distribution<float> smallMutation(
        0.0f,
        0.08f
    );

    child.speed +=
        child.speed *
        smallMutation(rng);

    child.vision +=
        child.vision *
        smallMutation(rng);

    child.stamina +=
        child.stamina *
        smallMutation(rng);

    child.hunting +=
        child.hunting *
        smallMutation(rng);

    child.escape +=
        child.escape *
        smallMutation(rng);

    child.speed = std::clamp(
        child.speed,
        1.0f,
        6.0f
    );

    child.vision = std::clamp(
        child.vision,
        30.0f,
        160.0f
    );

    child.stamina = std::clamp(
        child.stamina,
        60.0f,
        150.0f
    );

    child.hunting = std::clamp(
        child.hunting,
        0.0f,
        100.0f
    );

    child.escape = std::clamp(
        child.escape,
        0.0f,
        100.0f
    );

    std::uniform_real_distribution<float> maxAgeDist(
        10.0f,
        20.0f
    );

    child.maxAge = maxAgeDist(rng);
    child.energy = 100.0f;

    return child;
}


void Simulation::nextGeneration()
{
    recordFinalTraitMeans();

    stats.completedGenerations++;

    // If there are no prey, the experiment is over.
    if (getPreyCount() == 0)
    {
        finishExperiment();
        return;
    }

    std::vector<Creature> preyParents;
    std::vector<Creature> predatorParents;

    for (const Creature& creature : creatures)
    {
        if (!creature.alive)
        {
            continue;
        }

        if (creature.type == CreatureType::Prey)
        {
            preyParents.push_back(creature);
        }
        else
        {
            predatorParents.push_back(creature);
        }
    }

    auto sortByFitness =
        [this](Creature& a, Creature& b)
        {
            return calculateFitness(a) >
                   calculateFitness(b);
        };

    std::sort(
        preyParents.begin(),
        preyParents.end(),
        sortByFitness
    );

    std::sort(
        predatorParents.begin(),
        predatorParents.end(),
        sortByFitness
    );

    /*
        Keep the best 40% as potential parents.
    */

    if (!preyParents.empty())
    {
        int keepCount = std::max(
            1,
            static_cast<int>(
                preyParents.size() * 0.4f
            )
        );

        preyParents.resize(keepCount);
    }

    if (!predatorParents.empty())
    {
        int keepCount = std::max(
            1,
            static_cast<int>(
                predatorParents.size() * 0.4f
            )
        );

        predatorParents.resize(keepCount);
    }

    /*
        Add offspring to the EXISTING population.
    */

    if (!preyParents.empty())
    {
        std::uniform_int_distribution<int> parentDist(
            0,
            static_cast<int>(preyParents.size()) - 1
        );

        int offspringCount = std::max(
            1,
            getPreyCount() / 4
        );

        for (int i = 0; i < offspringCount; i++)
        {
            const Creature& firstParent =
                preyParents[parentDist(rng)];

            const Creature& secondParent =
                preyParents[parentDist(rng)];

            creatures.push_back(
                makeChild(
                    firstParent,
                    secondParent,
                    nextCreatureId++
                )
            );

            stats.preyBorn++;
        }
    }

    /*
        Predators also reproduce, but more slowly.
    */

    if (!predatorParents.empty())
    {
        std::uniform_int_distribution<int> parentDist(
            0,
            static_cast<int>(predatorParents.size()) - 1
        );

        int offspringCount = std::max(
            1,
            getPredatorCount() / 5
        );

        for (int i = 0; i < offspringCount; i++)
        {
            const Creature& firstParent =
                predatorParents[parentDist(rng)];

            const Creature& secondParent =
                predatorParents[parentDist(rng)];

            creatures.push_back(
                makeChild(
                    firstParent,
                    secondParent,
                    nextCreatureId++
                )
            );

            stats.predatorBorn++;
        }
    }

    /*
        Start the next 10-second evolutionary period.
    */

    generationTime = 0.0f;
    generation++;

    /*
        IMPORTANT:
        Do NOT clear the food here.
        The ecosystem continues naturally.
    */
}


void Simulation::recordFinalTraitMeans()
{
    TraitMeans preyMeans{};
    TraitMeans predatorMeans{};

    int preyCount = 0;
    int predatorCount = 0;

    for (const Creature& creature : creatures)
    {
        if (creature.type == CreatureType::Prey)
        {
            preyMeans.speed += creature.speed;
            preyMeans.vision += creature.vision;
            preyMeans.stamina += creature.stamina;
            preyMeans.hunting += creature.hunting;
            preyMeans.escape += creature.escape;
            preyCount++;
        }
        else
        {
            predatorMeans.speed += creature.speed;
            predatorMeans.vision += creature.vision;
            predatorMeans.stamina += creature.stamina;
            predatorMeans.hunting += creature.hunting;
            predatorMeans.escape += creature.escape;
            predatorCount++;
        }
    }

    if (preyCount > 0)
    {
        preyMeans.speed /= preyCount;
        preyMeans.vision /= preyCount;
        preyMeans.stamina /= preyCount;
        preyMeans.hunting /= preyCount;
        preyMeans.escape /= preyCount;

        stats.finalPreyTraits = preyMeans;
    }

    if (predatorCount > 0)
    {
        predatorMeans.speed /= predatorCount;
        predatorMeans.vision /= predatorCount;
        predatorMeans.stamina /= predatorCount;
        predatorMeans.hunting /= predatorCount;
        predatorMeans.escape /= predatorCount;

        stats.finalPredatorTraits = predatorMeans;
    }
}

void Simulation::finishExperiment()
{
    if (finished)
    {
        return;
    }

    recordFinalTraitMeans();

    stats.finalPrey = getPreyCount();
    stats.finalPredators = getPredatorCount();

    running = false;
    finished = true;
}

void Simulation::setWorldSize(
    float width,
    float height
)
{
    worldWidth = std::max(width, 100.0f);
    worldHeight = std::max(height, 100.0f);
}

void Simulation::draw(
    float worldWidthForDrawing,
    float worldHeightForDrawing
) const
{
    if (environmentTexture.id != 0)
    {
        Rectangle source{
            0.0f,
            0.0f,
            static_cast<float>(
                environmentTexture.width
            ),
            static_cast<float>(
                environmentTexture.height
            )
        };

        Rectangle destination{
            0.0f,
            0.0f,
            worldWidthForDrawing,
            worldHeightForDrawing
        };

        DrawTexturePro(
            environmentTexture,
            source,
            destination,
            Vector2{0.0f, 0.0f},
            0.0f,
            WHITE
        );
    }
    else
    {
        DrawRectangle(
            0,
            0,
            static_cast<int>(
                worldWidthForDrawing
            ),
            static_cast<int>(
                worldHeightForDrawing
            ),
            Color{245, 248, 244, 255}
        );
    }

    environment.draw(worldWidthForDrawing, worldHeightForDrawing);

    if (foodTexture.id != 0)
    {
        Rectangle source{0.0f, 0.0f, (float)foodTexture.width, (float)foodTexture.height};
        for (const Food& food : environment.foods)
        {
            Rectangle dest{food.position.x, food.position.y, 16.0f, 16.0f};
            Vector2 origin{8.0f, 8.0f};
            DrawTexturePro(foodTexture, source, dest, origin, 0.0f, WHITE);
        }
    }

    for (const Creature& creature : creatures)
    {
        DrawCircleLines(
            static_cast<int>(creature.x),
            static_cast<int>(creature.y),
            creature.vision,
            Color{120, 140, 180, 25}
        );

        if (creature.type == CreatureType::Prey &&
            preyTexture.id != 0)
        {
            Rectangle source{
                0.0f,
                0.0f,
                static_cast<float>(
                    preyTexture.width
                ),
                static_cast<float>(
                    preyTexture.height
                )
            };

            Rectangle destination{
                creature.x,
                creature.y,
                55.0f,
                26.0f
            };

            Vector2 origin{
                27.5f,
                13.0f
            };

            DrawTexturePro(
                preyTexture,
                source,
                destination,
                origin,
                0.0f,
                WHITE
            );
        }
        else if (
            creature.type == CreatureType::Predator &&
            predatorTexture.id != 0
        )
        {
            Rectangle source{
                0.0f,
                0.0f,
                static_cast<float>(
                    predatorTexture.width
                ),
                static_cast<float>(
                    predatorTexture.height
                )
            };

            Rectangle destination{
                creature.x,
                creature.y,
                45.0f,
                35.0f
            };

            Vector2 origin{
                22.5f,
                17.5f
            };

            DrawTexturePro(
                predatorTexture,
                source,
                destination,
                origin,
                0.0f,
                WHITE
            );
        }

        float energyPercent = std::clamp(
            creature.energy /
                creature.stamina,
            0.0f,
            1.0f
        );

        float ringSize =
            creature.type == CreatureType::Prey
                ? 17.0f
                : 23.0f;

        DrawCircleLines(
            static_cast<int>(creature.x),
            static_cast<int>(creature.y),
            ringSize,
            Fade(
                GREEN,
                energyPercent
            )
        );
    }
}

bool Simulation::isRunning() const
{
    return running;
}

bool Simulation::isFinished() const
{
    return finished;
}

int Simulation::getGeneration() const
{
    return generation;
}

int Simulation::getGenerationCount() const
{
    return generation;
}

float Simulation::getGenerationProgress() const
{
    if (GENERATION_LENGTH <= 0.0f)
    {
        return 1.0f;
    }

    return std::clamp(
        generationTime /
            GENERATION_LENGTH,
        0.0f,
        1.0f
    );
}

const SimulationConfig& Simulation::getConfig() const
{
    return config;
}

const SimulationStats& Simulation::getStats() const
{
    return stats;
}

const std::vector<Creature>& Simulation::getCreatures() const
{
    return creatures;
}

int Simulation::getPreyCount() const
{
    int count = 0;

    for (const Creature& creature : creatures)
    {
        if (creature.type == CreatureType::Prey)
        {
            count++;
        }
    }

    return count;
}

int Simulation::getPredatorCount() const
{
    int count = 0;

    for (const Creature& creature : creatures)
    {
        if (creature.type == CreatureType::Predator)
        {
            count++;
        }
    }

    return count;
}

int Simulation::getFoodCount() const
{
    return static_cast<int>(
        environment.foods.size()
    );
}

const Creature* Simulation::findCreature(int id) const
{
    for (const Creature& creature : creatures)
    {
        if (creature.id == id)
        {
            return &creature;
        }
    }

    return nullptr;
}