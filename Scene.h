#pragma once
#include "DrawableObject.h"
#include <string>
#include <vector>

class Scene
{
public:
    explicit Scene(const std::string& name);

    DrawableObject& add(Model* model, ShaderProgram* program, TransformPtr transform = nullptr);
    void draw(float time) const;
    const std::string& getName() const;

private:
    std::string name;
    std::vector<DrawableObject> objects;
};
