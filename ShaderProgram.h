#pragma once
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

class ShaderProgram
{
public:
    ShaderProgram(const char* vertexFile, const char* fragmentFile);
    ~ShaderProgram();

    void use() const;

    bool setUniform(const char* name, int value) const;
    bool setUniform(const char* name, float value) const;
    bool setUniform(const char* name, float x, float y) const;
    bool setUniform(const char* name, float x, float y, float z) const;
    bool setUniform(const char* name, float x, float y, float z, float w) const;

    void setTransform(float x, float y, float z, float scale, float rotation) const;

private:
    GLuint compile(GLenum type, const char* file);
    GLint uniformLocation(const char* name) const;
    GLuint id;
};
