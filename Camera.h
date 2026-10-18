#pragma once
#include <glm/glm.hpp>
#include <vector>

class ShaderProgram;


class Camera
{
public:
    Camera();

    void attach(ShaderProgram* program);
    void detach(ShaderProgram* program);

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;


    void rotate(float deltaFi, float deltaAlpha);

    void move(float forward, float right);
    void setAspect(float aspect);
    void setPose(const glm::vec3& newEye, float newFi, float newAlpha);

private:
    void updateTarget();
    void notify() const;

    glm::vec3 eye;
    glm::vec3 target;
    glm::vec3 up;
    float alpha;
    float fi;

    float fov;
    float aspect;
    float zNear;
    float zFar;

    std::vector<ShaderProgram*> observers;
};