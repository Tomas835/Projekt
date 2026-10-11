#pragma once
#include "Model.h"
#include "ShaderProgram.h"
#include "Transform.h"

class DrawableObject
{
public:
    DrawableObject(Model* model, ShaderProgram* program, TransformPtr transform = nullptr);

    void setTransform(TransformPtr newTransform);
    glm::mat4 modelMatrix(float time) const;
    void draw(float time) const;

private:
    Model* model;
    ShaderProgram* program;
    TransformPtr transform;
};
#pragma once
