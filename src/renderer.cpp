#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <functional>
#include <nbody/renderer.hpp>

const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";

const char *fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}\0";



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
    glCreateBuffers(1, &vbo);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo); 

    // Vertex Array
    glCreateVertexArrays(1, &vao);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);


    // Shaders
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

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


Renderer::~Renderer() {
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);  

    glfwDestroyWindow(m_window);
    glfwTerminate();
}