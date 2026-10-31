#pragma once
#include <functional>

struct GLFWwindow;

class Renderer{

public:
    Renderer(int width, int height, const char* title);
    ~Renderer();
    Renderer(const Renderer&) = delete; // don't allow copies

    bool shouldClose() const;
    void beginFrame();
    void endFrame();

    void draw();

private:
    GLFWwindow* m_window = nullptr;
    int m_width = 0;
    int m_height = 0;

    // OpenGL
    unsigned int vbo;
    unsigned int vao;
    unsigned int vertexShader;
    unsigned int fragmentShader;
    unsigned int shaderProgram;

};
