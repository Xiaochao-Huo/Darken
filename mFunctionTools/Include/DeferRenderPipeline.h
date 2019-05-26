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
	void RenderShadowDepthPass(UInt32 typeFlags);
	void RenderLightingPass(UInt32 typeFlags);
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
	Mat4f PPModelMatrix;
	Mat4f PPViewMatrix;
	Mat4f PPProjectMatrix;
	std::shared_ptr<RectBufferObject> PPObj;

	void SortSceneLights();
	std::vector<Light*> Lights;
};

struct ShadowDepthMaterialDataIDs
{
	Int32 LightSpaceVPMatrixID;
	Int32 LightCamera_ZBufferParamsID;
	Int32 bDirectLightID;
	
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
	std::vector<UInt32> ShadowDepth_Texs2D;
	std::vector<UInt32> ShadowDepth_TexsCube;
	std::vector<Light*> Lights;
	std::shared_ptr<MaterialInstance> ShadowDepthMaterialInst;
	Int32 ShadowDepthTexWidth;
	Int32 ShadowDepthTexHeight;

	void Render(unsigned typeFlags);
private:
	ShadowDepthMaterialDataIDs MaterialDataIDs;
	std::vector<UInt32> ShadowDepthFrameBuffers;
	std::vector<UInt32> ShadowDepthRenderBuffers;

	void CreateShadowDepthResources();
	void CalculateLightsVPMatrix();
	void RenderDirectLightDepth(Int32 LightIndex, const std::vector<std::shared_ptr<Object>> &Objects);
	void RenderPointLightDepth(Int32 LightIndex, const std::vector<std::shared_ptr<Object>> &Objects);
	void RenderSpotLightDepth(Int32 LightIndex, const std::vector<std::shared_ptr<Object>> &Objects);
};

struct LightingMaterialDataIDs
{
	Int32 ShadowDepth_Tex2DID;
	Int32 ShadowDepth_TexCubeID;
	Int32 LightSpaceVPMatrixID;
	Int32 PointLightSpaceVPMatricesID;
	Int32 LightDirWorldID;
	Int32 LightColorID;
	Int32 LightCamera_ZBufferParamsID;
	Int32 bDirectLightID;
	Int32 ShadowBufferSizeID;
	Int32 ShadowDepthSoftTransitionScaleID;
	Int32 ProjectionDepthBiasParametersID;
	Int32 bLastLightID;

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
	UInt32 ScreenDepthZ_Tex;
	UInt32 Lighting_Tex;
	UInt32 Velocity_Tex;
	UInt32 BaseColor_Tex;
	UInt32 CustomData_Tex;
	
	std::shared_ptr<ShadowDepth> ShadowMappingPass;
	std::shared_ptr<ReflectionEnvironment> ReflectEnvPass; //Only active at begining

private:
	UInt32 LightingPassFrameBuffer;
	UInt32 LightingPass_RBO;
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

	void ComputeTransmissionProfile(Vector4f* TargetBuffer, UInt32 TargetBufferSize, Vector4f SubsurfaceColor, Vector4f FalloffColor, Float32 ExtinctionScale);
	void ComputeTransmissionProfile(Int32 index, Vector4f* TargetBuffer, UInt32 TargetBufferSize);

	Vector4f GetSSSS_DUAL_SPECULAR_Params(Int32 index);
	Vector4f GetSSSS_TRANSMISSION_Params(Int32 index);

	std::shared_ptr<MaterialInstance> SSSSetupMaterialInst;
	std::shared_ptr<MaterialInstance> SSSScateringMaterialInst;
	std::shared_ptr<MaterialInstance> SSSRecombineMaterialInst;

