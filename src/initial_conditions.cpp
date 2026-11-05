#include <nbody/initial_conditions.hpp>
#include <vector>
#include <glm/glm.hpp>
#include <cstdint>
#include <random>
#include <array>
#include <optional>
#include <system_error>
#include <cmath>

static constexpr std::array<std::pair<std::string_view, InitMethod>, 6> kInitNames{{
    {"uniform",   InitMethod::Uniform},
    {"plummer",   InitMethod::Plummer},
    {"disk",      InitMethod::Disk},
    {"collision", InitMethod::Collision},
    {"collapse", InitMethod::Collapse},
    {"solar", InitMethod::SolarSystem}
}};

std::optional<InitMethod> parseInitMethod(std::string_view name) {
    for (auto [n, m] : kInitNames)
        if (n == name) return m;
    return std::nullopt;
}

InitParams InitParams::parse(std::string_view s) {
    InitParams out;
    // s expected par1=val1,par2=val2
    while (!s.empty()) {
        auto comma = s.find(',');
        auto tok = s.substr(0, comma);
        auto eq = tok.find('=');
        if (eq == tok.npos) throw std::invalid_argument("expected key=value, got: " + std::string(tok));

        out.m_vals[std::string(tok.substr(0, eq))] = std::stof(std::string(tok.substr(eq+1)));
        s = comma == s.npos ? std::string_view{} : s.substr(comma + 1);
    }
    return out;
}

float InitParams::get(const std::string& key, float def) const {
    if (m_vals.find(key) != m_vals.end()) {
        m_used.insert(key);
        return m_vals.at(key);
    } else {
        return def;
    }
}

void InitParams::assertAllUsed() const {
    for (auto& [k, _] : m_vals) {
        if (!m_used.count(k)) throw std::invalid_argument("unknown init parameter: " + k);
    }
}

std::pair<float, float> randomDirection(std::default_random_engine &gen) {
    std::uniform_real_distribution<float> uniform(0.0,1.0);
    float u1 = uniform(gen);
    float u2 = uniform(gen);
    float theta = acos(2*u1 - 1);
    float phi = 2*M_PI*u2;

    return {theta, phi};
}



Bodies makeUniform(size_t n, std::uint32_t seed, const InitParams& p) {
    std::default_random_engine gen(seed);
    std::uniform_real_distribution<float> uniform(-1.0,1.0);

    float mMax = p.get("m_max", 0.0);

    Bodies bodies = {std::vector<glm::vec4>(n), std::vector<glm::vec3>(n)};

    for (size_t i = 0; i < n; ++i)
    {
        auto& p = bodies.pos[i];
        auto& v = bodies.vel[i];
        if (!mMax) {
            p.w = 1.0;
        } else {
            p.w = (uniform(gen) + 1.1) / 2.0f * mMax;
        }
        p.x = uniform(gen); p.y = uniform(gen); p.z = uniform(gen); 
        v.x = uniform(gen); v.y = uniform(gen); v.z = uniform(gen);
        v *= 5; // for funsies remove later
    }
    recenter(bodies);
    p.assertAllUsed();
    return bodies;
}

Bodies makeCollapse(size_t n, std::uint32_t seed, const InitParams& p) {
    std::default_random_engine gen(seed);
    std::uniform_real_distribution<float> uniform(-1.0,1.0);

    Bodies bodies = {std::vector<glm::vec4>(n), std::vector<glm::vec3>(n)};
    float mMax = p.get("m_max", 0.0);
    for (size_t i = 0; i < n; ++i)
    {
        auto& p = bodies.pos[i];
        auto& v = bodies.vel[i];
        
        if (!mMax) {
            p.w = 1.0;
        } else {
            p.w = (uniform(gen) + 1.1) / 2.0f * mMax;
        }
        p.x = uniform(gen); p.y = uniform(gen); p.z = uniform(gen); 
        v.x = 0; v.y = 0; v.z = 0;
    }
    recenter(bodies);
    p.assertAllUsed();
    return bodies;
}

Bodies makeDisk(size_t n, std::uint32_t seed, const InitParams& p) {
    throw std::logic_error("Functionality not implemented yet.");
    return {};
}

Bodies makeCollision(size_t n, std::uint32_t seed, const InitParams& p) {
    throw std::logic_error("Functionality not implemented yet.");
    return {};
}

