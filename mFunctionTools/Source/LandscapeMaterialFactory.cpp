#include "mLandscapeMaterialFactory.h"

LandscapeMaterialFactory::LandscapeMaterialFactory(pvr::IAssetProvider& app, std::string VertexShaderFileName, std::string FragShaderTemplateFileName)
{
	auto VertexShaderSrc = app.getAssetStream(VertexShaderFileName);

	VertexShaderSrc->open();
	VertexShaderSrc->readIntoString(VertexShaderSrcCode);

	auto FragShaderTemplatesRC = app.getAssetStream(FragShaderTemplateFileName);

	FragShaderTemplatesRC->open();
	FragShaderTemplatesRC->readIntoString(FragShaderTemplateSrcCode);

	FindKeyFunctions();
}
LandscapeMaterialFactory::~LandscapeMaterialFactory()
{
}

void LandscapeMaterialFactory::FindKeyFunctions()
{
	int SamplePaintTextureFunctionBeginIndex = FragShaderTemplateSrcCode.find("//BeginSamplePaintTextureFunctionsTemplate");
	int SamplePaintTextureFunctionEndIndex = FragShaderTemplateSrcCode.find("//EndSamplePaintTextureFunctionsTemplate");

	int SampleFunctionBeginIndex = SamplePaintTextureFunctionBeginIndex;
	int SampleFunctionEndIndex = 0;
	while (true)
	{
		SampleFunctionEndIndex = FragShaderTemplateSrcCode.find("}", SampleFunctionBeginIndex);
		if (SampleFunctionEndIndex > SamplePaintTextureFunctionEndIndex)
		{
			break;
		}
		while (true)
		{
			if (FragShaderTemplateSrcCode[SampleFunctionEndIndex] == ' ')
			{
				SampleFunctionBeginIndex;
			}
			else
			{
				break;
			}
		}
		SampleFunctionBeginIndex = FragShaderTemplateSrcCode.find("vec3", SampleFunctionBeginIndex);
		while (true)
		{
			if (FragShaderTemplateSrcCode[SampleFunctionBeginIndex] == ' ')
			{
				SampleFunctionBeginIndex++;
			}
			else
			{
				break;
			}
		}
		SamplePaintTextureFunctions.push_back(FragShaderTemplateSrcCode.substr(SampleFunctionBeginIndex, SampleFunctionEndIndex - SampleFunctionBeginIndex + 1));
		SampleFunctionEndIndex++;
		SampleFunctionBeginIndex = SampleFunctionEndIndex;
	}

	int SampleWeightTextureFunctionBeginIndex = FragShaderTemplateSrcCode.find("//BeginSampleWeightTextureFunctionsTemplate");
	int SampleWeightTextureFunctionEndIndex = FragShaderTemplateSrcCode.find("//EndSampleWeightTextureFunctionsTemplate");

	SampleFunctionBeginIndex = SampleWeightTextureFunctionBeginIndex;
	SampleFunctionEndIndex = 0;
	while (true)
	{
		SampleFunctionEndIndex = FragShaderTemplateSrcCode.find("}", SampleFunctionBeginIndex);
		if (SampleFunctionEndIndex > SampleWeightTextureFunctionEndIndex)
		{
			break;
		}
		while (true)
		{
			if (FragShaderTemplateSrcCode[SampleFunctionEndIndex] == ' ')
			{
				SampleFunctionBeginIndex;
			}
			else
			{
				break;
			}
		}
		SampleFunctionBeginIndex = FragShaderTemplateSrcCode.find("float", SampleFunctionBeginIndex);
		while (true)
		{
			if (FragShaderTemplateSrcCode[SampleFunctionBeginIndex] == ' ')
			{
				SampleFunctionBeginIndex++;
			}
			else
			{
				break;
			}
		}
		SampleWeightTextureFunctions.push_back(FragShaderTemplateSrcCode.substr(SampleFunctionBeginIndex, SampleFunctionEndIndex - SampleFunctionBeginIndex + 1));
		SampleFunctionEndIndex++;
		SampleFunctionBeginIndex = SampleFunctionEndIndex;
	}
}


std::shared_ptr<Material> LandscapeMaterialFactory::CreateInstance(std::string MaterialDetail)
{
	std::string& VertShaderSrc = VertexShaderSrcCode;
	std::string& FragShaderSrcInstance = GenerateFragShaderSrcCode(MaterialDetail);

	return std::shared_ptr<Material>(new Material(VertShaderSrc, FragShaderSrcInstance));
}

