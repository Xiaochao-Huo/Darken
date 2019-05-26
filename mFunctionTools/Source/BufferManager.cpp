#include "BufferManager.h"
#include "GlobalPram.h"
#include <iostream>

BufferManager::BufferManager()
{
	hash<std::string>hs;
	MODEL_UNIFORM_BLOCK_NAME_ID = (Int32) hs(MODEL_UNIFORM_BLOCK_NAME);
	VIEW_UNIFORM_BLOCK_NAME_ID = (Int32) hs(VIEW_UNIFORM_BLOCK_NAME);
	SHADOWMAPPING_UNIFORM_BLOCK_NAME_ID = (Int32)hs(SHADOWMAPPING_UNIFORM_BLOCK_NAME);
	LIGHT_UNIFORM_BLOCK_NAME_ID = (Int32)hs(LIGHT_UNIFORM_BLOCK_NAME);
}

BufferManager::~BufferManager()
{
}

Int32 BufferManager::GetUniformBlockBindingPoint(const std::string & BlockName)
{
	if (BlockName == VIEW_UNIFORM_BLOCK_NAME) return VIEW_UNIFORM_BLOCK_BINDING_POINT;
	else if (BlockName == LIGHT_UNIFORM_BLOCK_NAME) return LIGHT_UNIFORM_BLOCK_BINDING_POINT;
	else if (BlockName == MODEL_UNIFORM_BLOCK_NAME) return MODEL_UNIFORM_BLOCK_BINDING_POINT;
	else if (BlockName == SHADOWMAPPING_UNIFORM_BLOCK_NAME) return SHADOWMAPPING_UNIFORM_BLOCK_BIDING_POINT;
	else return UNIFORM_BLOCK_BINGDING_POINT_COMMEN_BEGIN + (Int32)UniformBufferNameID_InfoPtrMap.size();
}

