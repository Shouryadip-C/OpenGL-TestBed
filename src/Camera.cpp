#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>


Camera::Camera(glm::vec3 position, glm::vec3 up, const float yaw, const float pitch)
  : m_position(position),
    m_worldUp(up),
    m_yaw(yaw),
    m_pitch(pitch),
    zoom(settings::camera::zoom),
    movementSpeed(settings::camera::movementSpeed),
    mouseSensitivity(settings::camera::mouseSensitivity)
{
    updateDirectionVectors();
}

Camera::~Camera() {}

glm::mat4 Camera::getViewMatrix()
{
    return glm::lookAt(m_position, m_position + m_front, m_up);
}

void Camera::processKeyboardEvents(const Movement direction, const float deltaTime)
{
    const float speed = movementSpeed * deltaTime;
    if (direction == Movement::Front) {
        m_position += speed * m_front;
    }
    else if (direction == Movement::Back) {
        m_position -= speed * m_front;
    }
    if (direction == Movement::Right) {
        m_position += m_right * speed;
    }
    else if (direction == Movement::Left) {
        m_position -= m_right * speed;
    }
    if (direction == Movement::Up) {
        m_position += m_up * speed;
    }
    else if (direction == Movement::Down) {
        m_position -= m_up * speed;
    }
}

void Camera::processMouseMove(const float xOffset, const float yOffset, const bool constrainPitch)
{
    m_yaw += xOffset * mouseSensitivity;
    m_pitch += yOffset * mouseSensitivity;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrainPitch) {
        if (m_pitch > 89.0f) m_pitch = 89.0f;
        if (m_pitch < -89.0f) m_pitch = -89.0f;
    }

    // update Front, Right and Up Vectors using the updated Euler angles
    updateDirectionVectors();
}

void Camera::processMouseScroll(const float yOffset)
{
    zoom -= (float)yOffset;
    if (zoom < 5.0f) zoom = 5.0f;
    if (zoom > 75.0f) zoom = 75.0f;
}

void Camera::updateDirectionVectors()
{
    glm::vec3 front;
    front.y = sinf(glm::radians(m_pitch));
    front.x = cosf(glm::radians(m_pitch)) * cosf(glm::radians(m_yaw));
    front.z = cosf(glm::radians(m_pitch)) * sinf(glm::radians(m_yaw));
    m_front = glm::normalize(front);
    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up    = glm::normalize(glm::cross(m_right, m_front));
}
