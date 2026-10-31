#pragma once
#include <vector>
#include <cstddef>

#define DIM 3

class Simulator {
public:
    Simulator(int n);
    ~Simulator();

    void step(float dt);
    const float* positions();
    const std::size_t count();


private:
    std::vector<float> m_pos;
    std::size_t m_n;
};