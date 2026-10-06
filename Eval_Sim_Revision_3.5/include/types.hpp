#pragma once

enum class CreatureType{
    Prey,
    Predator
};

enum class Action{
    Wander,
    FindFood,
    Escape,
    Hunt,
    Rest
};

enum class DeathCause{
    None,
    Starvation,
    Predation,
    OldAge
};
