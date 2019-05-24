#pragma once

#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsGles.h"
#include "mMaterial.h"
#include <map>

class LandscapeMaterialFactory
{
public:
	LandscapeMaterialFactory(pvr::IAssetProvider& app, std::string VertexShaderFileName, std::string FragShaderTemplateFileName);
	~LandscapeMaterialFactory();
	std::shared_ptr<Material> CreateInstance(std::string MaterialDetail);

private:
	std::string VertexShaderSrcCode;
	std::string FragShaderTemplateSrcCode;

	std::vector<std::string> SamplePaintTextureFunctions;
	std::vector<std::string> SampleWeightTextureFunctions;

	void FindKeyFunctions();
	std::string GenerateFragShaderSrcCode(std::string& MaterialDetail);

};

