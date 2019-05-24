#pragma once

#include "MaterialInstance.h"
#include "Light.h"
#include "Object.h"
#include "RectBufferObject.h"
#include <vector>

class ShadowDepth;
class Lighting;
class ReflectionEnvironment;
class SubSurfaceShading;
class UE4TemporalAA;
class ToneMapping;

class DeferRenderPipeline
{
public:
	DeferRenderPipeline();
	~DeferRenderPipeline();
	void RenderShadowDepthPass(unsigned int typeFlags);
	void RenderLightingPass(unsigned int typeFlags);
	void RenderSSSPass();
	void ExecuteTemporalAA();
	void ExecuteToneMapping();

	void Init();

	//ShadowMapping
	std::shared_ptr<ShadowDepth> ShadowMappingPass;
	//LightingPass
	std::shared_ptr<Lighting> LightingPass;
	//ReflectionEnvironment
	std::shared_ptr<ReflectionEnvironment> ReflectionPass;
	//SubSurfaceShadingPass
	std::shared_ptr<SubSurfaceShading> SSSPass;
	//TemporalAA
	std::shared_ptr<UE4TemporalAA> TAAPass;
	//ToneMapping
	std::shared_ptr<ToneMapping> ToneMappingPass;

private:
	glm::mat4 PPModelMatrix;
	glm::mat4 PPViewMatrix;
	glm::mat4 PPProjectMatrix;
	std::shared_ptr<RectBufferObject> PPObj;

	void SortSceneLights();
	std::vector<Light*> Lights;
};

struct ShadowDepthMaterialDataIDs
{
	int LightSpaceVPMatrixID;
	int LightCamera_ZBufferParamsID;
	int bDirectLightID;
	
	ShadowDepthMaterialDataIDs()
	{
		LightSpaceVPMatrixID = MaterialInstance::GetID("LightSpaceVPMatrix");
		LightCamera_ZBufferParamsID = MaterialInstance::GetID("LightCamera_ZBufferParams");
		bDirectLightID = MaterialInstance::GetID("bDirectLight");
	}
};

class ShadowDepth
{
public:
	ShadowDepth(std::vector<Light*> lights);
	~ShadowDepth();
	std::vector<unsigned int> ShadowDepth_Texs2D;
	std::vector<unsigned int> ShadowDepth_TexsCube;
	std::vector<Light*> Lights;
	std::shared_ptr<MaterialInstance> ShadowDepthMaterialInst;
	int ShadowDepthTexWidth;
	int ShadowDepthTexHeight;

	void Render(unsigned typeFlags);
private:
	ShadowDepthMaterialDataIDs MaterialDataIDs;
	std::vector<unsigned int> ShadowDepthFrameBuffers;
	std::vector<unsigned int> ShadowDepthRenderBuffers;

	void CreateShadowDepthResources();
	void CalculateLightsVPMatrix();
	void RenderDirectLightDepth(int LightIndex, const std::vector<std::shared_ptr<Object>> &Objects);
	void RenderPointLightDepth(int LightIndex, const std::vector<std::shared_ptr<Object>> &Objects);
	void RenderSpotLightDepth(int LightIndex, const std::vector<std::shared_ptr<Object>> &Objects);
};

struct LightingMaterialDataIDs
{
	int ShadowDepth_Tex2DID;
	int ShadowDepth_TexCubeID;
	int LightSpaceVPMatrixID;
	int PointLightSpaceVPMatricesID;
	int LightDirWorldID;
	int LightColorID;
	int LightCamera_ZBufferParamsID;
	int bDirectLightID;
	int ShadowBufferSizeID;
	int ShadowDepthSoftTransitionScaleID;
	int ProjectionDepthBiasParametersID;
	int bLastLightID;

	LightingMaterialDataIDs()
	{
		ShadowDepth_Tex2DID = MaterialInstance::GetID("ShadowDepth_Tex2D");
		ShadowDepth_TexCubeID = MaterialInstance::GetID("ShadowDepth_TexCube");
		LightSpaceVPMatrixID = MaterialInstance::GetID("LightSpaceVPMatrix");
		PointLightSpaceVPMatricesID = MaterialInstance::GetID("PointLightSpaceVPMatrices");
		LightDirWorldID = MaterialInstance::GetID("LightDir");
		LightColorID = MaterialInstance::GetID("LightColor");
		LightCamera_ZBufferParamsID = MaterialInstance::GetID("LightCamera_ZBufferParams");
		bDirectLightID = MaterialInstance::GetID("bDirectLight");
		ShadowBufferSizeID = MaterialInstance::GetID("ShadowBufferSize");
		ShadowDepthSoftTransitionScaleID = MaterialInstance::GetID("SoftTransitionScale");
		ProjectionDepthBiasParametersID = MaterialInstance::GetID("ProjectionDepthBiasParameters");
		bLastLightID = MaterialInstance::GetID("bLastLight");
	}
};

