#include "BufferManager.h"
#include "GlobalPram.h"
#include <iostream>

BufferManager::BufferManager()
{
	hash<std::string>hs;
	MODEL_UNIFORM_BLOCK_NAME_ID = hs(MODEL_UNIFORM_BLOCK_NAME);
	VIEW_UNIFORM_BLOCK_NAME_ID = hs(VIEW_UNIFORM_BLOCK_NAME);
	SHADOWMAPPING_UNIFORM_BLOCK_NAME_ID = hs(SHADOWMAPPING_UNIFORM_BLOCK_NAME);
	LIGHT_UNIFORM_BLOCK_NAME_ID = hs(LIGHT_UNIFORM_BLOCK_NAME);
}

BufferManager::~BufferManager()
{
}

int BufferManager::GetUniformBlockBindingPoint(const std::string & BlockName)
{
	if (BlockName == VIEW_UNIFORM_BLOCK_NAME) return VIEW_UNIFORM_BLOCK_BINDING_POINT;
	else if (BlockName == LIGHT_UNIFORM_BLOCK_NAME) return LIGHT_UNIFORM_BLOCK_BINDING_POINT;
	else if (BlockName == MODEL_UNIFORM_BLOCK_NAME) return MODEL_UNIFORM_BLOCK_BINDING_POINT;
	else if (BlockName == SHADOWMAPPING_UNIFORM_BLOCK_NAME) return SHADOWMAPPING_UNIFORM_BLOCK_BIDING_POINT;
	else return UNIFORM_BLOCK_BINGDING_POINT_COMMEN_BEGIN + UniformBufferNameID_InfoPtrMap.size();
}

