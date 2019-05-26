#include "Material.h"
#include "GlobalPram.h"
#include "BufferManager.h"
#include <glew.h>
#include <glfw3.h>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>

Material::Material()
{
}

Material::~Material()
{
}

Material::Material(std::vector<std::string> shaderNames)
{
	LoadAndCreateShaders(shaderNames);
	FindShaderNames(shaderNames);
	CreateProgram();
	FindAttibInfos();
	FindUniformInfos();
	LinkLocation();
}

void Material::CreateMaterial(std::vector<std::string>& shaderNames)
{
	LoadAndCreateShaders(shaderNames);
	FindShaderNames(shaderNames);
	CreateProgram();
	FindAttibInfos();
	FindUniformInfos();
	LinkLocation();
}
void Material::BindProgram()
{
	glUseProgram(MaterialProgram->Id);
}
void Material::UnBindProgram()
{
	glUseProgram(0);
}
void Material::BindUniforms()
{
	//for (std::unordered_map<std::string, UniformItem_Basic>::iterator it = MaterialProgram->Uniforms_Basic.begin(); it != MaterialProgram->Uniforms_Basic.end(); it++)
	for(auto& uniformitempair: MaterialProgram->Uniforms_Basic)
	{
		auto& uniformitem = uniformitempair.second;
		if (uniformitem.DataPtr)
		{
			Int32 Location      = uniformitem.Location;
			UInt32 Size = uniformitem.Size;
			switch (uniformitem.DataType)
			{
			case UniformType::GLSL_VEC2: glUniform2fv(Location, Size, reinterpret_cast<GLfloat*>(uniformitem.DataPtr)); break;
			case UniformType::GLSL_VEC3: glUniform3fv(Location, Size, reinterpret_cast<GLfloat*>(uniformitem.DataPtr)); break;
			case UniformType::GLSL_VEC4: glUniform4fv(Location, Size, reinterpret_cast<GLfloat*>(uniformitem.DataPtr)); break;
			case UniformType::GLSL_IVEC2: glUniform2iv(Location, Size, reinterpret_cast<GLint*>(uniformitem.DataPtr)); break;
			case UniformType::GLSL_IVEC3: glUniform3iv(Location, Size, reinterpret_cast<GLint*>(uniformitem.DataPtr)); break;
			case UniformType::GLSL_IVEC4: glUniform4iv(Location, Size, reinterpret_cast<GLint*>(uniformitem.DataPtr)); break;
			case UniformType::GLSL_MAT3: glUniformMatrix3fv(Location, Size, GL_FALSE, reinterpret_cast<GLfloat*>(uniformitem.DataPtr)); break;
			case UniformType::GLSL_MAT4: glUniformMatrix4fv(Location, Size, GL_FALSE, reinterpret_cast<GLfloat*>(uniformitem.DataPtr)); break;
			case UniformType::GLSL_FLOAT: glUniform1fv(Location, Size, reinterpret_cast<GLfloat*>(uniformitem.DataPtr)); break;
			case UniformType::GLSL_INT:	glUniform1iv(Location, Size, reinterpret_cast<GLint*>(uniformitem.DataPtr)); break;
			default:
				break;
			}
		}
	}
}
void Material::BindSamplers()
{
	//for (std::unordered_map<std::string, UniformItem_Texture>::iterator it = MaterialProgram->Uniforms_Texture.begin(); it != MaterialProgram->Uniforms_Texture.end(); it++)
	for (auto& uniformitempair : MaterialProgram->Uniforms_Texture)
	{
		auto& uniformitem = uniformitempair.second;
		Int32 Location      = uniformitem.Location;
		void* IDPtr       = uniformitem.IDPtr;
		glActiveTexture(GL_TEXTURE0 + Location);
		switch (uniformitem.DataType)
		{
		case UniformType::GLSL_TEXTURE2D:
			glBindTexture(GL_TEXTURE_2D, *((UInt32*)IDPtr));
			break;
		case UniformType::GLSL_TEXTURECUBE:
			glBindTexture(GL_TEXTURE_CUBE_MAP, *((UInt32*)IDPtr));
			break;
		case UniformType::GLSL_TEXTURE3D:
			glBindTexture(GL_TEXTURE_3D, *((UInt32*)IDPtr));
			break;
		case UniformType::GLSL_TEXTURE_2D_ARRAY:
			glBindTexture(GL_TEXTURE_2D_ARRAY, *((UInt32*)IDPtr));
			break;
		default:
			break;
		}
	}
}
void Material::Draw(UInt32 VAO, Int32 NumFaces, IndexSizeType indexSize, Int32 Offset, GLDrawType drawType)
{
	BindProgram();
	BindUniforms();
	BindSamplers();

	Int32 IndexSize = 0;
	Int32 GL_IndexType;
	switch (indexSize)
	{
	case Index16Bits:
		IndexSize = 2;
		GL_IndexType = GL_UNSIGNED_SHORT;
		break;
	case Index32Bits:
		IndexSize = 4;
		GL_IndexType = GL_UNSIGNED_INT;
		break;
	default:
		break;
	}
	glBindVertexArray(VAO);
	//glPolygonOffset
	switch (drawType)
	{
	case Unkown:
		break;
	case OGL_ELEMENT:
		glDrawElements(GL_TRIANGLES, NumFaces * 3, GL_IndexType, reinterpret_cast<void*>((Int64)Offset * IndexSize));
		break;
	case OGL_LINE_STRIP: glDrawElements(GL_LINE_STRIP, NumFaces * 3, GL_IndexType, reinterpret_cast<void*>((Int64)Offset * IndexSize));
		break;
	default:
		break;
	}
	glBindVertexArray(0);
}

