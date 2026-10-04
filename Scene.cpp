#include "Scene.h"

Scene::Scene(const std::string& name) : name(name) {}

void Scene::add(Model* model, ShaderProgram* program,
                float x, float y, float z, float scale, float rotation)
{
    objects.push_back({ model, program, x, y, z, scale, rotation });
}

void Scene::draw() const
{
    for (const SceneObject& object : objects)
    {
        object.program->use();
        object.program->setTransform(object.x, object.y, object.z,
                                     object.scale, object.rotation);
        object.model->draw();
    }
}

const std::string& Scene::getName() const
{
    return name;
}
