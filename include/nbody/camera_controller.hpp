#pragma once
#include<vector_types.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


class CameraController {
public:
    CameraController(glm::vec3 target, float radius);
    ~CameraController();

    void zoom(float offset);
    void rotate(float deltaX, float deltaY);

    glm::vec3 position();
    glm::mat4 getViewMatrix();
    glm::mat4 getProjectionmatrix(int windowWidth, int windowHeight);

    void setTarget(glm::vec3 target);


private:
    glm::vec3 m_target;
    float m_radius;
    float m_yaw;
    float m_pitch;

    float fov = glm::radians(45.0f);
    float nearPlane = 0.1f;          
    float farPlane = 100.0f;

    float m_sensitivity;
    float m_zoomSpeed;
};
