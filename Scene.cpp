#include "Scene.h"

namespace
{
    constexpr float PI = 3.14159265358979323846f;
}

Scene::Scene(const std::string& name)
    : name(name), cameraEye(0.0f, 0.0f, 1.75f), cameraFi(-PI / 2.0f), cameraAlpha(PI / 2.0f), moveSpeed(1.5f),
    lightPosition(0.0f), lightColor(1.0f), lightAttenuation(1.0f, 0.0f, 0.0f), clearColor(0.035f, 0.045f, 0.08f), freeLook(false) {}

DrawableObject& Scene::add(Model* model, ShaderProgram* program, TransformPtr transform)
{
    objects.emplace_back(model, program, std::move(transform));
    return objects.back();
}

void Scene::draw(float time) const
{
    for (const DrawableObject& object : objects)
        object.draw(time);
}

const std::string& Scene::getName() const
{
    return name;
}

void Scene::setCameraStart(const glm::vec3& eye, float fi, float alpha, float speed)
{
    cameraEye = eye;
    cameraFi = fi;
    cameraAlpha = alpha;
    moveSpeed = speed;
}

const glm::vec3& Scene::getCameraEye() const { return cameraEye; }
float Scene::getCameraFi() const { return cameraFi; }
float Scene::getCameraAlpha() const { return cameraAlpha; }
float Scene::getMoveSpeed() const { return moveSpeed; }
void Scene::setLight(const glm::vec3& position, const glm::vec3& color)
{
    lightPosition = position;
    lightColor = color;
}

const glm::vec3& Scene::getLightPosition() const { return lightPosition; }
const glm::vec3& Scene::getLightColor() const { return lightColor; }
void Scene::setLightPath(std::function<glm::vec3(float)> path) { lightPath = std::move(path); }
glm::vec3 Scene::getLightPosition(float time) const { return lightPath ? lightPath(time) : lightPosition; }
void Scene::setLightAttenuation(const glm::vec3& attenuation) { lightAttenuation = attenuation; }
const glm::vec3& Scene::getLightAttenuation() const { return lightAttenuation; }
void Scene::setClearColor(const glm::vec3& color) { clearColor = color; }
const glm::vec3& Scene::getClearColor() const { return clearColor; }
void Scene::setFreeLook(bool enabled) { freeLook = enabled; }
bool Scene::isFreeLook() const { return freeLook; }