	UInt32 SSSRender_Tex;
	UInt32 SSSSetup_TexOut;
	UInt32 SSSScatering_TexOut1;
	UInt32 SSSScatering_TexOut2;
	UInt32 SSSRecombine_TexOut;
	void Render(UInt32 VAO, Int32 NumFaces, IndexSizeType indexType = IndexSizeType::Index16Bits);

private:
	struct FSubsurfaceProfileStruct
	{
		// defaults from SeparableSSS.h and the demo
		Float32 ScatterRadius = 0.8f;
		Vector4f SubsurfaceColor = Vector4f(0.80f, 0.78f, 0.748f, 1.0);
		Vector4f FalloffColor = Vector4f(0.735f, 0.3083f, 0.30129f, 1.0);
		Vector4f BoundaryColorBleed = Vector4f(0.735f, 0.5279f, 0.5245f, 1.0);;
		Float32 ExtinctionScale = 1.0f;
		Float32 ScatteringDistribution = 0.93f;
		Float32 NormalScale = 0.08f;
		Float32 IOR = 1.55f;
		Float32 Roughness0 = 0.75f;
		Float32 Roughness1 = 1.30f;
		Float32 LobeMix = 0.85f;
	};
	FSubsurfaceProfileStruct SubsurfaceProfileEntries[5];

	void InitSubsurfaceProfileEntries();
	void InitSSSSProfilekernel(Int32 index);

	void ComputeMirroredSSSKernel(Int32 index, UInt32 startPos, UInt32 TargetBufferSize, Vector4f SubsurfaceColor, Vector4f FalloffColor);

	static Vector3f SeparableSSS_Gaussian(Float32 variance, Float32 r, Vector4f FalloffColor);
	static Vector3f SeparableSSS_Profile(Float32 r, Vector4f FalloffColor);

	//std::vector<Vector4f> SSSSMirroredProfilekernel;
	std::unordered_map<Int32, std::vector<Vector4f> > SSSSMirroredProfilekernelMap;
	static const Int32 QualityIndex = 3;
	static const Int32 SUBSURFACE_KERNEL_SIZE = 3;

	void CreateResources();
	void InitSSSSProfilekernelParams(std::string const & _prefix, Int32 index);
	UInt32 SSSFrameBuffer;
};

struct TemporalAAPixelUniformData
{
	Int32 JitterID;
	Int32 PlusWeights_0ID;
	Int32 PlusWeights_1ID;
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

	UInt32 TAAToScreenFrame_Tex;
	UInt32 TAAHistoryFrame_Tex;
	UInt32 TAAFrameBuffer;

	void Execute(UInt32 VAO, Int32 NumFaces, IndexSizeType indexType = IndexSizeType::Index16Bits);
private:
	std::shared_ptr<TemporalAAPixelUniformData> TAAPixelUniformData;

	std::vector<Vector2f> FrustumJitter;
	Int32 ActiveJitterIndex;
	Vector4f ActiveJitterSample;
	Mat4f JitterProjectMatrix;
	Mat4f JitterProjectMatrix_PreFrame;
	void InitUE4SampleType2();
	void InitUE4SampleType4();

	void HackUpdateCameraProjectMatrix(Float32 sampleOffsetX, Float32 sampleOffsetY);
	void HackRemoveCameraProjectMatrix(Float32 sampleOffsetX, Float32 sampleOffsetY);
	void UpdateCameraProjectMatrix(Float32 sampleOffsetX, Float32 sampleOffsetY);

private:
	void CreateTAAPassMaterial();
	void CreateTAAPassResources();
};

struct ToneMappingPixelShaderUniformData
{
	Int32 GrainRandomFullID;
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
	void Execute(UInt32 VAO, Int32 NumFaces, IndexSizeType indexType = IndexSizeType::Index16Bits);

	UInt32 LUT_TEX;
	void GenerateLUTTexture(std::shared_ptr<RectBufferObject> pPObj);

private:
	void CreateToneMappingPassMaterial();
	std::shared_ptr<ToneMappingPixelShaderUniformData> ToneMappingUniformDatas;
	Vector2f GrainRandomFromFrame(Int32 FrameCountMode8);
};




