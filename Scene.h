#pragma once
#include "DrawableObject.h"
#include <glm/glm.hpp>
#include <string>
#include <vector>

class Scene
{
public:
    explicit Scene(const std::string& name);

    DrawableObject& add(Model* model, ShaderProgram* program, TransformPtr transform = nullptr);
    void draw(float time) const;
    const std::string& getName() const;

    void setCameraStart(const glm::vec3& eye, float fi, float alpha, float moveSpeed);
    const glm::vec3& getCameraEye() const;
    float getCameraFi() const;
    float getCameraAlpha() const;
    float getMoveSpeed() const;

private:
    std::string name;
    std::vector<DrawableObject> objects;

    glm::vec3 cameraEye;
    float cameraFi;
    float cameraAlpha;
    float moveSpeed;
};
