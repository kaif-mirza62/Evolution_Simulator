#include "brain.hpp"
#include <cmath>

Action Brain::decide(
    CreatureType type,
    bool hungry,
    bool seesFood,
    bool seesDanger,
    bool seesPrey,
    std::mt19937& rng,
    float deltaTime
){
    decisionTimer -= deltaTime;

    if (decisionTimer <= 0.0f){
        std::uniform_real_distribution<float> angleChange(-1.2f, 1.2f);

        wanderAngle += angleChange(rng);
        decisionTimer = 0.8f;
    }

    if (type == CreatureType::Prey){

        if (seesDanger){
            currentAction = Action::Escape;
            return currentAction;
        }

        if (hungry && seesFood){
            currentAction = Action::FindFood;
            return currentAction;
        }

        currentAction = Action::Wander;
        return currentAction;
    }

    if (type == CreatureType::Predator){
        if (seesPrey){
            currentAction = Action::Hunt;
            return currentAction;
        }

        if (!hungry){
            currentAction = Action::Rest;
            return currentAction;
        }

        currentAction = Action::Wander;
        return currentAction;
    }

    currentAction = Action::Wander;
    return currentAction;
}
