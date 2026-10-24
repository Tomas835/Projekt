#include "Model.h"

Model::Model(const float* data, int floatCount, int floatsPerVertex, int colorOffset, int normalOffset)
{
	vertexCount = floatCount / floatsPerVertex;

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, floatCount * sizeof(float), data, GL_STATIC_DRAW);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, floatsPerVertex * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, floatsPerVertex * sizeof(float), (void*)(colorOffset * sizeof(float)));

	if (normalOffset >= 0)
	{
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, floatsPerVertex * sizeof(float), (void*)(normalOffset * sizeof(float)));
	}
}

void Model::draw() const
{
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, vertexCount);
}