std::string LandscapeMaterialFactory::GenerateFragShaderSrcCode(std::string& MaterialDetail)
{
	std::vector<std::string> LayerConfig;

	std::string FragShaderSrcCode = FragShaderTemplateSrcCode;

	int Begin = 0;
	while (true)
	{
		if (Begin == MaterialDetail.length())
		{
			break;
		}
		int Mid = MaterialDetail.find(',', Begin);
		LayerConfig.push_back(MaterialDetail.substr(Begin, Mid - Begin));
		Begin = Mid + 1;
	}

	int TextureDeclarationBeginIndex = FragShaderSrcCode.find("//BeginTextureDeclaration") + sizeof("//BeginTextureDeclaration") - 1;
	FragShaderSrcCode.insert(TextureDeclarationBeginIndex++, "\n");
	for (int LayerIndex = 0; LayerIndex < LayerConfig.size(); LayerIndex++)
	{
		int nameRight = LayerConfig[LayerIndex].find('_');
		std::string LayerName = LayerConfig[LayerIndex].substr(0, nameRight);
		FragShaderSrcCode.insert(TextureDeclarationBeginIndex, "uniform sampler2D PaintTexture_" + LayerName + ";\n");
		TextureDeclarationBeginIndex += sizeof("uniform sampler2D PaintTexture;\n") + LayerName.length();
	}

	std::vector<int>WeightTexAndChannel = std::vector<int>(LayerConfig.size());
	std::map<std::string, int> WeightTexIndexs;
	for (int LayerIndex = 0; LayerIndex < LayerConfig.size(); LayerIndex++)
	{
		int LayerWeightTexIndexBegin = LayerConfig[LayerIndex].find('_');
		int LayerWeightTexIndexEnd = LayerConfig[LayerIndex].find('_', ++LayerWeightTexIndexBegin);
		std::string LayerWeightTexIndex = LayerConfig[LayerIndex].substr(LayerWeightTexIndexBegin, LayerWeightTexIndexEnd - LayerWeightTexIndexBegin);

		std::map<std::string, int>::iterator it = WeightTexIndexs.find(LayerWeightTexIndex);
		if (it != WeightTexIndexs.end())
		{
			WeightTexAndChannel[LayerIndex] = std::stoi(LayerWeightTexIndex) << 16 | (it->second + 1);
			continue;
		}
		WeightTexAndChannel[LayerIndex] = std::stoi(LayerWeightTexIndex) << 16 | 0;
		WeightTexIndexs.insert(std::pair<std::string, int>(LayerWeightTexIndex, 0));
		FragShaderSrcCode.insert(TextureDeclarationBeginIndex, "uniform sampler2D WeightTexture_" + LayerWeightTexIndex + ";\n");
		TextureDeclarationBeginIndex += sizeof("uniform sampler2D WeightTexture_;\n") - 1 + LayerWeightTexIndex.length();
	}

	for (int LayerIndex = 0; LayerIndex < LayerConfig.size(); LayerIndex++)
	{
		//int LayerPosition = std::stoi(LayerConfig[LayerIndex].substr(0, LayerConfig[LayerIndex].find('_')));
		int SamplePaintPosition = FragShaderSrcCode.find(SamplePaintTextureFunctions[LayerIndex]);

		int SampleFunctionBegin = FragShaderSrcCode.find("{", SamplePaintPosition);
		int SampleFunctionEnd = FragShaderSrcCode.find("}", SamplePaintPosition);

		FragShaderSrcCode.erase(SampleFunctionBegin + 1, SampleFunctionEnd - SampleFunctionBegin - 1);
		FragShaderSrcCode.insert(SampleFunctionBegin++, "\n");
		FragShaderSrcCode.insert(++SampleFunctionBegin, "\n");

		int nameRight = LayerConfig[LayerIndex].find('_');
		std::string LayerName = LayerConfig[LayerIndex].substr(0, nameRight);

		bool SampleXY = LayerConfig[LayerIndex].find("XY") != -1;
		bool SampleXZ = LayerConfig[LayerIndex].find("XZ") != -1;
		bool SampleYZ = LayerConfig[LayerIndex].find("YZ") != -1;
		if (SampleYZ)
		{
			FragShaderSrcCode.insert(SampleFunctionBegin + 1, "    vec3 xColor = texture(PaintTexture_" + LayerName + ", PaintTexCoord_yz).rgb; \n");
			SampleFunctionBegin += sizeof("    vec3 xColor = texture(PaintTexture_") - 1 + LayerName.length() + sizeof(", PaintTexCoord_yz).rgb; \n") - 1;
		}
		if (SampleXZ)
		{
			FragShaderSrcCode.insert(SampleFunctionBegin + 1, "    vec3 yColor = texture(PaintTexture_" + LayerName + ", PaintTexCoord_xz).rgb; \n");
			SampleFunctionBegin += sizeof("    vec3 yColor = texture(PaintTexture_") - 1 + LayerName.length() + sizeof(", PaintTexCoord_xz).rgb; \n") - 1;
		}
		if (SampleXY)
		{
			FragShaderSrcCode.insert(SampleFunctionBegin + 1, "    vec3 zColor = texture(PaintTexture_" + LayerName + ", PaintTexCoord_xy).rgb; \n");
			SampleFunctionBegin += sizeof("    vec3 zColor = texture(PaintTexture_") - 1 + LayerName.length() + sizeof(", PaintTexCoord_xy).rgb; \n") - 1;
		}
		FragShaderSrcCode.insert(SampleFunctionBegin + 1, "    vec3 BlendWeights = abs(NormalWorld);\n");
		SampleFunctionBegin += sizeof("    vec3 BlendWeights = abs(NormalWorld);\n") - 1;

		if (!SampleYZ)
		{
			FragShaderSrcCode.insert(SampleFunctionBegin + 1, "    BlendWeights.x = 0.0;\n");
			SampleFunctionBegin += sizeof("    BlendWeights.x = 0.0;\n") - 1;
		}
		if (!SampleXZ)
		{
			FragShaderSrcCode.insert(SampleFunctionBegin + 1, "    BlendWeights.y = 0.0;\n");
			SampleFunctionBegin += sizeof("    BlendWeights.y = 0.0;\n") - 1;
		}
		if (!SampleXY)
		{
			FragShaderSrcCode.insert(SampleFunctionBegin + 1, "    BlendWeights.z = 0.0;\n");
			SampleFunctionBegin += sizeof("    BlendWeights.z = 0.0;\n") - 1;
		}

		FragShaderSrcCode.insert(SampleFunctionBegin + 1, "    BlendWeights = BlendWeights / (BlendWeights.x + BlendWeights.y + BlendWeights.z); \n");
		SampleFunctionBegin += sizeof("    BlendWeights = BlendWeights / (BlendWeights.x + BlendWeights.y + BlendWeights.z); \n") - 1;

		FragShaderSrcCode.insert(SampleFunctionBegin + 1, "    vec3 Color = vec3(0.0, 0.0, 0.0);\n");
		SampleFunctionBegin += sizeof("    vec3 Color = vec3(0.0, 0.0, 0.0);\n") - 1;

		if (SampleYZ)
		{
			FragShaderSrcCode.insert(SampleFunctionBegin + 1, "    Color += xColor * BlendWeights.x;\n");
			SampleFunctionBegin += sizeof("    Color += xColor * BlendWeights.x;\n") - 1;
		}
		if (SampleXZ)
		{
			FragShaderSrcCode.insert(SampleFunctionBegin + 1, "    Color += yColor * BlendWeights.y;\n");
			SampleFunctionBegin += sizeof("    Color += xColor * BlendWeights.x;\n") - 1;
		}
		if (SampleXY)
		{
			FragShaderSrcCode.insert(SampleFunctionBegin + 1, "    Color += zColor * BlendWeights.z;\n");
			SampleFunctionBegin += sizeof("    Color += zColor * BlendWeights.z;\n") - 1;
		}
		FragShaderSrcCode.insert(SampleFunctionBegin + 1, "    return Color;\n");
	}


	std::string MaskString[4] = { 
		"vec4(1.0, 0.0, 0.0, 0.0)",
		"vec4(0.0, 1.0, 0.0, 0.0)",
		"vec4(0.0, 0.0, 1.0, 0.0)",
		"vec4(0.0, 0.0, 0.0, 1.0)" };
	for (int LayerIndex = 0; LayerIndex < LayerConfig.size(); LayerIndex++)
	{
		int SampleWeightPosition = FragShaderSrcCode.find(SampleWeightTextureFunctions[LayerIndex]);

		int SampleFunctionBegin = FragShaderSrcCode.find("{", SampleWeightPosition);
		int SampleFunctionEnd = FragShaderSrcCode.find("}", SampleWeightPosition);

		FragShaderSrcCode.erase(SampleFunctionBegin + 1, SampleFunctionEnd - SampleFunctionBegin - 1);
		FragShaderSrcCode.insert(SampleFunctionBegin++, "\n");
		FragShaderSrcCode.insert(++SampleFunctionBegin, "\n");

		int LayerWeightTexIndex = WeightTexAndChannel[LayerIndex] >> 16;
		int LayerChannelIndex = WeightTexAndChannel[LayerIndex] & 255;

		FragShaderSrcCode.insert(SampleFunctionBegin + 1, "    return dot(texture(WeightTexture_" + std::to_string(LayerWeightTexIndex) + ", WeightTexCoord),");
		SampleFunctionBegin += sizeof("    return dot(texture(WeightTexture_") + std::to_string(LayerWeightTexIndex).length() + sizeof(", WeightTexCoord),") - 2;

		FragShaderSrcCode.insert(SampleFunctionBegin + 1, MaskString[LayerChannelIndex] + ");\n");
		SampleFunctionBegin += MaskString[LayerChannelIndex].length() + sizeof(");\n") - 1;
	}

	return FragShaderSrcCode;
}