Bodies makePlummer(size_t n, std::uint32_t seed, const InitParams& p) {
    // vel: https://physics.stackexchange.com/questions/94845/velocity-distribution-in-plummers-models-and-others-mass-distributions
    // rad: wikipedia inverting the enclosed mass formula

    std::default_random_engine gen(seed);
    std::uniform_real_distribution<float> uniform(0.0,1.0);
    float scaleRadius = p.get("scale_radius", 5.0);
    float mMax = p.get("m_max", 0.0);

    Bodies bodies = {std::vector<glm::vec4>(n), std::vector<glm::vec3>(n)};


    for (size_t i = 0; i < n; ++i)
    {
        float radius = scaleRadius / sqrtf(powf(uniform(gen), -2.0f/3.0f) - 1);
        // needs updating if we ever have differing mass per bodies in this one.
        float escVelocity = sqrtf(2.0) * powf(scaleRadius * scaleRadius + radius * radius, -0.25);
        
        auto&& [rTheta, rPhi] = randomDirection(gen);
        auto&& [vTheta, vPhi] = randomDirection(gen);
        
        float m;
        if (!mMax) {
            m= 1.0;
        } else {
            m = (uniform(gen) + 0.1) * mMax;
        }

        glm::vec4 pos = {
            radius * sin(rTheta) * cos(rPhi), // x
            radius * cos(rTheta), // y (vertical)
            radius * sin(rTheta) * sin(rPhi), // z
            m // mass
        };

        // Compute q using rejection sampling
        float gMax = 0.1f; // safe upper bound
        float q = -1.0f;
        while(1) {
            float qCand = uniform(gen);
            float yTest = uniform(gen) * gMax;

            float gQ = (qCand * qCand) * powf((1.0f - (qCand * qCand)), 3.5);

            if (yTest <= gQ) {
                q = qCand;
                break;
            }
        }
        

        glm::vec3 vel = {
            escVelocity * q * sin(vTheta) * cos(vPhi), // x
            escVelocity * q * cos(vTheta), // y (vertical)
            escVelocity * q * sin(vTheta) * sin(vPhi), // z
        };

        bodies.pos[i] = pos;
        bodies.vel[i] = vel;
    }
    
    recenter(bodies);
    p.assertAllUsed();
    return bodies;
}

Bodies makeSolarSystem(size_t n, std::uint32_t seed, const InitParams& p) {

    std::default_random_engine gen(seed);
    std::uniform_real_distribution<float> uniform(0.0,1.0);

    float massMult = p.get("mass_mult", 1000);
    float massPlanetMax = p.get("planet_max", 1);
    float maxRadius = p.get("max_radius", 1.0);
    float maxHeight = p.get("max_height", 0.05);

    Bodies bodies = {std::vector<glm::vec4>(n), std::vector<glm::vec3>(n)};

    // set up star:
    float starMass = massPlanetMax * massMult;
    bodies.pos[0] = {0, 0, 0, massPlanetMax * massMult};
    bodies.vel[0] = {0, 0, 0};

    // place planets first so the total mass is known before assigning velocities
    float totalMass = starMass;
    std::vector<float> radii(n, 0.0f), angles(n, 0.0f);
    for (size_t i = 1; i < n; ++i)
    {
        radii[i] = uniform(gen) * maxRadius + 0.2;
        angles[i] = 2 * M_PI * uniform(gen);

        bodies.pos[i] = {
            radii[i] * sin(angles[i]),
            (uniform(gen) - 0.5f) * maxHeight,
            radii[i] * cos(angles[i]),
            uniform(gen) * massPlanetMax
        };
        totalMass += bodies.pos[i].w;
    }

    // recenter() normalizes the total mass to 1 and the kernel uses G = 1,
    // so the star's effective mass is starMass / totalMass.
    float starMassEff = starMass / totalMass;
    for (size_t i = 1; i < n; ++i)
    {
        float speed = sqrt(starMassEff / radii[i]); // circular orbit speed
        bodies.vel[i] = {
            speed * cos(angles[i]),  // tangent to the orbit, no radial component
            0.0f,
            -speed * sin(angles[i])
        };
    }

    recenter(bodies);
    p.assertAllUsed();
    return bodies;
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
        p.w /= m; // normalize mass
        v -= comVel;
    }
}

Bodies makeBodies(InitMethod method, size_t n, std::uint32_t seed,
                  const InitParams& params) {
    switch (method)
    {
    case InitMethod::Uniform:   return makeUniform(n, seed, params);
    case InitMethod::Collapse:   return makeCollapse(n, seed, params);
    case InitMethod::Plummer:   return makePlummer(n, seed, params);
    case InitMethod::Disk:      return makeDisk(n, seed, params);
    case InitMethod::Collision: return makeCollision(n, seed, params);
    case InitMethod::SolarSystem: return makeSolarSystem(n, seed, params);
    }
    return {}; 
}