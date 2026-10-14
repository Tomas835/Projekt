#pragma once
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

class Model
{
public:
	Model(const float* data, int floatCount, int floatsPerVertex = 6, int colorOffset = 3);
	void draw() const;

private:
	GLuint vbo;
	GLuint vao;
	int vertexCount;
};