Int32 BufferManager::CreateUniformBuffer(const std::string& BufferName, std::shared_ptr<UniformItem_Block> UniformBlockInfo)
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
	   
	UInt32 UBO;
	glGenBuffers(1, &UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	glBufferData(GL_UNIFORM_BUFFER, UniformBlockInfo->DataSize_Byte, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	UniformBufferName_GPUIDMap.insert(std::pair<std::string, Int32>(BufferName, UBO));
	std::hash<std::string> hs;
	Int32 BufferNameID = (Int32)hs(BufferName);
	UniformBufferNameID_InfoPtrMap.insert(std::pair<Int32, std::shared_ptr<UniformItem_Block>>(BufferNameID, UniformBlockInfo));
	UniformBufferNameID_DirtyMap.insert(std::pair<Int32, Bool>(BufferNameID, false));

	return UBO;
}

void BufferManager::MarkBufferDirty(Int32 BufferNameID)
{
	for(std::multimap<Int32, Bool>::iterator it = UniformBufferNameID_DirtyMap.begin(); it != UniformBufferNameID_DirtyMap.end(); it++)
	{
		if(it->first == BufferNameID)
		{
			it->second = true;
		}
	}
}

void BufferManager::UpdateCustomBufferData()
{
	for (std::multimap<Int32, Bool>::iterator it = UniformBufferNameID_DirtyMap.begin(); it != UniformBufferNameID_DirtyMap.end(); it++)
	{
		if (it->second == true)
		{
			it->second = false;

			for(std::multimap<Int32, std::shared_ptr<UniformItem_Block>>::iterator it2 = UniformBufferNameID_InfoPtrMap.begin(); it2 != UniformBufferNameID_InfoPtrMap.end(); it2++)
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

void BufferManager::UpdateModelBuffer(const Mat4f &ModelMatrix,
	const Mat4f &Model_ITMatrix,
	const Mat4f &Model_PreMatrix,
	const Mat4f &Model_ITPreMatrix)
{
	Address BlockDataPtr = (Address)UniformBufferNameID_InfoPtrMap.find(MODEL_UNIFORM_BLOCK_NAME_ID)->second->DataPtr;
	std::map<UInt32, UniformItem_WithinBlock>::iterator it = UniformBufferNameID_InfoPtrMap.find(MODEL_UNIFORM_BLOCK_NAME_ID)->second->Uniforms.begin();

	memcpy((void*)(BlockDataPtr + it->first), &ModelMatrix, sizeof(Mat4f));				it++;
	memcpy((void*)(BlockDataPtr + it->first), &Model_ITMatrix, sizeof(Mat4f));			it++;
	memcpy((void*)(BlockDataPtr + it->first), &Model_PreMatrix, sizeof(Mat4f));			it++;
	memcpy((void*)(BlockDataPtr + it->first), &Model_ITPreMatrix, sizeof(Mat4f));		it++;

	UniformBufferNameID_DirtyMap.find(MODEL_UNIFORM_BLOCK_NAME_ID)->second = false;
	UniformItem_Block* Block = UniformBufferNameID_InfoPtrMap.find(MODEL_UNIFORM_BLOCK_NAME_ID)->second.get();
	glBindBuffer(GL_UNIFORM_BUFFER, Block->Id);
	glBufferData(GL_UNIFORM_BUFFER, Block->DataSize_Byte, Block->DataPtr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void BufferManager::UpdateViewBuffer(Camera * camera)
{
	if (UniformBufferNameID_InfoPtrMap.empty()) return;
	Mat4f ViewMatrix = camera->GetViewMatrix();
	Mat4f ProjectMatrix = camera->GetProjectMatrix();
	Mat4f ViewMatrix_PreFrame = camera->GetViewMatrix_PreFrame();
	Mat4f ProjectMatrix_PreFrame = camera->GetProjectMatrix_PreFrame();
	Vector3f ViewPosition = camera->GetPosition();
	Vector2f ScreenSize = Vector2f(camera->GetViewPortSize());

	Address BlockDataPtr = (Address) UniformBufferNameID_InfoPtrMap.find(VIEW_UNIFORM_BLOCK_NAME_ID)->second->DataPtr;
	std::map<UInt32, UniformItem_WithinBlock>::iterator it = UniformBufferNameID_InfoPtrMap.find(VIEW_UNIFORM_BLOCK_NAME_ID)->second->Uniforms.begin();

	memcpy((void*)(BlockDataPtr + it->first), &ViewMatrix, sizeof(Mat4f));				it++;
	memcpy((void*)(BlockDataPtr + it->first), &ProjectMatrix, sizeof(Mat4f));			it++;
	memcpy((void*)(BlockDataPtr + it->first), &ViewMatrix_PreFrame, sizeof(Mat4f));		it++;
	memcpy((void*)(BlockDataPtr + it->first), &ProjectMatrix_PreFrame, sizeof(Mat4f));	it++;
	memcpy((void*)(BlockDataPtr + it->first), &ViewPosition, sizeof(Vector3f));			it++;
	memcpy((void*)(BlockDataPtr + it->first), &ScreenSize, sizeof(Vector2f));				it++;

	UniformBufferNameID_DirtyMap.find(VIEW_UNIFORM_BLOCK_NAME_ID)->second = false;
	UniformItem_Block* Block = UniformBufferNameID_InfoPtrMap.find(VIEW_UNIFORM_BLOCK_NAME_ID)->second.get();
	glBindBuffer(GL_UNIFORM_BUFFER, Block->Id);
	glBufferData(GL_UNIFORM_BUFFER, Block->DataSize_Byte, Block->DataPtr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void BufferManager::UpdateShadowBuffer(ShadowData &shadowBuffer)
{
	Address BlockDataPtr = (Address)UniformBufferNameID_InfoPtrMap.find(SHADOWMAPPING_UNIFORM_BLOCK_NAME_ID)->second->DataPtr;
	std::map<UInt32, UniformItem_WithinBlock>::iterator it = UniformBufferNameID_InfoPtrMap.find(SHADOWMAPPING_UNIFORM_BLOCK_NAME_ID)->second->Uniforms.begin();

	memcpy((void*)(BlockDataPtr + it->first), &shadowBuffer.LightSpaceVPMatrix, sizeof(Mat4f));					it++;
	memcpy((void*)(BlockDataPtr + it->first), &shadowBuffer.ShadowBufferSize, sizeof(Vector4f));					it++;
	memcpy((void*)(BlockDataPtr + it->first), &shadowBuffer.SoftTransitionScale, sizeof(Float32));						it++;
	memcpy((void*)(BlockDataPtr + it->first), &shadowBuffer.ProjectionDepthBiasParameters, sizeof(Vector2f));		it++;
	memcpy((void*)(BlockDataPtr + it->first), &shadowBuffer.LightCamera_ZBufferParams, sizeof(Vector4f));			it++;
	memcpy((void*)(BlockDataPtr + it->first), &shadowBuffer.bDirectLight, sizeof(Int32));								it++;

	UniformBufferNameID_DirtyMap.find(SHADOWMAPPING_UNIFORM_BLOCK_NAME_ID)->second = false;
	UniformItem_Block* Block = UniformBufferNameID_InfoPtrMap.find(SHADOWMAPPING_UNIFORM_BLOCK_NAME_ID)->second.get();
	glBindBuffer(GL_UNIFORM_BUFFER, Block->Id);
	glBufferData(GL_UNIFORM_BUFFER, Block->DataSize_Byte, Block->DataPtr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void BufferManager::UpdateLightBuffer(LightData &lightBuffer)
{
	Address BlockDataPtr = (Address)UniformBufferNameID_InfoPtrMap.find(LIGHT_UNIFORM_BLOCK_NAME_ID)->second->DataPtr;
	std::map<UInt32, UniformItem_WithinBlock>::iterator it = UniformBufferNameID_InfoPtrMap.find(LIGHT_UNIFORM_BLOCK_NAME_ID)->second->Uniforms.begin();

	memcpy((void*)(BlockDataPtr + it->first), &lightBuffer.LightPosition, sizeof(Vector3f));				it++;
	memcpy((void*)(BlockDataPtr + it->first), &lightBuffer.LightInvRadius, sizeof(Float32));					it++;
	memcpy((void*)(BlockDataPtr + it->first), &lightBuffer.LightColor, sizeof(Vector3f));					it++;
	memcpy((void*)(BlockDataPtr + it->first), &lightBuffer.LightFallofExponent, sizeof(Float32));				it++;
	memcpy((void*)(BlockDataPtr + it->first), &lightBuffer.NormalizedLightDirection, sizeof(Vector3f));	it++;
	memcpy((void*)(BlockDataPtr + it->first), &lightBuffer.NormalizedLightTangent, sizeof(Vector3f));		it++;
	memcpy((void*)(BlockDataPtr + it->first), &lightBuffer.LightSpotAngles, sizeof(Vector2f));				it++;
	memcpy((void*)(BlockDataPtr + it->first), &lightBuffer.LightSourceRadius, sizeof(Float32));				it++;
	memcpy((void*)(BlockDataPtr + it->first), &lightBuffer.LightSourceLength, sizeof(Float32));				it++;
	memcpy((void*)(BlockDataPtr + it->first), &lightBuffer.LightSoftSourceRadius, sizeof(Float32));			it++;
	memcpy((void*)(BlockDataPtr + it->first), &lightBuffer.LightSpecularScale, sizeof(Float32));				it++;
	memcpy((void*)(BlockDataPtr + it->first), &lightBuffer.LightContactShadowLength, sizeof(Float32));		it++;
	memcpy((void*)(BlockDataPtr + it->first), &lightBuffer.LightDistanceFadeMAD, sizeof(Vector2f));		it++;
	memcpy((void*)(BlockDataPtr + it->first), &lightBuffer.LightShadowMapChannelMask, sizeof(Vector4f));	it++;
	memcpy((void*)(BlockDataPtr + it->first), &lightBuffer.LightShadowedBits, sizeof(Int32));					it++;
	memcpy((void*)(BlockDataPtr + it->first), &lightBuffer.LightType, sizeof(Int32));							it++;
	memcpy((void*)(BlockDataPtr + it->first), &lightBuffer.bLastLight, sizeof(Int32));						it++;

	UniformBufferNameID_DirtyMap.find(LIGHT_UNIFORM_BLOCK_NAME_ID)->second = false;
	UniformItem_Block* Block = UniformBufferNameID_InfoPtrMap.find(LIGHT_UNIFORM_BLOCK_NAME_ID)->second.get();
	glBindBuffer(GL_UNIFORM_BUFFER, Block->Id);
	glBufferData(GL_UNIFORM_BUFFER, Block->DataSize_Byte, Block->DataPtr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}