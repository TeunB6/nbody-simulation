#pragma once
#include <functional>
#include <nbody/camera_controller.hpp>

#define DIM 3

struct GLFWwindow;

enum class DrawMode { Points, Triangles };

struct InputState {
    bool isLeftMouseDown = false;
    double lastX = 0.0, lastY = 0.0;
};

class Renderer{

public:
    Renderer(int width, int height, const char* title);
    ~Renderer();
    Renderer(const Renderer&) = delete; // don't allow copies

    bool shouldClose() const;
    void beginFrame();
    void endFrame();

    void draw(const float* xyz, const float* vel, glm::vec3 center, std::size_t count,
              DrawMode mode = DrawMode::Points);
    void setColorBySpeed(bool on) { m_colorBySpeed = on; }
    void setCamera(float cx, float cy, float cz, float scale);

private:
    GLFWwindow* m_window = nullptr;
    CameraController m_cam;


    bool m_colorBySpeed = true;
    int m_width = 0;
    int m_height = 0;

    // OpenGL
    unsigned int vboPos;
    unsigned int vboVel;
    unsigned int vao;
    unsigned int shaderProgram;

};