void Material::LoadAndCreateShaders(std::vector<std::string>& shaderNames)
{
	if (!MaterialProgram)
	{
		MaterialProgram = std::shared_ptr<Program>(new Program);
	}
	MaterialProgram->shaders = std::vector<Shader>(shaderNames.size());

	for (UInt32 i = 0; i < shaderNames.size(); i++)
	{
		std::ifstream ShaderFile;
		ShaderFile.open(AssetFolderPath + shaderNames[i]);

		if (!ShaderFile)
		{
			std::cout << "Open Shader: " << shaderNames[i] << " Fail" << std::endl;
		}

		std::stringstream ShaderStream;
		ShaderStream << ShaderFile.rdbuf();
		std::string SourceCode = ShaderStream.str();

		Int32 loc =(Int32) shaderNames[i].find('.');
		const Int8 ShaderType = shaderNames[i][loc + 1];

		switch (ShaderType)
		{
		case 'v': { MaterialProgram->shaders[i].Id = CreateShaderGPUObjFromSrcCode(SourceCode, ShaderType::VertexShader);
			MaterialProgram->shaders[i].type = ShaderType::VertexShader;
			break; }
		case 'f': { MaterialProgram->shaders[i].Id = CreateShaderGPUObjFromSrcCode(SourceCode, ShaderType::FragmentShader);
			MaterialProgram->shaders[i].type = ShaderType::FragmentShader;
			break; }
		case 'g': { MaterialProgram->shaders[i].Id = CreateShaderGPUObjFromSrcCode(SourceCode, ShaderType::GeometryShader);
			MaterialProgram->shaders[i].type = ShaderType::GeometryShader;
			break; }
		case 'e': { MaterialProgram->shaders[i].Id = CreateShaderGPUObjFromSrcCode(SourceCode, ShaderType::TessEvaluationShader);
			MaterialProgram->shaders[i].type = ShaderType::TessEvaluationShader;
			break; }
		case 'c': {
			if (shaderNames[i][loc - 1] == 't') {
				MaterialProgram->shaders[i].Id = CreateShaderGPUObjFromSrcCode(SourceCode, ShaderType::TessControlShader);
				MaterialProgram->shaders[i].type = ShaderType::TessControlShader;
			}
			else {
				MaterialProgram->shaders[i].Id = CreateShaderGPUObjFromSrcCode(SourceCode, ShaderType::ComputeShader);
				MaterialProgram->shaders[i].type = ShaderType::ComputeShader;
			}
			break;
		}
		default:
			break;
		}
	}
}

