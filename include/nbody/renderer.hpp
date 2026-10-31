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

    void draw(const float* xyz, std::size_t count);

private:
    GLFWwindow* m_window = nullptr;
    int m_width = 0;
    int m_height = 0;

    // OpenGL
    unsigned int vbo;
    unsigned int vao;
    unsigned int shaderProgram;

};
