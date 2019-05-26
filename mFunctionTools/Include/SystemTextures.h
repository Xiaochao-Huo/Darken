#pragma once
#include "TypeDefine.h"

class SystemTextureFactory
{
public:
	SystemTextureFactory();
	~SystemTextureFactory();

	void GeneratePreIntegratedGFTexture();
	UInt32 GetPreIntegratedGF_Tex();

private:
	UInt32 PreIntegratedGF_Tex;
};


