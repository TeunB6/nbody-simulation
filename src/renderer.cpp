#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <functional>
#include <fstream>
#include <sstream>
#include <string>
#include <nbody/renderer.hpp>

#ifndef SHADER_DIR
#define SHADER_DIR "shaders"
#endif


static unsigned int compileShader(GLenum type, const std::string& path) {
    std::ifstream f(path);
    if (!f) { std::cerr << "Cannot open shader " << path << "\n"; exit(EXIT_FAILURE); }
    std::stringstream ss; ss << f.rdbuf();
    std::string src = ss.str();
    const char* c = src.c_str();

    unsigned int s = glCreateShader(type);
    glShaderSource(s, 1, &c, NULL);
    glCompileShader(s);

    int ok; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024]; glGetShaderInfoLog(s, sizeof log, NULL, log);
        std::cerr << path << ":\n" << log << "\n";
        exit(EXIT_FAILURE);
    }
    return s;
}


static void error_callback(int error, const char* description) {
    std::cerr << "Error: " << error << description;
}

static void key_callback(GLFWwindow* window, int key, int scanncode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);   
}

Renderer::Renderer(int width, int height, const char *title) {
    // Window setup
    m_height = height; m_width = width;
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_window = glfwCreateWindow(m_width, m_height, title, NULL, NULL);

    if (!m_window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(m_window, key_callback);

    glfwMakeContextCurrent(m_window);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(1);


    // Buffers
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // Vertex Array
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glEnable(GL_PROGRAM_POINT_SIZE);



    // Shaders
    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, std::string(SHADER_DIR) + "/vertex.glsl");
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, std::string(SHADER_DIR) + "/fragment.glsl");

    shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);  


    glUseProgram(shaderProgram);
}

void Renderer::beginFrame() {
    glfwPollEvents();
    glViewport(0, 0, m_width, m_height);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::endFrame() {
    glfwSwapBuffers(m_window);
}

bool Renderer::shouldClose() const {
    return glfwWindowShouldClose(m_window);
}

void Renderer::draw(const float* xyz, std::size_t count) {
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, count * DIM * sizeof(float), xyz, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(count));
}

Renderer::~Renderer() {
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);

    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(m_window);
    glfwTerminate();
}