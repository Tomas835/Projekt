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

    void setLight(const glm::vec3& position, const glm::vec3& color = glm::vec3(1.0f));
    const glm::vec3& getLightPosition() const;
    const glm::vec3& getLightColor() const;

    void setFreeLook(bool enabled);
    bool isFreeLook() const;

private:
    std::string name;
    std::vector<DrawableObject> objects;

    glm::vec3 cameraEye;
    float cameraFi;
    float cameraAlpha;
    float moveSpeed;
    glm::vec3 lightPosition;
    glm::vec3 lightColor;
    bool freeLook;
};