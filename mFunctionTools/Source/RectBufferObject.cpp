#include "RectBufferObject.h"

RectBufferObject::RectBufferObject()
{
	float Vertex[24] = { -1.0, -1.0, 0.0, 1.0, 0.0, 0.0,
						-1.0,  1.0, 0.0, 1.0, 0.0, 1.0,
						 1.0,  1.0, 0.0, 1.0, 1.0, 1.0,
						 1.0, -1.0, 0.0, 1.0, 1.0, 0.0 };

	unsigned short Index[6] = { 0, 1, 2, 0, 2, 3 };
	NumFaces = 2;
	IndexType = IndexSizeType::Index16Bits;

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), Vertex, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned short), Index, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glBindVertexBuffer(0, VBO, 0, 6 * sizeof(float));  //BindingIndex = 0;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glEnableVertexAttribArray(0);
	glVertexAttribBinding(0, 0); //BindingIndex = 0;
	glVertexAttribFormat(0, 4, GL_FLOAT, false, 0);
	glEnableVertexAttribArray(1);
	glVertexAttribBinding(1, 0);
	glVertexAttribFormat(1, 2, GL_FLOAT, false, 16);

	glBindVertexArray(0);
}

RectBufferObject::~RectBufferObject()
{
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &IBO);
	glDeleteVertexArrays(1, &VAO);
}