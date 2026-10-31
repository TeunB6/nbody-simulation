#include <iostream>
#include <nbody/renderer.hpp>
#include <nbody/simulator.hpp>

int main(int argc, char** argv) {
    int n = 1000;
    float soft = 0.5;
    bool triangleTest = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-N" && i + 1 < argc) {
            n = std::stoi(argv[++i]); // read next token
        } else if (arg == "-triangle") {
            triangleTest = true;
        } else if (arg == "-soft" && i + 1 < argc)
            soft = std::stof(argv[++i]);        
        else {
            std::cerr << "usage: " << argv[0] << " [-N bodies] [-soft eps] [-triangle]\n";
            return 1;
        }
    }

    Renderer r(1000, 1000, "nbody");

    if (triangleTest) {
        // Same draw path as the simulation: xyzm positions, xyz velocities (give each vertex a colour).
        const float pos[] = {-1.f, -1.f, 0.f, 1.f,
                              1.f, -1.f, 0.f, 1.f,
                              0.f,  1.f, 0.f, 1.f};
        const float vel[] = {1.f, 0.f, 0.f,
                             0.f, 1.f, 0.f,
                             0.f, 0.f, 1.f};
        while (!r.shouldClose()) {
            r.beginFrame();
            r.draw(pos, vel, glm::vec3(0.f), 3, DrawMode::Triangles);
            r.endFrame();
        }
        return 0;
    }

    Simulator sim(n, 1.0, soft);
    const float h = 1.f / 120.f;
    float acc = 0.f;

    while(!r.shouldClose()) {
        float dt = 1.f / 60.f;
        acc += std::min(dt, 0.25f);          // clamp so a stall doesn't cause a spiral
        while (acc >= h) { sim.step(h); acc -= h; }
        r.beginFrame();
        r.draw(sim.positions(), sim.velocities(), sim.centerOfMass(), sim.count());
        r.endFrame();
    }

    return 0;
}