#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <functional>
#include <memory>
#include <vector>

class Transform
{
public:
    virtual ~Transform() = default;
    virtual glm::mat4 matrix(float time) const = 0;
};

using TransformPtr = std::shared_ptr<Transform>;


class Translate : public Transform
{
public:
    explicit Translate(const glm::vec3& offset) : offset(offset) {}
    glm::mat4 matrix(float) const override { return glm::translate(glm::mat4(1.0f), offset); }
private:
    glm::vec3 offset;
};

class Rotate : public Transform
{
public:
    Rotate(float angle, const glm::vec3& axis) : angle(angle), axis(axis) {}
    glm::mat4 matrix(float) const override { return glm::rotate(glm::mat4(1.0f), angle, axis); }
private:
    float angle;
    glm::vec3 axis;
};

class Scale : public Transform
{
public:
    explicit Scale(const glm::vec3& factor) : factor(factor) {}
    explicit Scale(float factor) : factor(factor, factor, factor) {}
    glm::mat4 matrix(float) const override { return glm::scale(glm::mat4(1.0f), factor); }
private:
    glm::vec3 factor;
};


class DynamicRotate : public Transform
{
public:
    DynamicRotate(const glm::vec3& axis, float speed, float phase = 0.0f)
        : axis(axis), speed(speed), phase(phase) {}
    glm::mat4 matrix(float time) const override
    {
        return glm::rotate(glm::mat4(1.0f), phase + speed * time, axis);
    }
private:
    glm::vec3 axis;
    float speed;
    float phase;
};

class DynamicTranslate : public Transform
{
public:
    explicit DynamicTranslate(std::function<glm::vec3(float)> offset) : offset(std::move(offset)) {}
    glm::mat4 matrix(float time) const override { return glm::translate(glm::mat4(1.0f), offset(time)); }
private:
    std::function<glm::vec3(float)> offset;
};

class DynamicScale : public Transform
{
public:
    explicit DynamicScale(std::function<glm::vec3(float)> factor) : factor(std::move(factor)) {}
    glm::mat4 matrix(float time) const override { return glm::scale(glm::mat4(1.0f), factor(time)); }
private:
    std::function<glm::vec3(float)> factor;
};

class TransformComposite : public Transform
{
public:
    TransformComposite& add(TransformPtr child)
    {
        children.push_back(std::move(child));
        return *this;
    }

    void clear() { children.clear(); }

    void swap(size_t a, size_t b)
    {
        if (a < children.size() && b < children.size())
            std::swap(children[a], children[b]);
    }

    glm::mat4 matrix(float time) const override
    {
        glm::mat4 result(1.0f);
        for (const TransformPtr& child : children)
            result *= child->matrix(time);
        return result;
    }

private:
    std::vector<TransformPtr> children;
};

using CompositePtr = std::shared_ptr<TransformComposite>;

inline CompositePtr makeComposite(std::initializer_list<TransformPtr> parts)
{
    auto composite = std::make_shared<TransformComposite>();
    for (const TransformPtr& part : parts) composite->add(part);
    return composite;
}
