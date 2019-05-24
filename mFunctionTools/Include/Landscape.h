#pragma once


#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsGles.h"
#include "mLandscapeComponent.h"
#include "mObject.h"
#include "mTexture.h"
#include "mMaterial.h"
#include "mGlobalPram.h"
#include <hash_map>
#include <memory>
#include <string>
#include <vector>

class LandscapeBrushBase;

class LandscapePaintLayerInfo
{
public:
	LandscapePaintLayerInfo(std::shared_ptr<Texture> paintTexture, bool bWeightNormalBlend);
	~LandscapePaintLayerInfo();

	std::shared_ptr<Texture> GetPaintTexture();
	void SetWeightTexture(std::shared_ptr<Texture> weightTexture);

	bool isWeightNormalBlend;
	int Index;
	int ChannelInWeightTexutre;

private:
	std::shared_ptr<Texture> PaintTexture;
};

class LandscapeGBufferRenderPipeline
{
public:
	LandscapeGBufferRenderPipeline();
	~LandscapeGBufferRenderPipeline();

	void Init();
	void ActivePipeline();
	void ClosePipeline();
	void HandleOffScreenPixelsAsynBegin();
	void HandleOffScreenPixelsAsynEnd();
	glm::vec3 GetMouseHitPosition();

private:
	float * PixelDataGBufferWorldPosition;
	glm::vec3 HitPositionWorld;

	GLuint FrameBufferObject;
	GLuint RBO_Depth;
	GLuint Tex_WorldPosition;
	GLuint PixelBufferObjectForWorldPosition;
};

#define HEIGHT_TEXTURE_MAX_SIZE 512

class Landscape : public Object
{
public:
	Landscape();
	~Landscape();

	Landscape(glm::vec3 position,
		glm::vec3 scale,
		glm::vec2 numComponent,
		glm::uint numSubsection,
		glm::uint numSubsectionQuad,
		std::shared_ptr<Material> gBufferMaterial,
		std::shared_ptr<Material> linghtingMaterial,
		std::shared_ptr<Material> brushCircleMaterial,
		std::shared_ptr<Material> wireframeMaterial);

	std::hash_map<int, std::shared_ptr<LandscapeComponent>> LandScapeComponentMap;
	int GetComponentMapHashKey(int ComponentX, int ComponentY);
	std::shared_ptr<LandscapeSharedBuffer> LandscapeComponentSharedBuffer;
	std::shared_ptr<LandscapeMaterialUniformData> MaterialUniformData;

	std::unique_ptr<LandscapeGBufferRenderPipeline> LandscapeGBufferPipeline;

	virtual void CheckWhetherNeedClip() override;
	virtual void Draw() override;
	virtual void Start() {};
	virtual void InternalUpdate(){};
	virtual void Update() {};
	virtual void FixUpdate() {};

	void RenderGBufferPipeline();
	void RenderBasePassPipeline();
	void RenderBrushCirclePipeline();

	void CalculateLOD();

	int GetVertX();
	int GetVertY();
	void GetHeightTextureData(int MinX, int MaxX, int MinY, int MaxY, unsigned short * DataStore);
	void SetHeightTextureData(int MinX, int MaxX, int MinY, int MaxY, unsigned short * DataWrite);
	void UpdateHeightTextureGPUObject(int MinX, int MaxX, int MinY, int MaxY);
	std::shared_ptr<Texture> CreateAbstractHeightTex();

	void GetWeightTextureData(std::shared_ptr<LandscapePaintLayerInfo> layerInfo, int MinX, int MaxX, int MinY, int MaxY, unsigned char * DataStore);
	void SetWeightTextureData(std::shared_ptr<LandscapePaintLayerInfo> layerInfo, int MinX, int MaxX, int MinY, int MaxY, unsigned char * DataWrite);
	void UpdateWeightTextureGPUObject(int MinX, int MaxX, int MinY, int MaxY);

	void BindBrush(std::weak_ptr<LandscapeBrushBase> brush);
	void BindPaintLayers(std::vector<LandscapePaintLayerInfo>& layers);

	std::shared_ptr<LandscapeBrushBase> GetCurrentBrush();
	std::shared_ptr<LandscapePaintLayerInfo> GetLayer(int index);
	std::vector<std::shared_ptr<LandscapePaintLayerInfo>>& GetLayers();

	//Render Relate
	float LOD0DistributionSetting;
	float OtherLODDistributionSetting;
	float ThresholdSectionRatioSquared; //此参数控制了DrawCall中绘制4个Subsection还是1个Component
	std::vector<float> LODScreenRatios;

	std::shared_ptr<Texture> AbstractHeightTex;

	int GetNumSubsection();
	int GetNumSubsectionQuad();
	int GetNumComponentX();
	int GetNumComponentY();

private:
	const float Landscape_HeightHalfValue = 32768.0f;

	std::shared_ptr<std::shared_ptr<Material>> LandscapeRenderMaterial;
	std::shared_ptr<Material> LandscapeLightingMaterial;
	std::shared_ptr<Material> LandscapeGBufferMaterial;
	std::shared_ptr<Material> LandscapeBrushCircleMaterial;
	std::shared_ptr<Material> LandscapeWireframeMaterial;

	std::weak_ptr<LandscapeBrushBase> LandscapeBrush;
	std::vector<std::shared_ptr<LandscapePaintLayerInfo>> Layers;

	glm::vec2 NumComponent;
	int NumSubsection;
	int NumSubsectionQuad;
		
	void BindMaterialDataPtr();
	void BindSubsectionNeighbors(int ComponentX, int ComponentY);

	void CalculateInclusiveComponentIndexOverlap(int MinX, int MaxX, int MinY, int MaxY, int * ComponentIndexX1, int * ComponentIndexX2, int * ComponentIndexY1, int * ComponentIndexY2);
	void CalculateInclusiveComponentIndexNoOverlap(int MinX, int MaxX, int MinY, int MaxY, int * ComponentIndexX1, int * ComponentIndexX2, int * ComponentIndexY1, int * ComponentIndexY2);
};

