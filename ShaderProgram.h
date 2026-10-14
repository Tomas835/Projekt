#pragma once
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Camera; // dopredna deklarace (z duvodu krizoveho odkazu)

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

    bool setUniform(const char* name, const glm::mat4& matrix) const;

    // Observer: zavola kamera pri zmene, program si aktualizuje view a projection matici.
    void onCameraChanged(const Camera& camera) const;

private:
    GLuint compile(GLenum type, const char* file);
    GLint uniformLocation(const char* name) const;
    GLuint id;
};