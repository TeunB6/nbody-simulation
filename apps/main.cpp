#include <iostream>
#include <iterator>
#include <vector>
#include <glm/glm.hpp>
#include <nbody/renderer.hpp>
#include <nbody/simulator.hpp>
#include <nbody/initial_conditions.hpp>
#include <nbody/bodies.hpp>

int main(int argc, char** argv) {
    int n = 1000;
    float soft = 0.5;
    bool triangleTest = false;
    std::uint32_t seed = 33;
    InitMethod init = InitMethod::Uniform;
    InitParams params = InitParams{};


    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-N" && i + 1 < argc) {
            n = std::stoi(argv[++i]); // read next token
        } else if (arg == "-test") {
            triangleTest = true;
        } else if (arg == "-soft" && i + 1 < argc) {
            soft = std::stof(argv[++i]);  
        } else if (arg == "-seed" && i + 1 < argc) {
            seed = std::stoul(argv[++i]);        
        } else if (arg == "-init" && i + 1 < argc) {
            arg = argv[++i];
            auto colon = arg.find(':');
            auto name = arg.substr(0, colon);
            params = colon == arg.npos ? InitParams{} : InitParams::parse(arg.substr(colon + 1));
            auto m = parseInitMethod(name);
            if (!m) {
                std::cerr << "unknown -init '" << argv[i] << "'\n";
                return 1;
            }          
            init = *m;

        } else {
            std::cerr << "usage: " << argv[0] << " [-N bodies] [-soft eps] [-triangle]\n";
            return 1;
        }
    }

    Renderer r(1000, 1000, "nbody");

    if (triangleTest) {
        // Square pyramid through the same draw path as the simulation: xyzm positions, xyz velocities.
        // The velocity is only used for colouring, so reuse each corner's position to give distinct colours.
        const glm::vec3 apex(0.f, 1.f, 0.f);
        const glm::vec3 b0(-1.f, -1.f, -1.f), b1(1.f, -1.f, -1.f), b2(1.f, -1.f, 1.f), b3(-1.f, -1.f, 1.f);
        const glm::vec3 corners[] = {
            b0, b1, apex,  b1, b2, apex,  b2, b3, apex,  b3, b0, apex, // sides
            b0, b2, b1,    b0, b3, b2,                                 // base
        };
        std::vector<float> pos, vel;
        for (const glm::vec3& c : corners) {
            pos.insert(pos.end(), {c.x, c.y, c.z, 1.f});
            vel.insert(vel.end(), {c.x, c.y, c.z});
        }
        while (!r.shouldClose()) {
            r.beginFrame();
            r.draw(pos.data(), vel.data(), glm::vec3(0.f), std::size(corners), DrawMode::Triangles);
            r.endFrame();
        }
        return 0;
    }

    Bodies b = makeBodies(init, n, seed, params);

    Simulator sim(b, soft);
    const float h = 1.f / 120.f;
    float acc = 0.f;

    while(!r.shouldClose()) {
        float dt = 1.f / 120.f;
        acc += std::min(dt, 0.25f);          // clamp so a stall doesn't cause a spiral
        while (acc >= h) { sim.step(h); acc -= h; }
        r.beginFrame();
        r.draw(sim.positions(), sim.velocities(), sim.centerOfMass(), sim.count());
        r.endFrame();

        std::cout <<  "Virial Ratio:\t" << sim.virialRatio() << '\n';
    }

    return 0;
}