void Material::FindShaderNames(std::vector<std::string>& shaderNames)
{
	for (UInt32 i = 0; i < shaderNames.size(); i++)
	{
		std::ifstream ShaderFile;
		ShaderFile.open(AssetFolderPath + shaderNames[i]);

		std::stringstream ShaderStream;
		ShaderStream << ShaderFile.rdbuf();
		std::string SourceCode = ShaderStream.str();

		MaterialProgram->shaders[i].Name = shaderNames[i];
	}
}

void Material::CreateProgram()
{
	MaterialProgram->Id = glCreateProgram();
	for (UInt32 i = 0; i < MaterialProgram->shaders.size(); i++)
	{
		glAttachShader(MaterialProgram->Id, MaterialProgram->shaders[i].Id);
	}
	glLinkProgram(MaterialProgram->Id);

	GLint glStatus;

	glGetProgramiv(MaterialProgram->Id, GL_LINK_STATUS, &glStatus);
	std::string default_infolog;
	std::string& infolog = default_infolog;
	if (!glStatus)
	{
		int32_t infoLogLength, charWriten;
		glGetProgramiv(MaterialProgram->Id, GL_INFO_LOG_LENGTH, &infoLogLength);
		infolog.resize(infoLogLength);
		if (infoLogLength)
		{
			glGetProgramInfoLog(MaterialProgram->Id, infoLogLength, &charWriten, &infolog[0]);
			std::cout << "Failed to link program with infolog" << infolog << std::endl;
		}
		std::cout << "Failed to link shader" << std::endl;
	}
}

void Material::FindAttibInfos()
{
	Int32 AttribCount;
	Int32 AttribName_MaxLength;
	glGetProgramiv(MaterialProgram->Id, GL_ACTIVE_ATTRIBUTES, &AttribCount);
	glGetProgramiv(MaterialProgram->Id, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &AttribName_MaxLength);
	Int8 * AttribName = new Int8[AttribName_MaxLength];
	for (Int32 AttribIndex = 0; AttribIndex < AttribCount; AttribIndex++)
	{
		AttribItem Attrib;
		Int32 NameLength;
		Int32 AttribSize;
		UInt32 AttribType;
		glGetActiveAttrib(MaterialProgram->Id, AttribIndex, AttribName_MaxLength, &NameLength, &AttribSize, &AttribType, AttribName);
		MaterialProgram->Attribs.insert(std::pair<std::string, AttribItem>(std::string(AttribName), Attrib));
	}
	delete[] AttribName;
}

