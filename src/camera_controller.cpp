#include <nbody/camera_controller.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

CameraController::CameraController(glm::vec3 target, float radius)
    : m_target(target), m_radius(radius), m_yaw(0.0f), m_pitch(0.0f),
      m_sensitivity(0.0005f), m_zoomSpeed(0.5f) {
}

CameraController::~CameraController() = default;

void CameraController::rotate(float deltaX, float deltaY) {
    m_yaw += deltaX * m_sensitivity;
    m_pitch += deltaY * m_sensitivity;
    if (m_pitch > 89.0f) m_pitch = 89.0f;
    if (m_pitch < -89.0f) m_pitch = -89.0f;
}

void CameraController::zoom(float offset) {
    m_radius += offset * m_zoomSpeed;
    if (m_radius < 1.0f) 
        m_radius = 1.0f;
}

glm::mat4 CameraController::getViewMatrix() {
    glm::vec3 up = {0.0,1.0,0.0};
    glm::vec3 camPos = this->position();
    return glm::lookAt(camPos, m_target, up);
}

glm::mat4 CameraController::getProjectionmatrix(int windowWidth, int windowHeight) {
    float aspect = (windowHeight > 0) ? (float)windowWidth / (float)windowHeight : 1.0f; 
    return glm::perspective(fov, aspect, nearPlane, farPlane);

}


glm::vec3 CameraController::position() {
    float yawRad = glm::radians(m_yaw);
    float pitchRad = glm::radians(m_pitch);


    glm::vec3 offset;
    offset.x = m_radius * sin(pitchRad) * cos(yawRad);
    offset.y = m_radius * sin(pitchRad) * sin(yawRad);
    offset.z = m_radius * cos(yawRad);

    return offset + m_target;
}

void CameraController::setTarget(glm::vec3 target) {
    m_target = target;
}