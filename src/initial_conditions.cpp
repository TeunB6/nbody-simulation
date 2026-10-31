#include <nbody/initial_conditions.hpp>
#include <vector>
#include <glm/glm.hpp>
#include <cstdint>
#include <random>
#include <array>
#include <optional>
#include <system_error>

static constexpr std::array<std::pair<std::string_view, InitMethod>, 5> kInitNames{{
    {"uniform",   InitMethod::Uniform},
    {"plummer",   InitMethod::Plummer},
    {"disk",      InitMethod::Disk},
    {"collision", InitMethod::Collision},
    {"collapse", InitMethod::Collapse}
}};

std::optional<InitMethod> parseInitMethod(std::string_view name) {
    for (auto [n, m] : kInitNames)
        if (n == name) return m;
    return std::nullopt;
}

Bodies makeUniform(size_t n, std::uint32_t seed) {
    std::default_random_engine gen(seed);
    std::uniform_real_distribution<float> uniform(-1.0,1.0);

    Bodies bodies = {std::vector<glm::vec4>(n), std::vector<glm::vec3>(n)};

    for (size_t i = 0; i < n; ++i)
    {
        auto& p = bodies.pos[i];
        auto& v = bodies.vel[i];
        
        p.w = 1.0;
        p.x = uniform(gen); p.y = uniform(gen); p.z = uniform(gen); 
        v.x = uniform(gen); v.y = uniform(gen); v.z = uniform(gen);
    }
    recenter(bodies);
    return bodies;
}

Bodies makeCollapse(size_t n, std::uint32_t seed) {
    std::default_random_engine gen(seed);
    std::uniform_real_distribution<float> uniform(-1.0,1.0);

    Bodies bodies = {std::vector<glm::vec4>(n), std::vector<glm::vec3>(n)};

    for (size_t i = 0; i < n; ++i)
    {
        auto& p = bodies.pos[i];
        auto& v = bodies.vel[i];
        
        p.w = 1.0;
        p.x = uniform(gen); p.y = uniform(gen); p.z = uniform(gen); 
        v.x = 0; v.y = 0; v.z = 0;
    }
    recenter(bodies);
    return bodies;
}

Bodies makeDisk(size_t n, std::uint32_t seed, float central_mass) {
    throw std::logic_error("Functionality not implemented yet.");
    return {};
}

Bodies makeCollision(size_t n, std::uint32_t seed) {
    throw std::logic_error("Functionality not implemented yet.");
    return {};
}

Bodies makePlummer(size_t, std::uint32_t seed) {
    throw std::logic_error("Functionality not implemented yet.");
    return {};
}

void recenter(Bodies& b) {
    // compute v and p avg
    glm::vec3 comPos = {0.0, 0.0, 0.0};
    glm::vec3 comVel = {0.0, 0.0, 0.0};
    float m = 0;

    for (size_t i = 0; i < b.pos.size(); i++)
    {
        auto& p = b.pos[i];
        auto& v = b.vel[i];

        comPos += glm::vec3({p.x, p.y, p.z}) * p.w;
        comVel += glm::vec3({v.x, v.y, v.z}) * p.w;

        m += p.w;
    }
    
    comPos /= m; comVel /= m;

    for (size_t i = 0; i < b.pos.size(); i++)
    {
        auto& p = b.pos[i];
        auto& v = b.vel[i];

        p.x -= comPos.x; p.y -= comPos.y; p.z -= comPos.z;
        v -= comVel;
    }
}

Bodies makeBodies(InitMethod method, size_t n, std::uint32_t seed) {
    switch (method)
    {
    case InitMethod::Uniform:   return makeUniform(n, seed);
    case InitMethod::Collapse:   return makeCollapse(n, seed);
    case InitMethod::Plummer:   return makePlummer(n, seed);
    case InitMethod::Disk:      return makeDisk(n, seed, 0.1);
    case InitMethod::Collision: return makeCollision(n, seed);
    }
    return {}; 
}