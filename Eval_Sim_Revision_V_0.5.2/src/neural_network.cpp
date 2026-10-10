#include "neural_network.hpp"
#include <algorithm>
#include <cmath>

namespace
{

float relu(float x)
{
    return x > 0.0f ? x : 0.0f;
}

float reluDerivative(float x)
{
    return x > 0.0f ? 1.0f : 0.0f;
}

float huberGradient(float error)
{
    return std::clamp(
        error,
        -1.0f,
        1.0f
    );
}

}

NeuralNetwork::NeuralNetwork( std::mt19937& rng )
{
    std::normal_distribution<float> firstLayerDistribution(
        0.0f,
        std::sqrt(
            2.0f /
            static_cast<float>(
                LEARNING_STATE_SIZE
            )
        )
    );

    for (float& weight : w1)
    {
        weight = firstLayerDistribution(rng);
    }

    std::normal_distribution<float> secondLayerDistribution(
        0.0f,
        std::sqrt(
            2.0f /
            static_cast<float>(HIDDEN_1)
        )
    );

    for (float& weight : w2)
    {
        weight =
            secondLayerDistribution(rng);
    }

    std::normal_distribution<float> outputLayerDistribution(
        0.0f,
        std::sqrt(
            2.0f /
            static_cast<float>(HIDDEN_2)
        )
    );

    for (float& weight : w3)
    {
        weight =
            outputLayerDistribution(rng);
    }
}

NeuralNetwork::Output
NeuralNetwork::predict(
    const LearningState& state
) const
{
    std::array<float, HIDDEN_1> z1{};
    std::array<float, HIDDEN_1> a1{};

    std::array<float, HIDDEN_2> z2{};
    std::array<float, HIDDEN_2> a2{};

    Output output{};

    for (
        std::size_t h = 0;
        h < HIDDEN_1;
        ++h
    )
    {
        float sum = b1[h];

        for (
            std::size_t i = 0;
            i < LEARNING_STATE_SIZE;
            ++i
        )
        {
            sum +=  w1[ h * LEARNING_STATE_SIZE + i ] * state[i];
        }

        z1[h] = sum;

        a1[h] =
            relu(sum);
    }

    for (
        std::size_t h = 0;
        h < HIDDEN_2;
        ++h
    )
    {
        float sum = b2[h];

        for (
            std::size_t i = 0;
            i < HIDDEN_1;
            ++i
        )
        {
            sum +=
                w2[
                    h * HIDDEN_1 + i
                ]
                *
                a1[i];
        }

        z2[h] = sum;

        a2[h] =
            relu(sum);
    }

    for (
        std::size_t o = 0;
        o < OUTPUTS;
        ++o
    )
    {
        float sum = b3[o];

        for (
            std::size_t h = 0;
            h < HIDDEN_2;
            ++h
        )
        {
            sum +=
                w3[
                    o * HIDDEN_2 + h
                ]
                *
                a2[h];
        }

        output[o] = sum;
    }

    return output;
}

