#pragma once
#include <functional>

#define DIM 3

struct GLFWwindow;

class Renderer{

public:
    Renderer(int width, int height, const char* title);
    ~Renderer();
    Renderer(const Renderer&) = delete; // don't allow copies

    bool shouldClose() const;
    void beginFrame();
    void endFrame();

    void draw(const float* xyz, const float* vel, std::size_t count);
    void setCamera(float cx, float cy, float cz, float scale);

private:
    GLFWwindow* m_window = nullptr;
    int m_width = 0;
    int m_height = 0;

    // OpenGL
    unsigned int posvbo;
    unsigned int velvbo;
    unsigned int vao;
    unsigned int shaderProgram;

};
