#pragma once
#include <glm/glm.hpp>
#include <nbody/bodies.hpp>
#include <vector>
#include <cstdint>
#include <utility>
#include <optional>
#include <string_view>
#include <map>
#include <set>


enum class InitMethod {Uniform, Plummer, Disk, Collision, Collapse, SolarSystem};
std::optional<InitMethod> parseInitMethod(std::string_view name);

class InitParams {
    public:
        static InitParams parse(std::string_view s);
        float get(const std::string& key, float def) const;
        void assertAllUsed() const;
        void setDefault(const std::string& key, float val); 

    private:
        std::map<std::string, float> m_vals;
        mutable std::set<std::string> m_used;
};


Bodies  makeBodies(InitMethod method, size_t n, std::uint32_t seed,
                  const InitParams& params);

Bodies makeUniform(size_t n, std::uint32_t seed, const InitParams& p);
Bodies makePlummer(size_t n, std::uint32_t seed, const InitParams& p);
Bodies makeSolarSystem(size_t n, std::uint32_t seed, const InitParams& p);
Bodies makeDisk(size_t n, std::uint32_t seed, const InitParams& p);
Bodies makeCollision(size_t n, std::uint32_t seed, const InitParams& p);
Bodies makeCollapse(size_t n, std::uint32_t seed, const InitParams& p);


void recenter(Bodies& b);   // zero total momentum and centre of mass
