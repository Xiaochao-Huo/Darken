#pragma once
#include "MaterialInstance.h"
#include "Camera.h"
#include <glew.h>
#include <map>

#define MODEL_UNIFORM_BLOCK_NAME "Model"
#define MODEL_UNIFORM_BLOCK_BINDING_POINT 0

#define VIEW_UNIFORM_BLOCK_NAME "View"
#define VIEW_UNIFORM_BLOCK_BINDING_POINT (Int32)1

#define SHADOWMAPPING_UNIFORM_BLOCK_NAME "Shadow"
#define SHADOWMAPPING_UNIFORM_BLOCK_BIDING_POINT (Int32)2

#define LIGHT_UNIFORM_BLOCK_NAME "LightBuffer"
#define LIGHT_UNIFORM_BLOCK_BINDING_POINT (Int32)3

#define UNIFORM_BLOCK_BINGDING_POINT_COMMEN_BEGIN (Int32)4

struct ShadowData
{
	Mat4f LightSpaceVPMatrix;
	Vector4f ShadowBufferSize;
	Float32 SoftTransitionScale;
	Vector2f ProjectionDepthBiasParameters;
	Vector4f LightCamera_ZBufferParams;
	Int32 bDirectLight;
};

struct LightData
{
	Vector3f LightPosition;
	Float32 LightInvRadius;
	Vector3f LightColor;
	Float32 LightFallofExponent;
	Vector3f NormalizedLightDirection;
	Vector3f NormalizedLightTangent;
	Vector2f LightSpotAngles;
	Float32 LightSourceRadius;
	Float32 LightSourceLength;
	Float32 LightSoftSourceRadius;
	Float32 LightSpecularScale;
	Float32 LightContactShadowLength;
	Vector2f LightDistanceFadeMAD;
	Vector4f LightShadowMapChannelMask;
	Int32 LightShadowedBits;
	Int32 LightType;
	Int32 bLastLight;
};

class BufferManager
{
public:
	BufferManager();
	~BufferManager();
	
	Int32 GetUniformBlockBindingPoint(const std::string & BlockName);
	Int32 CreateUniformBuffer(const std::string& BufferName, std::shared_ptr<UniformItem_Block> UniformBlockInfo);
	void MarkBufferDirty(Int32 BufferNameID);
	void UpdateViewBuffer(Camera * camera); //View uniform buffer need update befer Render main view port
	void UpdateModelBuffer(const Mat4f &ModelMatrix,
							const Mat4f &Model_ITMatrix,
							const Mat4f &Model_PreMatrix,
							const Mat4f &Model_ITPreMatrix); //Model uniform buffer need update before per draw
	void UpdateShadowBuffer(ShadowData &shadowBuffer);  //Shadow uniform buffer need update before draw next light

	void UpdateLightBuffer(LightData &lightBuffer); //Light uniform buffer need update before draw next light
							
	
	void UpdateCustomBufferData();

private:
	std::multimap<Int32, std::shared_ptr<UniformItem_Block>> UniformBufferNameID_InfoPtrMap;
	std::multimap<std::string, Int32> UniformBufferName_GPUIDMap;
	std::map<Int32, Bool> UniformBufferNameID_DirtyMap;

	Int32 MODEL_UNIFORM_BLOCK_NAME_ID;
	Int32 VIEW_UNIFORM_BLOCK_NAME_ID;
	Int32 SHADOWMAPPING_UNIFORM_BLOCK_NAME_ID;
	Int32 LIGHT_UNIFORM_BLOCK_NAME_ID;
};