void Material::FindUniformInfos()
{
	Int32 UniformBlockCount;
	glGetProgramiv(MaterialProgram->Id, GL_ACTIVE_UNIFORM_BLOCKS, &UniformBlockCount);
	if (UniformBlockCount > 0) {
		Int32 UniformBlockName_MaxLength;
		glGetProgramiv(MaterialProgram->Id, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &UniformBlockName_MaxLength);
		Int8* UniformBlockName = new Int8[UniformBlockName_MaxLength];
		for (Int32 UniformBlockIndex = 0; UniformBlockIndex < UniformBlockCount; UniformBlockIndex++)
		{
			Int32 NameLength;
			glGetActiveUniformBlockName(MaterialProgram->Id, UniformBlockIndex, UniformBlockName_MaxLength, &NameLength, UniformBlockName);

			Int32 UniformCountInBlock;
			glGetActiveUniformBlockiv(MaterialProgram->Id, UniformBlockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &UniformCountInBlock);
			Int32* InUniformIndices = new Int32[UniformCountInBlock];
			glGetActiveUniformBlockiv(MaterialProgram->Id, UniformBlockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, InUniformIndices);
			Int32 UniformInBlockName_MaxLength;
			glGetProgramiv(MaterialProgram->Id, GL_ACTIVE_UNIFORM_MAX_LENGTH, &UniformInBlockName_MaxLength);

			Int32 DataSize;
			glGetActiveUniformBlockiv(MaterialProgram->Id, UniformBlockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &DataSize);

			std::hash<string> hs;
			std::shared_ptr<UniformItem_Block> Block = std::shared_ptr<UniformItem_Block>(new UniformItem_Block());
			Block->HashCode = (Int32)hs(UniformBlockName);
			Block->DataSize_Byte = DataSize;
			Block->Index = UniformBlockIndex;
			MaterialProgram->Uniforms_Block.insert(std::pair<std::string, std::shared_ptr<UniformItem_Block>>(UniformBlockName, Block));

			Int8* UniformInBlockName = new Int8[UniformInBlockName_MaxLength];
			for (Int32 Index = 0; Index < UniformCountInBlock; Index++)
			{
				UInt32 InUniformIndex = InUniformIndices[Index];

				Int32 NameLength;
				Int32 Size;
				UInt32 Type;
				Int32 Offset;
				glGetActiveUniform(MaterialProgram->Id, InUniformIndex, UniformInBlockName_MaxLength, &NameLength, &Size, &Type, UniformInBlockName);
				glGetActiveUniformsiv(MaterialProgram->Id, 1, &InUniformIndex, GL_UNIFORM_OFFSET, &Offset);

				UniformInBlockName[NameLength - 3] = Size > 1 ? '\0' : UniformInBlockName[NameLength - 3]; //Multi Size variable's name is "abcd[0]". We delete [0] here, and we can also get location with "abcd".

				UniformItem_WithinBlock UniformInBlock;
				UniformInBlock.Name = UniformInBlockName;
				UniformInBlock.DataType = UniformTypeMap[Type];
				UniformInBlock.Offset_Byte = Offset;
				UniformInBlock.Size = Size;
				Block->Uniforms.insert(std::pair<UInt32, UniformItem_WithinBlock>(UniformInBlock.Offset_Byte, UniformInBlock));
			}
			delete[] InUniformIndices;
		}
		delete[] UniformBlockName;
		
		std::unordered_map<std::string, std::shared_ptr<UniformItem_Block>>::iterator it;
		for(it = MaterialProgram->Uniforms_Block.begin(); it != MaterialProgram->Uniforms_Block.end(); it++)
		{
			it->second->Id = _GPUBuffers->CreateUniformBuffer(it->first, it->second);
			glBindBuffer(GL_UNIFORM_BUFFER, it->second->Id);
			glUniformBlockBinding(MaterialProgram->Id, it->second->Index, _GPUBuffers->GetUniformBlockBindingPoint(it->first));
			glBindBufferBase(GL_UNIFORM_BUFFER, _GPUBuffers->GetUniformBlockBindingPoint(it->first), it->second->Id);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);			
		}
	}

	Int32 UniformCount;
	glGetProgramiv(MaterialProgram->Id, GL_ACTIVE_UNIFORMS, &UniformCount);
	Int32 UniformName_MaxLength;
	glGetProgramiv(MaterialProgram->Id, GL_ACTIVE_UNIFORM_MAX_LENGTH, &UniformName_MaxLength);
	Int8* UniformName = new Int8[UniformName_MaxLength];
	for (Int32 UniformIndex = 0; UniformIndex < UniformCount; UniformIndex++)
	{
		Int32 NameLength;
		Int32 Size;
		UInt32 Type;
		glGetActiveUniform(MaterialProgram->Id, UniformIndex, UniformName_MaxLength, &NameLength, &Size, &Type, UniformName);

		UniformName[NameLength - 3] = Size > 1 ? '\0' : UniformName[NameLength - 3]; //Multi Size variable's name is "abcd[0]". We delete [0] here, and we can also get location with "abcd".

		if (-1 == glGetUniformLocation(MaterialProgram->Id, UniformName)) continue;	//Uniform is in uniform block	

		UniformType T = UniformTypeMap[Type];
		if (T < UniformType::GLSL_TEXTURE2D)
		{
			UniformItem_Basic UniformBasic;
			UniformBasic.Size = Size;
			UniformBasic.DataType = T;
			MaterialProgram->Uniforms_Basic.insert(std::pair<std::string, UniformItem_Basic>(UniformName, UniformBasic));
		}
		else
		{
			UniformItem_Texture UniformTexture;
			UniformTexture.DataType = T;
			MaterialProgram->Uniforms_Texture.insert(std::pair<std::string, UniformItem_Texture>(UniformName, UniformTexture));
		}
	}
	delete[] UniformName;
}

