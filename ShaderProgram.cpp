#include "ShaderProgram.h"
#include "Camera.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <glm/gtc/type_ptr.hpp>

GLuint ShaderProgram::compile(GLenum type, const char* file)
{
    std::ifstream f(file);
    if (!f.is_open())
    {
        std::cout << "Nelze otevrit " << file << std::endl;
        std::exit(1);
    }

    std::stringstream ss;
    ss << f.rdbuf();
    std::string code = ss.str();
    const char* src = code.c_str();

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint ok = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        char log[2048] = {};
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        std::cout << "Chyba v shaderu " << file << ":\n" << log << std::endl;
        glDeleteShader(shader);
        std::exit(1);
    }
    return shader;
}

ShaderProgram::ShaderProgram(const char* vertexFile, const char* fragmentFile)
{
    GLuint vs = compile(GL_VERTEX_SHADER, vertexFile);
    GLuint fs = compile(GL_FRAGMENT_SHADER, fragmentFile);

    id = glCreateProgram();
    glAttachShader(id, vs);
    glAttachShader(id, fs);
    glLinkProgram(id);

    GLint ok = GL_FALSE;
    glGetProgramiv(id, GL_LINK_STATUS, &ok);
    if (!ok)
    {
        char log[2048] = {};
        glGetProgramInfoLog(id, sizeof(log), nullptr, log);
        std::cout << "Chyba pri linkovani shader programu:\n" << log << std::endl;
        std::exit(1);
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
}

ShaderProgram::~ShaderProgram()
{
    if (id != 0)
        glDeleteProgram(id);
}

void ShaderProgram::use() const
{
    glUseProgram(id);
}

GLint ShaderProgram::uniformLocation(const char* name) const
{
    GLint location = glGetUniformLocation(id, name);
    if (location == -1)
        std::cerr << "Uniformni promenna '" << name << "' nebyla v shader programu nalezena.\n";
    return location;
}

bool ShaderProgram::setUniform(const char* name, int value) const
{
    GLint location = uniformLocation(name);
    if (location == -1) return false;
    glUniform1i(location, value);
    return true;
}

bool ShaderProgram::setUniform(const char* name, float value) const
{
    GLint location = uniformLocation(name);
    if (location == -1) return false;
    glUniform1f(location, value);
    return true;
}

bool ShaderProgram::setUniform(const char* name, float x, float y) const
{
    GLint location = uniformLocation(name);
    if (location == -1) return false;
    glUniform2f(location, x, y);
    return true;
}

bool ShaderProgram::setUniform(const char* name, float x, float y, float z) const
{
    GLint location = uniformLocation(name);
    if (location == -1) return false;
    glUniform3f(location, x, y, z);
    return true;
}

bool ShaderProgram::setUniform(const char* name, float x, float y, float z, float w) const
{
    GLint location = uniformLocation(name);
    if (location == -1) return false;
    glUniform4f(location, x, y, z, w);
    return true;
}

bool ShaderProgram::setUniform(const char* name, const glm::mat4& matrix) const
{
    GLint location = uniformLocation(name);
    if (location == -1) return false;
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
    return true;
}

void ShaderProgram::onCameraChanged(const Camera& camera) const
{
    use(); 
    setUniform("viewMatrix", camera.getViewMatrix());
    setUniform("projectionMatrix", camera.getProjectionMatrix());
}