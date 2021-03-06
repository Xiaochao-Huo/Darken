#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <typeinfo>
#include <memory>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <glew.h>
#include <map>
#include "TypeDefine.h"

enum ShaderType
{
	VertexShader,
	FragmentShader,
	GeometryShader,
	TessEvaluationShader,
	TessControlShader,
	ComputeShader
};

enum IndexSizeType
{
	Index16Bits,
	Index32Bits
};

enum UniformType
{
	Unknown,
	GLSL_INT,
	GLSL_UINT,
	GLSL_FLOAT,
	GLSL_VEC2,
	GLSL_VEC4,
	GLSL_VEC3,
	GLSL_IVEC2,
	GLSL_IVEC3,
	GLSL_IVEC4,
	GLSL_MAT3,
	GLSL_MAT4,
	GLSL_TEXTURE2D,
	GLSL_TEXTURECUBE,
	GLSL_TEXTURE3D,
	GLSL_TEXTURE_2D_ARRAY
};

enum GLDrawType
{
	Unkown,
	OGL_ELEMENT,
	OGL_LINE_STRIP
};

static std::unordered_map<UInt32, UniformType> UniformTypeMap = {
	{ GL_INT,   UniformType::GLSL_INT },
	{ GL_UNSIGNED_INT, UniformType::GLSL_UINT},
	{ GL_FLOAT, UniformType::GLSL_FLOAT},
	{ GL_FLOAT_VEC2,  UniformType::GLSL_VEC2 },
	{ GL_FLOAT_VEC3,  UniformType::GLSL_VEC3 },
	{ GL_FLOAT_VEC4,  UniformType::GLSL_VEC4 },
	{ GL_INT_VEC2, UniformType::GLSL_IVEC2 },
	{ GL_INT_VEC3, UniformType::GLSL_IVEC3 },
	{ GL_INT_VEC4, UniformType::GLSL_IVEC4 },
	{ GL_FLOAT_MAT3,  UniformType::GLSL_MAT3 },
	{ GL_FLOAT_MAT4,  UniformType::GLSL_MAT4 },
	{ GL_SAMPLER_2D, UniformType::GLSL_TEXTURE2D },
	{ GL_SAMPLER_CUBE, UniformType::GLSL_TEXTURECUBE },
	{ GL_SAMPLER_3D, UniformType::GLSL_TEXTURE3D },
	{ GL_SAMPLER_2D_ARRAY, UniformType::GLSL_TEXTURE_2D_ARRAY}
};

struct AttribItem
{
	Int32 Location;
	AttribItem() {}
};

struct UniformItem_WithinBlock
{
	std::string Name;
	UniformType DataType;
	UInt32 Size;
	Int32 Offset_Byte;
	UniformItem_WithinBlock() {}
};

struct UniformItem_Block
{
	Int32 Id;
	Int32 HashCode;
	UInt32 DataSize_Byte;
	void * DataPtr;
	UInt32 Index;
	std::map<UInt32, UniformItem_WithinBlock> Uniforms;
};

struct UniformItem_Basic
{
	Int32 Location;
	UniformType DataType;
	UInt32 Size;
	void* DataPtr;
	UniformItem_Basic() {}
};

struct UniformItem_Texture
{
	Int32 Location;
	UInt32* IDPtr;
	UniformType DataType;
	UniformItem_Texture() : Location(-1) {}
};

struct Shader
{
	UInt32 Id;
	std::string Name;
	ShaderType type;
	Shader() {}
};

struct Program
{
	UInt32 Id;
	std::vector<Shader> shaders;
	std::unordered_map<std::string, AttribItem> Attribs;
	std::unordered_map<std::string, std::shared_ptr<UniformItem_Block>> Uniforms_Block;
	std::unordered_map<std::string, UniformItem_Basic> Uniforms_Basic;
	std::unordered_map<std::string, UniformItem_Texture> Uniforms_Texture;
};

class Material
{
public:
	Material();
	~Material();

	std::shared_ptr<Program> MaterialProgram;

	void CreateMaterial(std::vector<std::string>& shaderNames);
	Material(std::vector<std::string> shaderNames);

	void Draw(UInt32 VAO, Int32 NumFaces, IndexSizeType indexSize, Int32 Offset = 0, GLDrawType drawType = GLDrawType::OGL_ELEMENT);
	void BindProgram();
	void UnBindProgram();
	void BindUniforms();
	void BindSamplers();
private:
	void LoadAndCreateShaders(std::vector<std::string>& shaderNames);
	void CreateProgram();
	void FindShaderNames(std::vector<std::string>& shaderNames);
	void FindAttibInfos();
	void FindUniformInfos();
	void LinkLocation();
	UInt32 CreateShaderGPUObjFromSrcCode(std::string & Code, ShaderType type);
};