void Material::LinkLocation()
{
	glUseProgram(MaterialProgram->Id);
	for (std::unordered_map<std::string, AttribItem>::iterator it = MaterialProgram->Attribs.begin(); it != MaterialProgram->Attribs.end(); it++)
	{
		std::string name = it->first;;
		GLint loc = glGetAttribLocation(MaterialProgram->Id, name.c_str());
		MaterialProgram->Attribs[name].Location = loc;
	}

	for (std::unordered_map<std::string, UniformItem_Basic>::iterator it = MaterialProgram->Uniforms_Basic.begin(); it != MaterialProgram->Uniforms_Basic.end(); it++)
	{
		std::string name = it->first;
		GLint loc = glGetUniformLocation(MaterialProgram->Id, name.c_str());
		MaterialProgram->Uniforms_Basic[name].Location = loc;
	}

	Int32 newLoc;
	std::unordered_map<std::string, UniformItem_Texture>::iterator it;
	for (it = MaterialProgram->Uniforms_Texture.begin(), newLoc = 0; it != MaterialProgram->Uniforms_Texture.end(); it++, newLoc++)
	{
		std::string name = it->first;
		GLint loc = glGetUniformLocation(MaterialProgram->Id, name.c_str());
		if (loc == -1) continue;
		glUniform1i(loc, newLoc);
		MaterialProgram->Uniforms_Texture[name].Location = newLoc;
	}

	glUseProgram(0);
}

UInt32 Material::CreateShaderGPUObjFromSrcCode(std::string & srcCode, ShaderType type)
{
	UInt32 outShader;

	switch (type)
	{
	case ShaderType::VertexShader:
		outShader = glCreateShader(GL_VERTEX_SHADER);
		break;
	case ShaderType::FragmentShader:
		outShader = glCreateShader(GL_FRAGMENT_SHADER);
		break;
	case ShaderType::ComputeShader:
		outShader = glCreateShader(GL_COMPUTE_SHADER);
		break;
	case ShaderType::GeometryShader:
		outShader = glCreateShader(GL_GEOMETRY_SHADER);
		break;
	case ShaderType::TessControlShader:
		outShader = glCreateShader(GL_TESS_CONTROL_SHADER);
		break;
	case ShaderType::TessEvaluationShader:
		outShader = glCreateShader(GL_TESS_EVALUATION_SHADER);
		break;
	default:
		std::cout << "loadShader: Unknown shader type requested." << std::endl;
	}

	const Int8* vShaderCode = srcCode.c_str();
	glShaderSource(outShader, 1, &vShaderCode, NULL);
	glCompileShader(outShader);

	GLint success;
	GLchar infoLog[1024];
	glGetShaderiv(outShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(outShader, 1024, NULL, infoLog);
		std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
	}

	return outShader;
}