#pragma once

#include <glm/glm.hpp>

#include <Settings.h>


enum class Movement { None, Front, Back, Left, Right, Up, Down };

class Camera
{
public:
    float mouseSensitivity;
    float movementSpeed;
    float zoom;

private:
    float     m_pitch;
    float     m_yaw;
    glm::vec3 m_position;
    glm::vec3 m_front;
    glm::vec3 m_right;
    glm::vec3 m_up;
    glm::vec3 m_worldUp;

public:
    Camera(glm::vec3   position = glm::vec3(0.0f, 0.0f, 0.0f),
           glm::vec3   up       = glm::vec3(0.0f, 1.0f, 0.0f),
           const float yaw      = settings::camera::yaw,
           const float pitch    = settings::camera::pitch);
    ~Camera();

    glm::mat4 getViewMatrix();
    void      processKeyboardEvents(const Movement direction, const float deltaTime);
    void      processMouseMove(const float xOffset, const float yOffset, const bool constrainPitch = true);
    void      processMouseScroll(const float yOffset);

private:
    void updateDirectionVectors();
};
