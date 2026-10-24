#include "Camera.h"
#include "ShaderProgram.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

namespace
{
    constexpr float PI = 3.14159265358979323846f;
    constexpr float ALPHA_MIN = 0.0174533f;
    constexpr float ALPHA_MAX = PI - ALPHA_MIN;
}

Camera::Camera()
    : eye(0.0f, 0.0f, 1.75f), target(0.0f, 0.0f, -1.0f), up(0.0f, 1.0f, 0.0f),
    alpha(PI / 2.0f), fi(-PI / 2.0f),
    fov(glm::radians(60.0f)), aspect(4.0f / 3.0f), zNear(0.1f), zFar(200.0f)
{
    updateTarget();
}

void Camera::attach(ShaderProgram* program)
{
    if (std::find(observers.begin(), observers.end(), program) == observers.end())
        observers.push_back(program);
    program->onCameraChanged(*this);
}

void Camera::detach(ShaderProgram* program)
{
    observers.erase(std::remove(observers.begin(), observers.end(), program), observers.end());
}

glm::mat4 Camera::getViewMatrix() const
{
    return glm::lookAt(eye, eye + target, up);
}

glm::mat4 Camera::getProjectionMatrix() const
{
    return glm::perspective(fov, aspect, zNear, zFar);
}

glm::vec3 Camera::getPosition() const
{
    return eye;
}

void Camera::rotate(float deltaFi, float deltaAlpha)
{
    fi = std::fmod(fi + deltaFi, 2.0f * PI);
    alpha = std::clamp(alpha + deltaAlpha, ALPHA_MIN, ALPHA_MAX);
    updateTarget();
    notify();
}

void Camera::move(float forward, float right)
{
    const glm::vec3 rightDir = glm::normalize(glm::cross(target, up));
    eye += target * forward + rightDir * right;
    notify();
}

void Camera::setAspect(float newAspect)
{
    aspect = newAspect;
    notify();
}

void Camera::setPose(const glm::vec3& newEye, float newFi, float newAlpha)
{
    eye = newEye;
    fi = newFi;
    alpha = std::clamp(newAlpha, ALPHA_MIN, ALPHA_MAX);
    updateTarget();
    notify();
}

void Camera::updateTarget()
{
    target.x = std::sin(alpha) * std::cos(fi);
    target.z = std::sin(alpha) * std::sin(fi);
    target.y = std::cos(alpha);
}

void Camera::notify() const
{
    for (ShaderProgram* program : observers)
        program->onCameraChanged(*this);
}