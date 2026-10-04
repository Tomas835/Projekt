#pragma once
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

// Model = VBO + VAO, data jsou pozice (3 floaty) + barva/normala (3 floaty) na vrchol
class Model
{
public:
	Model(const float* data, int floatCount);
	void draw() const;

private:
	GLuint vbo;
	GLuint vao;
	int vertexCount;
};
