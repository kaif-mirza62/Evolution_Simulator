#include "simulation.hpp"
#include <algorithm>
#include <cmath>

Simulation::Simulation() : randomGenerator(std::random_device{}()){

}

float Simulation::randomFloat(float minimum,float maximum){
    std::uniform_real_distribution<float> distribution(minimum,maximum);
    return distribution(randomGenerator);
}
float Simulation::distanceBetween(float firstX,float firstY,float secondX,float secondY){
    float differenceX = secondX - firstX;
    float differenceY = secondY - firstY;

    return std::sqrt(differenceX * differenceX + differenceY * differenceY);
}
void Simulation::start(int preyCount,int predatorCount,int foodCount,int generationCount){
    creatures.clear();
    environment.clearFood();

    generation = 1;
    generationTimer = 0.0f;

    maxGenerations = generationCount;
    finished = false;

    startingPrey = preyCount;
    startingPredators = predatorCount;
    startingFood = foodCount;

    preyBorn = 0;
    predatorBorn = 0;

    preyDied = 0;
    predatorDied = 0;

    preyEaten = 0;
    foodEaten = 0;

    peakPopulation = 0;
    peakPrey = 0;
    peakPredators = 0;

    generationsCompleted = 0;

    initialPreySpeed = 0;
    finalPreySpeed = 0;

    initialPredatorSpeed = 0;
    finalPredatorSpeed = 0;

    initialPreyVision = 0;
    finalPreyVision = 0;

    initialPredatorVision = 0;
    finalPredatorVision = 0;

    for(int creatureNumber = 0; creatureNumber < preyCount; creatureNumber++){
        createStartingCreature(CreatureType::Prey);
    }
    for(int creatureNumber = 0; creatureNumber < predatorCount; creatureNumber++){
        createStartingCreature(CreatureType::Predator);
    }

    environment.generateFood(foodCount,randomGenerator,WORLD_WIDTH,WORLD_HEIGHT);
    initialPreySpeed = getAverageTrait(CreatureType::Prey, true);
    initialPredatorSpeed = getAverageTrait(CreatureType::Predator, true);
    initialPreyVision = getAverageTrait(CreatureType::Prey, false);
    initialPredatorVision = getAverageTrait(CreatureType::Predator, false);

    updatePeakStatistics();

}
void Simulation::createStartingCreature(CreatureType type){
    Creature creature;
    creature.x = randomFloat(10.0f, WORLD_WIDTH - 10.0f);
    creature.y = randomFloat(10.0f, WORLD_WIDTH -10.0f);

    creature.type = type;
    creature.foodEaten = 0;

    if(type == CreatureType::Prey){
        creature.speed = randomFloat(60.0f,90.0f);
        creature.vision = randomFloat(110.0f,150.0f);
        creature.energy = PREY_START_ENERGY;
    }else{
        creature.speed = randomFloat(85.0f,110.0f);
        creature.vision = randomFloat(140.0f, 180.0f);
        creature.energy = PREDATOR_START_ENERGY;
    }
    creatures.push_back(creature);
}
void Simulation::update(float deltaTime){
    if (finished){
        return;
    }

    movePrey(deltaTime);
    movePredators(deltaTime);
    preyEatFood();
    predatorsEatPrey();
    updateEnergy(deltaTime);
    removeDeadCreatures();
    updatePeakStatistics();

    generationTimer += deltaTime;

    if (creatures.empty()){
        finished = true;
        generationsCompleted = generation;
        saveFinalAverages();
        return;
    }

    if (generationTimer >= GENERATION_LENGTH){
        finishGeneration();
    }
}
void Simulation::movePrey(float deltaTime)
{
    for (Creature& prey : creatures)
    {
        if (prey.type != CreatureType::Prey)
        {
            continue;
        }

        int closestPredatorIndex = -1;

        float closestPredatorDistance =
            prey.vision;

        for (int creatureIndex = 0;creatureIndex < static_cast<int>(creatures.size());creatureIndex++){
            if (creatures[creatureIndex].type != CreatureType::Predator){
                continue;
            }

            if (creatures[creatureIndex].energy <= 0 ){
                continue;
            }

            float predatorDistance = distanceBetween( prey.x,prey.y,creatures[creatureIndex].x,creatures[creatureIndex].y );
            if (predatorDistance < closestPredatorDistance){
                closestPredatorDistance = predatorDistance;
                closestPredatorIndex = creatureIndex;
            }
        }

        if (closestPredatorIndex != -1){
            float directionX =  prey.x - creatures[closestPredatorIndex].x;

            float directionY = prey.y - creatures[closestPredatorIndex].y;

            float directionLength = std::sqrt(directionX * directionX +  directionY * directionY  );

            if (directionLength > 0){
                directionX /= directionLength;
                directionY /= directionLength;

                prey.x += directionX * prey.speed * deltaTime;

                prey.y += directionY * prey.speed * deltaTime;
            }
        }
        else
        {
            int closestFoodIndex = -1;

            float closestFoodDistance =
                prey.vision;

            for (int foodIndex = 0; foodIndex <static_cast<int>( environment.foods.size() );foodIndex++ ){
                float foodDistance = distanceBetween( prey.x, prey.y, environment.foods[foodIndex]   .position.x, environment.foods[foodIndex]  .position.y  );

                if (  foodDistance  < closestFoodDistance  )   {
                    closestFoodDistance =
                        foodDistance;

                    closestFoodIndex =
                        foodIndex;
                }
            }

            if (closestFoodIndex != -1){
                float directionX = environment.foods[closestFoodIndex].position.x- prey.x;

                float directionY =
                    environment
                        .foods[closestFoodIndex]
                        .position.y
                    - prey.y;

                float directionLength =
                    std::sqrt(
                        directionX * directionX
                        +
                        directionY * directionY
                    );

                if (directionLength > 0)
                {
                    directionX /= directionLength;
                    directionY /= directionLength;

                    prey.x +=
                        directionX
                        * prey.speed
                        * deltaTime;

                    prey.y +=
                        directionY
                        * prey.speed
                        * deltaTime;
                }
            }
        }

        prey.x = std::clamp( prey.x,0.0f,WORLD_WIDTH );
        prey.y = std::clamp(  prey.y,0.0f, WORLD_HEIGHT);
    }
}
void Simulation::movePredators(float deltaTime)
{
    for (Creature& predator : creatures)
    {
        if (predator.type != CreatureType::Predator)
        {
            continue;
        }

        int closestPreyIndex = -1;

        float closestPreyDistance =
            predator.vision;

        for (
            int creatureIndex = 0;
            creatureIndex < static_cast<int>(creatures.size());
            creatureIndex++
        )
        {
            if (
                creatures[creatureIndex].type
                != CreatureType::Prey
            )
            {
                continue;
            }

            if (
                creatures[creatureIndex].energy <= 0
            )
            {
                continue;
            }

            float preyDistance =
                distanceBetween(
                    predator.x,
                    predator.y,
                    creatures[creatureIndex].x,
                    creatures[creatureIndex].y
                );

            if (
                preyDistance
                < closestPreyDistance
            )
            {
                closestPreyDistance =
                    preyDistance;

                closestPreyIndex =
                    creatureIndex;
            }
        }

        if (closestPreyIndex != -1)
        {
            float directionX =
                creatures[closestPreyIndex].x
                - predator.x;

            float directionY =
                creatures[closestPreyIndex].y
                - predator.y;

            float directionLength =
                std::sqrt(
                    directionX * directionX
                    +
                    directionY * directionY
                );

            if (directionLength > 0)
            {
                directionX /= directionLength;
                directionY /= directionLength;

                predator.x +=
                    directionX
                    * predator.speed
                    * deltaTime;

                predator.y +=
                    directionY
                    * predator.speed
                    * deltaTime;
            }
        }

        predator.x = std::clamp(predator.x, 0.0f, WORLD_WIDTH );
        predator.y = std::clamp( predator.y,  0.0f, WORLD_HEIGHT );
    }
}
void Simulation::preyEatFood()
{
    for (Creature& prey : creatures)
    {
        if (prey.type != CreatureType::Prey)
        {
            continue;
        }

        for (
            int foodIndex =
                static_cast<int>(
                    environment.foods.size()
                ) - 1;

            foodIndex >= 0;

            foodIndex--
        )
        {
            float foodDistance =
                distanceBetween(
                    prey.x,
                    prey.y,
                    environment.foods[foodIndex]
                        .position.x,
                    environment.foods[foodIndex]
                        .position.y
                );

            if (
                foodDistance
                <= FOOD_EAT_RANGE
            )
            {
                prey.energy +=
                    environment.foods[foodIndex]
                        .energy;

                prey.foodEaten++;

                foodEaten++;

                environment.foods.erase(
                    environment.foods.begin()
                    + foodIndex
                );

                break;
            }
        }
    }
}
void Simulation::predatorsEatPrey()
{
    for (Creature& predator : creatures)
    {
        if (
            predator.type
            != CreatureType::Predator
        )
        {
            continue;
        }

        int closestPreyIndex = -1;

        float closestPreyDistance =
            PREDATOR_ATTACK_RANGE;

        for (
            int creatureIndex = 0;
            creatureIndex < static_cast<int>(creatures.size());
            creatureIndex++
        )
        {
            if (
                creatures[creatureIndex].type
                != CreatureType::Prey
            )
            {
                continue;
            }

            if (
                creatures[creatureIndex].energy
                <= 0
            )
            {
                continue;
            }

            float preyDistance =
                distanceBetween(
                    predator.x,
                    predator.y,
                    creatures[creatureIndex].x,
                    creatures[creatureIndex].y
                );

            if (
                preyDistance
                <= closestPreyDistance
            )
            {
                closestPreyDistance =
                    preyDistance;

                closestPreyIndex =
                    creatureIndex;
            }
        }

        if (closestPreyIndex != -1)
        {
            predator.energy +=
                PREDATOR_FOOD_ENERGY;

            preyEaten++;

            creatures[closestPreyIndex].energy =
                -1;
        }
    }
}
void Simulation::updateEnergy(float deltaTime)
{
    for (Creature& creature : creatures)
    {
        if (
            creature.type
            == CreatureType::Prey
        )
        {
            creature.energy -=
                PREY_ENERGY_COST
                * deltaTime;
        }
        else
        {
            creature.energy -=
                PREDATOR_ENERGY_COST
                * deltaTime;
        }
    }
}
void Simulation::removeDeadCreatures()
{
    for (
        auto iterator = creatures.begin();
        iterator != creatures.end();
    )
    {
        if (iterator->energy <= 0)
        {
            if (
                iterator->type
                == CreatureType::Prey
            )
            {
                preyDied++;
            }
            else
            {
                predatorDied++;
            }

            iterator =
                creatures.erase(iterator);
        }
        else
        {
            iterator++;
        }
    }
}
void Simulation::finishGeneration()
{
    if (
        generation
        >= maxGenerations
    )
    {
        finished = true;

        generationsCompleted =
            generation;

        saveFinalAverages();

        return;
    }

    generationsCompleted =
        generation;

    createNextGeneration();

    generation++;

    generationTimer = 0;

    environment.clearFood();

    environment.generateFood(
        startingFood,
        randomGenerator,
        WORLD_WIDTH,
        WORLD_HEIGHT
    );

    updatePeakStatistics();
}
void Simulation::createNextGeneration()
{
    std::vector<Creature> nextGeneration;

    for (const Creature& parent : creatures)
    {
        Creature child = parent;

        child.x =
            randomFloat(
                10.0f,
                WORLD_WIDTH - 10.0f
            );

        child.y =
            randomFloat(
                10.0f,
                WORLD_HEIGHT - 10.0f
            );

        child.foodEaten = 0;

        if (
            child.type
            == CreatureType::Prey
        )
        {
            child.energy =
                PREY_START_ENERGY;

            preyBorn++;
        }
        else
        {
            child.energy =
                PREDATOR_START_ENERGY;

            predatorBorn++;
        }

        mutate(child);

        nextGeneration.push_back(child);

        if (
            parent.energy
            >= SECOND_CHILD_ENERGY
        )
        {
            Creature secondChild = parent;

            secondChild.x =
                randomFloat(
                    10.0f,
                    WORLD_WIDTH - 10.0f
                );

            secondChild.y =
                randomFloat(
                    10.0f,
                    WORLD_HEIGHT - 10.0f
                );

            secondChild.foodEaten = 0;

            if (
                secondChild.type
                == CreatureType::Prey
            )
            {
                secondChild.energy =
                    PREY_START_ENERGY;

                preyBorn++;
            }
            else
            {
                secondChild.energy =
                    PREDATOR_START_ENERGY;

                predatorBorn++;
            }

            mutate(secondChild);

            nextGeneration.push_back(
                secondChild
            );
        }
    }
    creatures = nextGeneration;
}
void Simulation::mutate(
    Creature& child
)
{
    std::normal_distribution<float>
        speedMutation(
            0.0f,
            4.0f
        );

    std::normal_distribution<float>
        visionMutation(
            0.0f,
            7.0f
        );

    child.speed +=
        speedMutation(randomGenerator);

    child.vision +=
        visionMutation(randomGenerator);

    if (child.speed < 40.0f)
    {
        child.speed = 40.0f;
    }

    if (child.speed > 140.0f)
    {
        child.speed = 140.0f;
    }

    if (child.vision < 60.0f)
    {
        child.vision = 60.0f;
    }

    if (child.vision > 220.0f)
    {
        child.vision = 220.0f;
    }
}
void Simulation::updatePeakStatistics()
{
    int preyCount = 0;

    int predatorCount = 0;

    for (const Creature& creature : creatures)
    {
        if (
            creature.type
            == CreatureType::Prey
        )
        {
            preyCount++;
        }
        else
        {
            predatorCount++;
        }
    }

    int totalPopulation =
        preyCount
        + predatorCount;

    if (
        totalPopulation
        > peakPopulation
    )
    {
        peakPopulation =
            totalPopulation;
    }

    if (preyCount > peakPrey)
    {
        peakPrey = preyCount;
    }

    if (
        predatorCount
        > peakPredators
    )
    {
        peakPredators =
            predatorCount;
    }
}
float Simulation::getAverageTrait(
    CreatureType type,
    bool getSpeed
) const
{
    float total = 0.0f;

    int count = 0;

    for (const Creature& creature : creatures)
    {
        if (creature.type != type)
        {
            continue;
        }

        if (getSpeed)
        {
            total += creature.speed;
        }
        else
        {
            total += creature.vision;
        }

        count++;
    }

    if (count == 0)
    {
        return 0.0f;
    }

    return total / count;
}
void Simulation::saveFinalAverages()
{
    finalPreySpeed =
        getAverageTrait(
            CreatureType::Prey,
            true
        );

    finalPredatorSpeed =
        getAverageTrait(
            CreatureType::Predator,
            true
        );

    finalPreyVision =
        getAverageTrait(
            CreatureType::Prey,
            false
        );

    finalPredatorVision =
        getAverageTrait(
            CreatureType::Predator,
            false
        );
}
void Simulation::draw() const
{
    environment.draw();

    for (const Creature& creature : creatures)
    {
        if (
            creature.type
            == CreatureType::Prey
        )
        {
            DrawCircle(
                static_cast<int>(creature.x),
                static_cast<int>(creature.y),
                5.0f,
                BLACK
            );
        }
        else
        {
            DrawCircle(
                static_cast<int>(creature.x),
                static_cast<int>(creature.y),
                7.0f,
                RED
            );
        }
    }
}
