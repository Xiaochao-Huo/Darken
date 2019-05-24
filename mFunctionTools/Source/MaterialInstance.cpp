#include "MaterialInstance.h"
#include "GlobalPram.h"

MaterialInstance::MaterialInstance(std::shared_ptr<Material> parentMaterial)
{
	SetParent(parentMaterial);
}

MaterialInstance::~MaterialInstance()
{
	for (std::map<int, void*>::iterator it = BasicUniformID_PtrMap.begin(); it != BasicUniformID_PtrMap.end(); it++)
	{
		free(it->second);
	}

	for (std::map<int, unsigned int*>::iterator it = TextureUniformID_PtrMap.begin(); it != TextureUniformID_PtrMap.end(); it++)
	{
		delete it->second;
	}
	
	for (std::unordered_map<std::string, std::shared_ptr<UniformItem_Block>>::iterator ItBlock = ParentMaterial->MaterialProgram->Uniforms_Block.begin(); ItBlock != ParentMaterial->MaterialProgram->Uniforms_Block.end(); ItBlock++)
	{
		free(ItBlock->second->DataPtr);
	}
}

void MaterialInstance::SetParent(std::shared_ptr<Material> parentMaterial)
{
	ParentMaterial = parentMaterial;

	void* BlockData;
	for(std::unordered_map<std::string, std::shared_ptr<UniformItem_Block>>::iterator ItBlock = ParentMaterial->MaterialProgram->Uniforms_Block.begin(); ItBlock != ParentMaterial->MaterialProgram->Uniforms_Block.end(); ItBlock++)
	{
		BlockData = malloc(ItBlock->second->DataSize_Byte);
		ItBlock->second->DataPtr = BlockData;
		int BlockID = hs(ItBlock->first);
		std::map<int, void*> UniformID_PtrMap;
		for(std::map<long long, UniformItem_WithinBlock>::iterator ItUniform = ItBlock->second->Uniforms.begin(); ItUniform != ItBlock->second->Uniforms.end(); ItUniform++)
		{
			int UniformID = hs(ItUniform->second.Name);
			void* UniformPtr = (void*) ((long long)BlockData + ItUniform->second.Offset_Byte);
			UniformID_PtrMap.insert(std::pair<int, void*>(UniformID, UniformPtr));
		}
		BlockID_UniformID_DataPtrMap.insert(std::pair<int, std::map<int, void*>>(BlockID, UniformID_PtrMap));
	}

	void * Data;
	for (std::unordered_map<std::string, UniformItem_Basic>::iterator it = ParentMaterial->MaterialProgram->Uniforms_Basic.begin(); it != ParentMaterial->MaterialProgram->Uniforms_Basic.end(); it++)
	{
		int ID = hs(it->first);
		switch (it->second.DataType)
		{
		case GLSL_INT: Data = malloc(sizeof(int) * it->second.Size); break;
		case GLSL_FLOAT: Data = malloc(sizeof(float) * it->second.Size); break;
		case GLSL_VEC2: Data = malloc(sizeof(glm::vec2) * it->second.Size);	break;
		case GLSL_VEC3:	Data = malloc(sizeof(glm::vec3) * it->second.Size);	break;
		case GLSL_VEC4:	Data = malloc(sizeof(glm::vec4) * it->second.Size);	break;
		case GLSL_IVEC2: Data = malloc(sizeof(glm::ivec2) * it->second.Size); break;
		case GLSL_IVEC3: Data = malloc(sizeof(glm::ivec3) * it->second.Size); break;
		case GLSL_IVEC4: Data = malloc(sizeof(glm::ivec4) * it->second.Size); break;
		case GLSL_MAT3:	Data = malloc(sizeof(glm::mat3) * it->second.Size);	break;
		case GLSL_MAT4: Data = malloc(sizeof(glm::mat4) * it->second.Size); break;
		default:
			Data = nullptr;
			break;
		}
		it->second.DataPtr = (void*)Data;
		BasicUniformID_PtrMap.insert(std::pair<int, void*>(ID, it->second.DataPtr));
	}
	for (std::unordered_map<std::string, UniformItem_Texture>::iterator it = ParentMaterial->MaterialProgram->Uniforms_Texture.begin(); it != ParentMaterial->MaterialProgram->Uniforms_Texture.end(); it++)
	{
		int ID = hs(it->first);
		unsigned int * textureID = new unsigned int;
		it->second.IDPtr = textureID;
		TextureUniformID_PtrMap.insert(std::pair<int, unsigned int *>(ID, it->second.IDPtr));
	}
}

void MaterialInstance::ChangedParentDynamic(std::shared_ptr<Material> newParentMaterial)
{	 
	for(std::unordered_map<std::string, std::shared_ptr<UniformItem_Block>>::iterator it = newParentMaterial->MaterialProgram->Uniforms_Block.begin(); it != newParentMaterial->MaterialProgram->Uniforms_Block.end(); it++)
	{
		std::unordered_map<std::string, std::shared_ptr<UniformItem_Block>>::iterator it2 = ParentMaterial->MaterialProgram->Uniforms_Block.find(it->first);
		if(it2 == ParentMaterial->MaterialProgram->Uniforms_Block.end()) std::cout << "No " << it->first << " in new Shader." << std::endl;
		it->second->DataPtr = it2->second->DataPtr;
	}


	for (std::unordered_map<std::string, UniformItem_Basic>::iterator it = newParentMaterial->MaterialProgram->Uniforms_Basic.begin(); it != newParentMaterial->MaterialProgram->Uniforms_Basic.end(); it++)
	{
		std::unordered_map<std::string, UniformItem_Basic>::iterator it2 = ParentMaterial->MaterialProgram->Uniforms_Basic.find(it->first);
		if (it2 == ParentMaterial->MaterialProgram->Uniforms_Basic.end()) std::cout << "No " << it->first << " in new Shader." << std::endl;
		it->second.DataPtr = it2->second.DataPtr;
	}

	for (std::unordered_map<std::string,UniformItem_Texture>::iterator it = newParentMaterial->MaterialProgram->Uniforms_Texture.begin(); it != newParentMaterial->MaterialProgram->Uniforms_Texture.end(); it++)
	{
		std::unordered_map<std::string, UniformItem_Texture>::iterator it2 = ParentMaterial->MaterialProgram->Uniforms_Texture.find(it->first);
		if (it2 == ParentMaterial->MaterialProgram->Uniforms_Texture.end()) std::cout << "No " << it->first << " in new Shader." << std::endl;
		it->second.IDPtr = it2->second.IDPtr;
	}
	ParentMaterial = newParentMaterial;
}

std::shared_ptr<Material> MaterialInstance::GetParent()
{
	return ParentMaterial;
}

int MaterialInstance::GetID(const std::string& ParameterName)
{
	std::hash<std::string> hs;
	return hs(ParameterName);
}

void MaterialInstance::MarkDirty(int BlockID)
{
	_GPUBuffers->MarkBufferDirty(BlockID);
}