float NeuralNetwork::trainBatch(
    const std::vector<TrainingExample>& batch,
    float learningRate
)
{
    if (batch.empty())
    {
        return 0.0f;
    }

    std::array<
        float,
        LEARNING_STATE_SIZE * HIDDEN_1
    > gradW1{};

    std::array<float, HIDDEN_1> gradB1{};

    std::array<
        float,
        HIDDEN_1 * HIDDEN_2
    > gradW2{};

    std::array<float, HIDDEN_2> gradB2{};

    std::array<
        float,
        HIDDEN_2 * OUTPUTS
    > gradW3{};

    std::array<float, OUTPUTS> gradB3{};

    float totalLoss = 0.0f;

    for (
        const TrainingExample& example
        : batch
    )
    {
        const LearningState& state =
            example.first;

        const Output& target =
            example.second;

        std::array<float, HIDDEN_1> z1{};
        std::array<float, HIDDEN_1> a1{};

        std::array<float, HIDDEN_2> z2{};
        std::array<float, HIDDEN_2> a2{};

        Output output{};

        for (
            std::size_t h = 0;
            h < HIDDEN_1;
            ++h
        )
        {
            float sum = b1[h];

            for (
                std::size_t i = 0;
                i < LEARNING_STATE_SIZE;
                ++i
            )
            {
                sum +=
                    w1[
                        h * LEARNING_STATE_SIZE + i
                    ]
                    *
                    state[i];
            }

            z1[h] = sum;

            a1[h] =
                relu(sum);
        }

        for (
            std::size_t h = 0;
            h < HIDDEN_2;
            ++h
        )
        {
            float sum = b2[h];

            for (
                std::size_t i = 0;
                i < HIDDEN_1;
                ++i
            )
            {
                sum +=
                    w2[
                        h * HIDDEN_1 + i
                    ]
                    *
                    a1[i];
            }

            z2[h] = sum;

            a2[h] =
                relu(sum);
        }

        for (
            std::size_t o = 0;
            o < OUTPUTS;
            ++o
        )
        {
            float sum = b3[o];

            for (
                std::size_t h = 0;
                h < HIDDEN_2;
                ++h
            )
            {
                sum +=
                    w3[
                        o * HIDDEN_2 + h
                    ]
                    *
                    a2[h];
            }

            output[o] = sum;
        }

        std::array<float, OUTPUTS> d3{};

        for (
            std::size_t o = 0;
            o < OUTPUTS;
            ++o
        )
        {
            float error =
                output[o] -
                target[o];

            float absoluteError =
                std::fabs(error);

            if (
                absoluteError <= 1.0f
            )
            {
                totalLoss +=
                    0.5f *
                    error *
                    error;
            }
            else
            {
                totalLoss +=
                    absoluteError -
                    0.5f;
            }

            d3[o] =
                huberGradient(error);
        }

        std::array<float, HIDDEN_2> d2{};

        for (
            std::size_t h = 0;
            h < HIDDEN_2;
            ++h
        )
        {
            float sum = 0.0f;

            for (
                std::size_t o = 0;
                o < OUTPUTS;
                ++o
            )
            {
                sum +=
                    w3[
                        o * HIDDEN_2 + h
                    ]
                    *
                    d3[o];
            }

            d2[h] =
                sum *
                reluDerivative(
                    z2[h]
                );
        }

        std::array<float, HIDDEN_1> d1{};

        for (
            std::size_t h = 0;
            h < HIDDEN_1;
            ++h
        )
        {
            float sum = 0.0f;

            for (
                std::size_t h2 = 0;
                h2 < HIDDEN_2;
                ++h2
            )
            {
                sum +=
                    w2[
                        h2 * HIDDEN_1 + h
                    ]
                    *
                    d2[h2];
            }

            d1[h] =
                sum *
                reluDerivative(
                    z1[h]
                );
        }

        for (
            std::size_t o = 0;
            o < OUTPUTS;
            ++o
        )
        {
            gradB3[o] +=
                d3[o];

            for (
                std::size_t h = 0;
                h < HIDDEN_2;
                ++h
            )
            {
                gradW3[
                    o * HIDDEN_2 + h
                ] +=
                    d3[o] *
                    a2[h];
            }
        }

        for (
            std::size_t h = 0;
            h < HIDDEN_2;
            ++h
        )
        {
            gradB2[h] +=
                d2[h];

            for (
                std::size_t i = 0;
                i < HIDDEN_1;
                ++i
            )
            {
                gradW2[
                    h * HIDDEN_1 + i
                ] +=
                    d2[h] *
                    a1[i];
            }
        }

        for (
            std::size_t h = 0;
            h < HIDDEN_1;
            ++h
        )
        {
            gradB1[h] +=
                d1[h];

            for (
                std::size_t i = 0;
                i < LEARNING_STATE_SIZE;
                ++i
            )
            {
                gradW1[
                    h * LEARNING_STATE_SIZE + i
                ] +=
                    d1[h] *
                    state[i];
            }
        }
    }

    float scale =
        1.0f /
        static_cast<float>(
            batch.size()
        );

    auto updateParameter =
        [learningRate, scale]
        (
            float& parameter,
            float gradient
        )
        {
            gradient *= scale;

            gradient =
                std::clamp(
                    gradient,
                    -5.0f,
                    5.0f
                );

            parameter -=
                learningRate *
                gradient;
        };

    for (
        std::size_t i = 0;
        i < w1.size();
        ++i
    )
    {
        updateParameter(
            w1[i],
            gradW1[i]
        );
    }

    for (
        std::size_t i = 0;
        i < b1.size();
        ++i
    )
    {
        updateParameter(
            b1[i],
            gradB1[i]
        );
    }

    for (
        std::size_t i = 0;
        i < w2.size();
        ++i
    )
    {
        updateParameter(
            w2[i],
            gradW2[i]
        );
    }

    for (
        std::size_t i = 0;
        i < b2.size();
        ++i
    )
    {
        updateParameter(
            b2[i],
            gradB2[i]
        );
    }

    for (
        std::size_t i = 0;
        i < w3.size();
        ++i
    )
    {
        updateParameter(
            w3[i],
            gradW3[i]
        );
    }

    for (
        std::size_t i = 0;
        i < b3.size();
        ++i
    )
    {
        updateParameter(
            b3[i],
            gradB3[i]
        );
    }

    return
        totalLoss /
        static_cast<float>(
            batch.size()
        );
}

void NeuralNetwork::copyFrom(
    const NeuralNetwork& other
)
{
    *this = other;
}