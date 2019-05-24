#pragma once
#include "MaterialInstance.h"

class RectBufferObject
{
public:
	RectBufferObject();
	~RectBufferObject();
	unsigned int VAO;
	unsigned int VBO;
	unsigned int IBO;
	int NumFaces;
	IndexSizeType IndexType;
private:

};