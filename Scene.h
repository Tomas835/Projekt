#pragma once
#include "DrawableObject.h"
#include <glm/glm.hpp>
#include <functional>
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

    // Moving light (firefly): the position is a function of time.
    // Attenuation = (constant, linear, quadratic) coefficients of 1 / (kc + kl * d + kq * d^2).
    void setLightPath(std::function<glm::vec3(float)> path);
    glm::vec3 getLightPosition(float time) const;   // path position if a path is set, otherwise the static position
    void setLightAttenuation(const glm::vec3& attenuation);
    const glm::vec3& getLightAttenuation() const;

    // Background (clear) color of the scene - e.g. almost black for the night forest
    void setClearColor(const glm::vec3& color);
    const glm::vec3& getClearColor() const;

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
    glm::vec3 lightAttenuation;                      // default (1, 0, 0) = no attenuation
    std::function<glm::vec3(float)> lightPath;       // optional light motion over time
    glm::vec3 clearColor;                            // background color
    bool freeLook;
};