#pragma once
#include "Model.h"
#include "ShaderProgram.h"
#include <string>
#include <vector>

struct SceneObject
{
    Model* model = nullptr;
    ShaderProgram* program = nullptr;
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float scale = 1.0f;
    float rotation = 0.0f;
};

class Scene
{
public:
    explicit Scene(const std::string& name);

    void add(Model* model, ShaderProgram* program,
             float x, float y, float z, float scale, float rotation = 0.0f);
    void draw() const;
    const std::string& getName() const;

private:
    std::string name;
    std::vector<SceneObject> objects;
};