class Lighting
{
public:
	Lighting(std::vector<Light*> lights);
	~Lighting();

	std::vector<Light*> Lights;
	void Render(unsigned typeFlags);
	std::shared_ptr<MaterialInstance> LightingPassMaterialInst;
	unsigned int ScreenDepthZ_Tex;
	unsigned int Lighting_Tex;
	unsigned int Velocity_Tex;
	unsigned int BaseColor_Tex;
	unsigned int CustomData_Tex;
	
	std::shared_ptr<ShadowDepth> ShadowMappingPass;
	std::shared_ptr<ReflectionEnvironment> ReflectEnvPass; //Only active at begining

private:
	unsigned int LightingPassFrameBuffer;
	unsigned int LightingPass_RBO;
	//void CreateLightingPassMaterial();
	void CreateLightingPassResources();
	
	std::shared_ptr<LightingMaterialDataIDs> MaterialDataIDs;
};

class ReflectionEnvironment
{
public:
	ReflectionEnvironment();
	~ReflectionEnvironment();

private:
	void CreateReflectionEnvResources();
};


#define SSSS_KERNEL0_OFFSET						0
#define SSSS_KERNEL0_SIZE						13
#define SSSS_KERNEL1_OFFSET						(SSSS_KERNEL0_OFFSET + SSSS_KERNEL0_SIZE)
#define SSSS_KERNEL1_SIZE						9
#define SSSS_KERNEL2_OFFSET						(SSSS_KERNEL1_OFFSET + SSSS_KERNEL1_SIZE)
#define SSSS_KERNEL2_SIZE						6
#define SSSS_KERNEL_TOTAL_SIZE					(SSSS_KERNEL0_SIZE + SSSS_KERNEL1_SIZE + SSSS_KERNEL2_SIZE)
#define SSSS_KERNEL_TOTAL_SIZE_WITH_PADDING		32
#define SSSS_TRANSMISSION_PROFILE_SIZE			32
#define	SSSS_MAX_TRANSMISSION_PROFILE_DISTANCE	5.0f // See MaxTransmissionProfileDistance in ComputeTransmissionProfile(), SeparableSSS.cpp
#define	SSSS_MAX_DUAL_SPECULAR_ROUGHNESS		2.0f
#define eye_blend_index  4
#define skin_profile_index  3
#define eye_refractive_index  2

class SubSurfaceShading
{
public:
	SubSurfaceShading();
	~SubSurfaceShading();

	void ComputeTransmissionProfile(glm::vec4* TargetBuffer, glm::uint32 TargetBufferSize, glm::vec4 SubsurfaceColor, glm::vec4 FalloffColor, float ExtinctionScale);
	void ComputeTransmissionProfile(int index, glm::vec4* TargetBuffer, glm::uint32 TargetBufferSize);

	glm::vec4 GetSSSS_DUAL_SPECULAR_Params(int index);
	glm::vec4 GetSSSS_TRANSMISSION_Params(int index);

	std::shared_ptr<MaterialInstance> SSSSetupMaterialInst;
	std::shared_ptr<MaterialInstance> SSSScateringMaterialInst;
	std::shared_ptr<MaterialInstance> SSSRecombineMaterialInst;

