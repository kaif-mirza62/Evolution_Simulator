#include "replay_buffer.hpp"

#include <algorithm>
#include <stdexcept>

ReplayBuffer::ReplayBuffer(
    std::size_t capacityValue
)
    : capacity(capacityValue)
{
    if (capacity == 0)
    {
        throw std::invalid_argument(
            "ReplayBuffer capacity must be greater than zero"
        );
    }
}

void ReplayBuffer::add(
    const Experience& experience
)
{
    if (buffer.size() >= capacity)
    {
        buffer.pop_front();
    }

    buffer.push_back(experience);
}

std::vector<Experience>
ReplayBuffer::sample(
    std::size_t batchSize,
    std::mt19937& rng
) const
{
    std::size_t count =
        std::min(
            batchSize,
            buffer.size()
        );

    std::vector<Experience> result;

    result.reserve(count);

    if (count == 0)
    {
        return result;
    }

    std::uniform_int_distribution<std::size_t> distribution(
        0,
        buffer.size() - 1
    );

    for (std::size_t i = 0;i < count;i++)
    {
        result.push_back(
            buffer[
                distribution(rng)
            ]
        );
    }

    return result;
}

std::size_t ReplayBuffer::size() const{
    return buffer.size();
}

void ReplayBuffer::clear(){
    buffer.clear();
}