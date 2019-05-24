#pragma once
#include "glm.hpp"

class SystemTextureFactory
{
public:
	SystemTextureFactory();
	~SystemTextureFactory();

	void GeneratePreIntegratedGFTexture();
	unsigned int GetPreIntegratedGF_Tex();

private:
	unsigned int PreIntegratedGF_Tex;
};


