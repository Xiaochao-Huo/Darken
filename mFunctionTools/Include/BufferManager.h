#pragma once
#include "MaterialInstance.h"
#include "Camera.h"
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <glew.h>
#include <map>

#define MODEL_UNIFORM_BLOCK_NAME "Model"
#define MODEL_UNIFORM_BLOCK_BINDING_POINT 0

#define VIEW_UNIFORM_BLOCK_NAME "View"
#define VIEW_UNIFORM_BLOCK_BINDING_POINT 1

#define SHADOWMAPPING_UNIFORM_BLOCK_NAME "Shadow"
#define SHADOWMAPPING_UNIFORM_BLOCK_BIDING_POINT 2

#define LIGHT_UNIFORM_BLOCK_NAME "LightBuffer"
#define LIGHT_UNIFORM_BLOCK_BINDING_POINT 3

#define UNIFORM_BLOCK_BINGDING_POINT_COMMEN_BEGIN 4

struct ShadowData
{
	glm::mat4 LightSpaceVPMatrix;
	glm::vec4 ShadowBufferSize;
	float SoftTransitionScale;
	glm::vec2 ProjectionDepthBiasParameters;
	glm::vec4 LightCamera_ZBufferParams;
	int bDirectLight;
};

struct LightData
{
	glm::vec3 LightPosition;
	float LightInvRadius;
	glm::vec3 LightColor;
	float LightFallofExponent;
	glm::vec3 NormalizedLightDirection;
	glm::vec3 NormalizedLightTangent;
	glm::vec2 LightSpotAngles;
	float LightSourceRadius;
	float LightSourceLength;
	float LightSoftSourceRadius;
	float LightSpecularScale;
	float LightContactShadowLength;
	glm::vec2 LightDistanceFadeMAD;
	glm::vec4 LightShadowMapChannelMask;
	int LightShadowedBits;
	int LightType;
	int bLastLight;
};

class BufferManager
{
public:
	BufferManager();
	~BufferManager();
	
	int GetUniformBlockBindingPoint(const std::string & BlockName);
	int CreateUniformBuffer(const std::string& BufferName, std::shared_ptr<UniformItem_Block> UniformBlockInfo);
	void MarkBufferDirty(int BufferNameID);
	void UpdateViewBuffer(Camera * camera); //View uniform buffer need update befer Render main view port
	void UpdateModelBuffer(const glm::mat4 &ModelMatrix,
							const glm::mat4 &Model_ITMatrix,
							const glm::mat4 &Model_PreMatrix,
							const glm::mat4 &Model_ITPreMatrix); //Model uniform buffer need update before per draw
	void UpdateShadowBuffer(ShadowData &shadowBuffer);  //Shadow uniform buffer need update before draw next light

	void UpdateLightBuffer(LightData &lightBuffer); //Light uniform buffer need update before draw next light
							
	
	void UpdateCustomBufferData();

private:
	std::multimap<int, std::shared_ptr<UniformItem_Block>> UniformBufferNameID_InfoPtrMap;
	std::multimap<std::string, int> UniformBufferName_GPUIDMap;
	std::map<int, bool> UniformBufferNameID_DirtyMap;

	int MODEL_UNIFORM_BLOCK_NAME_ID;
	int VIEW_UNIFORM_BLOCK_NAME_ID;
	int SHADOWMAPPING_UNIFORM_BLOCK_NAME_ID;
	int LIGHT_UNIFORM_BLOCK_NAME_ID;
};