int BufferManager::CreateUniformBuffer(const std::string& BufferName, std::shared_ptr<UniformItem_Block> UniformBlockInfo)
{
	if(BufferName == VIEW_UNIFORM_BLOCK_NAME && UniformBufferName_GPUIDMap.find(VIEW_UNIFORM_BLOCK_NAME) != UniformBufferName_GPUIDMap.end())
	{
		return UniformBufferName_GPUIDMap.find(VIEW_UNIFORM_BLOCK_NAME)->second;
	}else if(BufferName == LIGHT_UNIFORM_BLOCK_NAME && UniformBufferName_GPUIDMap.find(LIGHT_UNIFORM_BLOCK_NAME) != UniformBufferName_GPUIDMap.end())
	{
		return UniformBufferName_GPUIDMap.find(LIGHT_UNIFORM_BLOCK_NAME)->second;
	}
	else if (BufferName == MODEL_UNIFORM_BLOCK_NAME && UniformBufferName_GPUIDMap.find(MODEL_UNIFORM_BLOCK_NAME) != UniformBufferName_GPUIDMap.end())
	{
		return UniformBufferName_GPUIDMap.find(MODEL_UNIFORM_BLOCK_NAME)->second;
	}
	else if (BufferName == SHADOWMAPPING_UNIFORM_BLOCK_NAME && UniformBufferName_GPUIDMap.find(SHADOWMAPPING_UNIFORM_BLOCK_NAME) != UniformBufferName_GPUIDMap.end())
	{
		return UniformBufferName_GPUIDMap.find(SHADOWMAPPING_UNIFORM_BLOCK_NAME)->second;
	}

	if(UniformBufferName_GPUIDMap.find(BufferName) != UniformBufferName_GPUIDMap.end())
	{
		std::cout << "Warning: Several shader all have Uniform Buffer: " << BufferName << ". It will influence efficiency." << std::endl;
	}
	   
	unsigned int UBO;
	glGenBuffers(1, &UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	glBufferData(GL_UNIFORM_BUFFER, UniformBlockInfo->DataSize_Byte, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	UniformBufferName_GPUIDMap.insert(std::pair<std::string, int>(BufferName, UBO));
	std::hash<std::string> hs;
	int BufferNameID = hs(BufferName);
	UniformBufferNameID_InfoPtrMap.insert(std::pair<int, std::shared_ptr<UniformItem_Block>>(BufferNameID, UniformBlockInfo));
	UniformBufferNameID_DirtyMap.insert(std::pair<int, bool>(BufferNameID, false));

	return UBO;
}

void BufferManager::MarkBufferDirty(int BufferNameID)
{
	for(std::multimap<int, bool>::iterator it = UniformBufferNameID_DirtyMap.begin(); it != UniformBufferNameID_DirtyMap.end(); it++)
	{
		if(it->first == BufferNameID)
		{
			it->second = true;
		}
	}
}

void BufferManager::UpdateCustomBufferData()
{
	for (std::multimap<int, bool>::iterator it = UniformBufferNameID_DirtyMap.begin(); it != UniformBufferNameID_DirtyMap.end(); it++)
	{
		if (it->second == true)
		{
			it->second = false;

			for(std::multimap<int, std::shared_ptr<UniformItem_Block>>::iterator it2 = UniformBufferNameID_InfoPtrMap.begin(); it2 != UniformBufferNameID_InfoPtrMap.end(); it2++)
			{
				if(it2->first == it->first)
				{
					UniformItem_Block* Block = it2->second.get();
					glBindBuffer(GL_UNIFORM_BUFFER, Block->Id);
					glBufferData(GL_UNIFORM_BUFFER, Block->DataSize_Byte, Block->DataPtr, GL_DYNAMIC_DRAW);
					glBindBuffer(GL_UNIFORM_BUFFER, 0);
				}
			}			
		}
	}
}

void BufferManager::UpdateModelBuffer(const glm::mat4 &ModelMatrix,
	const glm::mat4 &Model_ITMatrix,
	const glm::mat4 &Model_PreMatrix,
	const glm::mat4 &Model_ITPreMatrix)
{
	long long BlockDataPtr = (long long)UniformBufferNameID_InfoPtrMap.find(MODEL_UNIFORM_BLOCK_NAME_ID)->second->DataPtr;
	std::map<long long, UniformItem_WithinBlock>::iterator it = UniformBufferNameID_InfoPtrMap.find(MODEL_UNIFORM_BLOCK_NAME_ID)->second->Uniforms.begin();

	memcpy((void*)(BlockDataPtr + it->first), &ModelMatrix, sizeof(glm::mat4));				it++;
	memcpy((void*)(BlockDataPtr + it->first), &Model_ITMatrix, sizeof(glm::mat4));			it++;
	memcpy((void*)(BlockDataPtr + it->first), &Model_PreMatrix, sizeof(glm::mat4));			it++;
	memcpy((void*)(BlockDataPtr + it->first), &Model_ITPreMatrix, sizeof(glm::mat4));		it++;

	UniformBufferNameID_DirtyMap.find(MODEL_UNIFORM_BLOCK_NAME_ID)->second = false;
	UniformItem_Block* Block = UniformBufferNameID_InfoPtrMap.find(MODEL_UNIFORM_BLOCK_NAME_ID)->second.get();
	glBindBuffer(GL_UNIFORM_BUFFER, Block->Id);
	glBufferData(GL_UNIFORM_BUFFER, Block->DataSize_Byte, Block->DataPtr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void BufferManager::UpdateViewBuffer(Camera * camera)
{
	if (UniformBufferNameID_InfoPtrMap.empty()) return;
	glm::mat4 ViewMatrix = camera->GetViewMatrix();
	glm::mat4 ProjectMatrix = camera->GetProjectMatrix();
	glm::mat4 ViewMatrix_PreFrame = camera->GetViewMatrix_PreFrame();
	glm::mat4 ProjectMatrix_PreFrame = camera->GetProjectMatrix_PreFrame();
	glm::vec3 ViewPosition = camera->GetPosition();
	glm::vec2 ScreenSize = glm::vec2(camera->GetViewPortSize());

	long long BlockDataPtr = (long long) UniformBufferNameID_InfoPtrMap.find(VIEW_UNIFORM_BLOCK_NAME_ID)->second->DataPtr;
	std::map<long long, UniformItem_WithinBlock>::iterator it = UniformBufferNameID_InfoPtrMap.find(VIEW_UNIFORM_BLOCK_NAME_ID)->second->Uniforms.begin();

	memcpy((void*)(BlockDataPtr + it->first), &ViewMatrix, sizeof(glm::mat4));				it++;
	memcpy((void*)(BlockDataPtr + it->first), &ProjectMatrix, sizeof(glm::mat4));			it++;
	memcpy((void*)(BlockDataPtr + it->first), &ViewMatrix_PreFrame, sizeof(glm::mat4));		it++;
	memcpy((void*)(BlockDataPtr + it->first), &ProjectMatrix_PreFrame, sizeof(glm::mat4));	it++;
	memcpy((void*)(BlockDataPtr + it->first), &ViewPosition, sizeof(glm::vec3));			it++;
	memcpy((void*)(BlockDataPtr + it->first), &ScreenSize, sizeof(glm::vec2));				it++;

	UniformBufferNameID_DirtyMap.find(VIEW_UNIFORM_BLOCK_NAME_ID)->second = false;
	UniformItem_Block* Block = UniformBufferNameID_InfoPtrMap.find(VIEW_UNIFORM_BLOCK_NAME_ID)->second.get();
	glBindBuffer(GL_UNIFORM_BUFFER, Block->Id);
	glBufferData(GL_UNIFORM_BUFFER, Block->DataSize_Byte, Block->DataPtr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void BufferManager::UpdateShadowBuffer(ShadowData &shadowBuffer)
{
	long long BlockDataPtr = (long long)UniformBufferNameID_InfoPtrMap.find(SHADOWMAPPING_UNIFORM_BLOCK_NAME_ID)->second->DataPtr;
	std::map<long long, UniformItem_WithinBlock>::iterator it = UniformBufferNameID_InfoPtrMap.find(SHADOWMAPPING_UNIFORM_BLOCK_NAME_ID)->second->Uniforms.begin();

	memcpy((void*)(BlockDataPtr + it->first), &shadowBuffer.LightSpaceVPMatrix, sizeof(glm::mat4));					it++;
	memcpy((void*)(BlockDataPtr + it->first), &shadowBuffer.ShadowBufferSize, sizeof(glm::vec4));					it++;
	memcpy((void*)(BlockDataPtr + it->first), &shadowBuffer.SoftTransitionScale, sizeof(float));						it++;
	memcpy((void*)(BlockDataPtr + it->first), &shadowBuffer.ProjectionDepthBiasParameters, sizeof(glm::vec2));		it++;
	memcpy((void*)(BlockDataPtr + it->first), &shadowBuffer.LightCamera_ZBufferParams, sizeof(glm::vec4));			it++;
	memcpy((void*)(BlockDataPtr + it->first), &shadowBuffer.bDirectLight, sizeof(int));								it++;

	UniformBufferNameID_DirtyMap.find(SHADOWMAPPING_UNIFORM_BLOCK_NAME_ID)->second = false;
	UniformItem_Block* Block = UniformBufferNameID_InfoPtrMap.find(SHADOWMAPPING_UNIFORM_BLOCK_NAME_ID)->second.get();
	glBindBuffer(GL_UNIFORM_BUFFER, Block->Id);
	glBufferData(GL_UNIFORM_BUFFER, Block->DataSize_Byte, Block->DataPtr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void BufferManager::UpdateLightBuffer(LightData &lightBuffer)
{
	long long BlockDataPtr = (long long)UniformBufferNameID_InfoPtrMap.find(LIGHT_UNIFORM_BLOCK_NAME_ID)->second->DataPtr;
	std::map<long long, UniformItem_WithinBlock>::iterator it = UniformBufferNameID_InfoPtrMap.find(LIGHT_UNIFORM_BLOCK_NAME_ID)->second->Uniforms.begin();

	memcpy((void*)(BlockDataPtr + it->first), &lightBuffer.LightPosition, sizeof(glm::vec3));				it++;
	memcpy((void*)(BlockDataPtr + it->first), &lightBuffer.LightInvRadius, sizeof(float));					it++;
	memcpy((void*)(BlockDataPtr + it->first), &lightBuffer.LightColor, sizeof(glm::vec3));					it++;
	memcpy((void*)(BlockDataPtr + it->first), &lightBuffer.LightFallofExponent, sizeof(float));				it++;
	memcpy((void*)(BlockDataPtr + it->first), &lightBuffer.NormalizedLightDirection, sizeof(glm::vec3));	it++;
	memcpy((void*)(BlockDataPtr + it->first), &lightBuffer.NormalizedLightTangent, sizeof(glm::vec3));		it++;
	memcpy((void*)(BlockDataPtr + it->first), &lightBuffer.LightSpotAngles, sizeof(glm::vec2));				it++;
	memcpy((void*)(BlockDataPtr + it->first), &lightBuffer.LightSourceRadius, sizeof(float));				it++;
	memcpy((void*)(BlockDataPtr + it->first), &lightBuffer.LightSourceLength, sizeof(float));				it++;
	memcpy((void*)(BlockDataPtr + it->first), &lightBuffer.LightSoftSourceRadius, sizeof(float));			it++;
	memcpy((void*)(BlockDataPtr + it->first), &lightBuffer.LightSpecularScale, sizeof(float));				it++;
	memcpy((void*)(BlockDataPtr + it->first), &lightBuffer.LightContactShadowLength, sizeof(float));		it++;
	memcpy((void*)(BlockDataPtr + it->first), &lightBuffer.LightDistanceFadeMAD, sizeof(glm::vec2));		it++;
	memcpy((void*)(BlockDataPtr + it->first), &lightBuffer.LightShadowMapChannelMask, sizeof(glm::vec4));	it++;
	memcpy((void*)(BlockDataPtr + it->first), &lightBuffer.LightShadowedBits, sizeof(int));					it++;
	memcpy((void*)(BlockDataPtr + it->first), &lightBuffer.LightType, sizeof(int));							it++;
	memcpy((void*)(BlockDataPtr + it->first), &lightBuffer.bLastLight, sizeof(int));						it++;

	UniformBufferNameID_DirtyMap.find(LIGHT_UNIFORM_BLOCK_NAME_ID)->second = false;
	UniformItem_Block* Block = UniformBufferNameID_InfoPtrMap.find(LIGHT_UNIFORM_BLOCK_NAME_ID)->second.get();
	glBindBuffer(GL_UNIFORM_BUFFER, Block->Id);
	glBufferData(GL_UNIFORM_BUFFER, Block->DataSize_Byte, Block->DataPtr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}