#pragma once
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

class Model
{
public:
	// Attributes: 0 = position (offset 0), 1 = color (colorOffset), 2 = normal (normalOffset, -1 = model has no normals)
	Model(const float* data, int floatCount, int floatsPerVertex = 6, int colorOffset = 3, int normalOffset = -1);
	void draw() const;

private:
	GLuint vbo;
	GLuint vao;
	int vertexCount;
};
