#pragma once

#include "types.hpp"
#include <cstddef>
#include <deque>
#include <random>
#include <vector>

class ReplayBuffer
{
public:
    explicit ReplayBuffer(std::size_t capacityValue);

    void add(const Experience& experience);

    std::vector<Experience> sample(
        std::size_t batchSize,
        std::mt19937& rng
    ) const;

    std::size_t size() const;

    void clear();

private:
    std::deque<Experience> buffer;
    std::size_t capacity;
};
