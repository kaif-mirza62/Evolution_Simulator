#include "rl_agent.hpp"
#include <array>
#include <limits>
#include <vector>

RLAgent::RLAgent(
    CreatureType typeValue,
    std::mt19937& rngValue
)
    : type(typeValue),
      rng(rngValue),
      onlineNetwork(rng),
      targetNetwork(rng),
      replayBuffer(BUFFER_CAPACITY)
{
    targetNetwork.copyFrom(onlineNetwork);
}

bool RLAgent::isValidAction(
    Action action
) const
{
    if ( type == CreatureType::Prey)
    {
        return
            action == Action::Wander ||
            action == Action::FindFood ||
            action == Action::Escape;
    }

    return
        action == Action::Wander ||
        action == Action::Hunt ||
        action == Action::Rest;
}

Action RLAgent::randomValidAction()
{
    std::uniform_int_distribution<int>
        distribution(0, 2);

    if (
        type == CreatureType::Prey
    )
    {
        const std::array<Action, 3> actions =
        {
            Action::Wander,
            Action::FindFood,
            Action::Escape
        };

        return actions[
            distribution(rng)
        ];
    }

    const std::array<Action, 3> actions =
    {
        Action::Wander,
        Action::Hunt,
        Action::Rest
    };

    return actions[
        distribution(rng)
    ];
}

int RLAgent::bestValidAction(
    const NeuralNetwork::Output& qValues,
    Action instinct
) const
{
    float bestScore =
        -std::numeric_limits<float>::infinity();

    int bestAction = 0;

    for (
        int i = 0;
        i < static_cast<int>(
                NeuralNetwork::OUTPUTS
            );
        ++i
    )
    {
        Action action =
            static_cast<Action>(i);

        if (
            !isValidAction(action)
        )
        {
            continue;
        }

        float score =
            qValues[i];

        if (
            action == instinct
        )
        {
            score +=
                INSTINCT_BIAS;
        }

        if (
            score > bestScore
        )
        {
            bestScore = score;

            bestAction = i;
        }
    }

    return bestAction;
}

Action RLAgent::chooseAction(
    const LearningState& state,
    Action instinct
)
{
    std::uniform_real_distribution<float>
        randomValue(0.0f, 1.0f);

    if (replayBuffer.size() < WARMUP_EXPERIENCES)
    {
        if (
            randomValue(rng) < 0.15f
        )
        {
            return randomValidAction();
        }

        if (
            isValidAction(instinct)
        )
        {
            return instinct;
        }

        return randomValidAction();
    }

    if (
        randomValue(rng) < epsilon
    )
    {
        return randomValidAction();
    }

    NeuralNetwork::Output qValues =
        onlineNetwork.predict(
            state
        );

    return static_cast<Action>(
        bestValidAction(
            qValues,
            instinct
        )
    );
}

void RLAgent::remember(
    const Experience& experience
)
{
    replayBuffer.add(
        experience
    );
}

float RLAgent::train()
{
    if (
        replayBuffer.size()
        < WARMUP_EXPERIENCES
    )
    {
        return lastLoss;
    }

    std::vector<Experience> experiences =
        replayBuffer.sample(
            BATCH_SIZE,
            rng
        );

    if (
        experiences.empty()
    )
    {
        return lastLoss;
    }

    std::vector<
        NeuralNetwork::TrainingExample
    > trainingBatch;

    trainingBatch.reserve(
        experiences.size()
    );

    for (
        const Experience& experience
        : experiences
    )
    {
        NeuralNetwork::Output
            currentQ =
                onlineNetwork.predict(
                    experience.state
                );

        NeuralNetwork::Output
            nextQ =
                targetNetwork.predict(
                    experience.nextState
                );

        float maxNextQ =
            -std::numeric_limits<float>::infinity();

        for (
            int i = 0;
            i < static_cast<int>(
                    NeuralNetwork::OUTPUTS
                );
            ++i
        )
        {
            Action action =
                static_cast<Action>(i);

            if (
                !isValidAction(action)
            )
            {
                continue;
            }

            if (
                nextQ[i] > maxNextQ
            )
            {
                maxNextQ =
                    nextQ[i];
            }
        }

        float target =
            experience.reward;

        if (
            !experience.done
        )
        {
            target +=
                GAMMA * maxNextQ;
        }

        int actionIndex =
            static_cast<int>(
                experience.action
            );

        if (
            actionIndex >= 0 &&
            actionIndex <
                static_cast<int>(
                    NeuralNetwork::OUTPUTS
                )
        )
        {
            currentQ[
                actionIndex
            ] = target;
        }

        trainingBatch.emplace_back(
            experience.state,
            currentQ
        );
    }

    lastLoss =
        onlineNetwork.trainBatch(
            trainingBatch,
            LEARNING_RATE
        );

    ++trainingSteps;

    float progress =
        static_cast<float>(
            trainingSteps
        )
        /
        static_cast<float>(
            EPSILON_DECAY_STEPS
        );

    epsilon =
        std::max(
            MIN_EPSILON,
            INITIAL_EPSILON -
            (
                INITIAL_EPSILON -
                MIN_EPSILON
            ) * progress
        );

    if (
        trainingSteps
        % TARGET_UPDATE_STEPS
        == 0
    )
    {
        targetNetwork.copyFrom(
            onlineNetwork
        );
    }

    return lastLoss;
}

void RLAgent::reset()
{
    onlineNetwork =
        NeuralNetwork(rng);

    targetNetwork.copyFrom(
        onlineNetwork
    );

    replayBuffer.clear();

    epsilon = INITIAL_EPSILON;

    trainingSteps = 0;

    lastLoss = 0.0f;
}

float RLAgent::getEpsilon() const
{
    return epsilon;
}

std::size_t RLAgent::getMemorySize() const
{
    return replayBuffer.size();
}

std::size_t RLAgent::getTrainingSteps() const
{
    return trainingSteps;
}

float RLAgent::getLastLoss() const
{
    return lastLoss;
}