	unsigned int SSSRender_Tex;
	unsigned int SSSSetup_TexOut;
	unsigned int SSSScatering_TexOut1;
	unsigned int SSSScatering_TexOut2;
	unsigned int SSSRecombine_TexOut;
	void Render(unsigned int VAO, int NumFaces, IndexSizeType indexType = IndexSizeType::Index16Bits);

private:
	struct FSubsurfaceProfileStruct
	{
		// defaults from SeparableSSS.h and the demo
		float ScatterRadius = 0.8f;
		glm::vec4 SubsurfaceColor = glm::vec4(0.80f, 0.78f, 0.748f, 1.0);
		glm::vec4 FalloffColor = glm::vec4(0.735f, 0.3083f, 0.30129f, 1.0);
		glm::vec4 BoundaryColorBleed = glm::vec4(0.735f, 0.5279f, 0.5245f, 1.0);;
		float ExtinctionScale = 1.0f;
		float ScatteringDistribution = 0.93f;
		float NormalScale = 0.08f;
		float IOR = 1.55f;
		float Roughness0 = 0.75f;
		float Roughness1 = 1.30f;
		float LobeMix = 0.85f;
	};
	FSubsurfaceProfileStruct SubsurfaceProfileEntries[5];

	void InitSubsurfaceProfileEntries();
	void InitSSSSProfilekernel(int index);

	void ComputeMirroredSSSKernel(int index, glm::uint32 startPos, glm::uint32 TargetBufferSize, glm::vec4 SubsurfaceColor, glm::vec4 FalloffColor);

	static glm::vec3 SeparableSSS_Gaussian(float variance, float r, glm::vec4 FalloffColor);
	static glm::vec3 SeparableSSS_Profile(float r, glm::vec4 FalloffColor);

	//std::vector<glm::vec4> SSSSMirroredProfilekernel;
	std::unordered_map<int, std::vector<glm::vec4> > SSSSMirroredProfilekernelMap;
	static const int QualityIndex = 3;
	static const glm::int32 SUBSURFACE_KERNEL_SIZE = 3;

	void CreateResources();
	void InitSSSSProfilekernelParams(std::string const & _prefix, int index);
	unsigned int SSSFrameBuffer;
};

struct TemporalAAPixelUniformData
{
	int JitterID;
	int PlusWeights_0ID;
	int PlusWeights_1ID;
	TemporalAAPixelUniformData()
	{
		JitterID = MaterialInstance::GetID("JitterUV"); 
		PlusWeights_0ID = MaterialInstance::GetID("PlusWeights_0");
		PlusWeights_1ID = MaterialInstance::GetID("PlusWeights_1");
	}
};

class UE4TemporalAA
{
public:
	UE4TemporalAA();
	~UE4TemporalAA();
	void UpdateJitter();
	void RemoveJitter();

	std::shared_ptr<MaterialInstance> TAAPassMaterialInst;

	unsigned int TAAToScreenFrame_Tex;
	unsigned int TAAHistoryFrame_Tex;
	unsigned int TAAFrameBuffer;

	void Execute(unsigned int VAO, int NumFaces, IndexSizeType indexType = IndexSizeType::Index16Bits);
private:
	std::shared_ptr<TemporalAAPixelUniformData> TAAPixelUniformData;

	std::vector<glm::vec2> FrustumJitter;
	int ActiveJitterIndex;
	glm::vec4 ActiveJitterSample;
	glm::mat4 JitterProjectMatrix;
	glm::mat4 JitterProjectMatrix_PreFrame;
	void InitUE4SampleType2();
	void InitUE4SampleType4();

	void HackUpdateCameraProjectMatrix(float sampleOffsetX, float sampleOffsetY);
	void HackRemoveCameraProjectMatrix(float sampleOffsetX, float sampleOffsetY);
	void UpdateCameraProjectMatrix(float sampleOffsetX, float sampleOffsetY);

private:
	void CreateTAAPassMaterial();
	void CreateTAAPassResources();
};

struct ToneMappingPixelShaderUniformData
{
	int GrainRandomFullID;
	ToneMappingPixelShaderUniformData()
	{
		GrainRandomFullID = MaterialInstance::GetID("GrainRandomFull");
	}
};

class ToneMapping
{
public:
	ToneMapping();
	~ToneMapping();

	std::shared_ptr<MaterialInstance> ToneMappingMaterialInst;
	void Execute(unsigned int VAO, int NumFaces, IndexSizeType indexType = IndexSizeType::Index16Bits);

	unsigned int LUT_TEX;
	void GenerateLUTTexture(std::shared_ptr<RectBufferObject> pPObj);

private:
	void CreateToneMappingPassMaterial();
	std::shared_ptr<ToneMappingPixelShaderUniformData> ToneMappingUniformDatas;
	glm::vec2 GrainRandomFromFrame(int FrameCountMode8);
};




