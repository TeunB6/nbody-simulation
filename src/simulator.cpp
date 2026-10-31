#include <nbody/simulator.hpp>
#include <random>

Simulator::Simulator(int n): m_n(n), m_pos(n * DIM) {
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> d(-1.f, 1.f);
    for (auto& v : m_pos)
        v = d(rng);
    
    for (std::size_t i = 2; i < m_pos.size(); i += DIM)
        m_pos[i] = 0.f; // flat for now    
}

const float* Simulator::positions() {
    return m_pos.data();
}
 
const std::size_t Simulator::count() {
    return m_pos.size() / DIM;
}
void Simulator::step(float dt) {
    // drift for now
    for (size_t i = 0; i < m_pos.size(); i += DIM)
    {
        m_pos[i] += 0.1f * dt;
        if (m_pos[i] >= 1)
        {
            m_pos[i] = -1.f;
        }
    }
    
}


Simulator::~Simulator() {

}