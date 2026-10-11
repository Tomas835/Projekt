#include "Scene.h"

Scene::Scene(const std::string& name) : name(name) {}

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
