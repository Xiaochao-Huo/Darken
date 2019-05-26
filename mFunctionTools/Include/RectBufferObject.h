#pragma once
#include "MaterialInstance.h"

class RectBufferObject
{
public:
	RectBufferObject();
	~RectBufferObject();
	UInt32 VAO;
	UInt32 VBO;
	UInt32 IBO;
	Int32 NumFaces;
	IndexSizeType IndexType;
private:

};