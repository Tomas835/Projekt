#include "DrawableObject.h"

DrawableObject::DrawableObject(Model* model, ShaderProgram* program, TransformPtr transform)
    : model(model), program(program), transform(std::move(transform)) {}

void DrawableObject::setTransform(TransformPtr newTransform)
{
    transform = std::move(newTransform);
}

glm::mat4 DrawableObject::modelMatrix(float time) const
{
    return transform ? transform->matrix(time) : glm::mat4(1.0f);
}

void DrawableObject::draw(float time) const
{
    program->use();
    program->setUniform("modelMatrix", modelMatrix(time));
    model->draw();
}