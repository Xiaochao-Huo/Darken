#pragma once

#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsGles.h"
#include "mObject.h"
#include "mGlobalPram.h"
#include "mMaterial.h"
#include "mTexture.h"
#include "mUtil.h"

class Landscape;
class LandscapeComponent;

//       -Y       
//    - - 0 - -   
//    |       |   
// -X 1   P   2 +X
//    |       |   
//    - - 3 - -   
//       +Y    
#define SUBSECTIONNUM 4
#define LANDSCAPE_ZSCALE 1.0f / 128.0f

struct SubsectionIndexRange
{
	SubsectionIndexRange(int numFaces, int indexOffset)
		: NumFaces(numFaces)
		, IndexOffset(indexOffset)
	{}

	int NumFaces;
	int IndexOffset;
};

static const glm::vec3 WireframeBarycentricValues[3][3] =
{ { glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0) },
  { glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0), glm::vec3(1.0, 0.0, 0.0) },
  { glm::vec3(0.0, 0.0, 1.0), glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0) } };

class LandscapeSharedBuffer
{
public:
	LandscapeSharedBuffer();
	~LandscapeSharedBuffer();

	void Create(int numSubsection, int numSebsectionQuad);
	void CreateGPUObject();
	void ConfirmSubsectionIndex();
	//ES3.1 without ext is not support GeometryShader, so use trick method temporary
	void CreateWholeWireframeDataObject();

	void UseTrickWireframeVAOs();
	void UseBaseVAOs();

	GLuint * RenderVAOs;
	std::vector<std::vector<std::shared_ptr<SubsectionIndexRange>>> ElementInfoToDraw;

private:
	int NumSubsection;
	int NumSubsectionQuad;
	int NumMips;

	GLfloat * VertexBuffer;
	std::vector<GLuint*> IndexBuffers;

	GLuint vertexLocation = 0; //注意这里是约定inVertex的Location为0
	GLuint barycentricLocation = 1; //注意这里是约定barycentric的Location为1(WireframeVertexShader)

	GLuint SharedVertexVBO;
	GLuint* SharedIBOs;
	GLuint* SharedBaseVAOs;

	GLfloat* WireframeBarycentricBuffer;
	GLuint SharedBarycentricVBO;
	GLuint* SharedWireframeVAOs;
};

struct LandscapeMaterialUniformData
{
	GLuint HeightTextureID;

	glm::vec3 ViewPosition;
	glm::vec3 LightDirWorld;
	glm::vec3 LightColor;

	glm::mat4 ModelMatrix;
	glm::mat4 ViewMatrix;
	glm::mat4 ProjectMatrix;

	glm::vec4 LodBias;
	glm::vec4 LodValues;
	glm::vec4 SectionLods;
	glm::mat4 NeighborSectionsLod;
	glm::vec4 LandscapeParameters_SubsectionSizeVertsLayerUVPan;
	glm::vec4 LandscapeParameters_SubsectionOffsetParams;
	glm::vec4 LandscapeParameters_HeightTextureUVScaleBias;
	glm::vec4 LandscapeParameters_WeightTextureUVScaleBias;

	GLuint WeightTexture0ID;
	GLuint WeightTexture1ID;
	GLuint PaintTexture0ID;
	GLuint PaintTexture1ID;
	GLuint PaintTexture2ID;
	GLuint PaintTexture3ID;
	GLuint PaintTexture4ID;
	GLuint PaintTexture5ID;
	glm::vec4 PixelShaderPaintTexChannelIndex0;
	glm::vec4 PixelShaderPaintTexChannelIndex1;
	glm::vec4 PixelShaderPaintTexChannelIndex2;
	glm::vec4 PixelShaderPaintTexChannelIndex3;
	glm::vec4 PixelShaderPaintTexChannelIndex4;
	glm::vec4 PixelShaderPaintTexChannelIndex5;

	glm::vec3 HitPositionWorld;
	glm::vec2 BrushParameters_Sizes;
	LandscapeMaterialUniformData(){}
};

struct SubSectionData
{
	SubSectionData(glm::int8 InSubSectionOffsetX, glm::int8 InSubSectionOffsetY, bool InInsideComponent)
		: SubSectionOffsetX(InSubSectionOffsetX)
		, SubSectionOffsetY(InSubSectionOffsetY)
		, InsideComponent(InInsideComponent)
	{}

	glm::int8 SubSectionOffsetX;
	glm::int8 SubSectionOffsetY;
	bool InsideComponent;
};

static const SubSectionData SubSectionValues[4][4] = { { SubSectionData(0, 1, false), SubSectionData(1, 0, false), SubSectionData(1, 0, true), SubSectionData(0, 1, true) },
													   { SubSectionData(0, 1, false), SubSectionData(-1, 0, true), SubSectionData(-1, 0, false), SubSectionData(0, 1, true) },
													   { SubSectionData(0, -1, true), SubSectionData(1, 0, false), SubSectionData(1, 0, true), SubSectionData(0, -1, false) },
													   { SubSectionData(0, -1, true), SubSectionData(-1, 0, true), SubSectionData(-1, 0, false), SubSectionData(0, -1, false) } };

class LandscapeSubsection : public Object
{
public:
	LandscapeSubsection();
	LandscapeSubsection(glm::vec2 &centerPos, int numSubsectionQuad, LandscapeComponent* componentBelongTo);
	~LandscapeSubsection();

	std::vector<std::weak_ptr<LandscapeSubsection>> NeighborSubsection;
	
	glm::vec4 GetNeighborsLod();

	float LODValue;
	int CurrentLOD;
	int MaxLOD;
	float LODThreshold;
	float LODFactor;

	virtual void Draw(){};
	virtual void Start(){};
	virtual void InternalUpdate(){};
	virtual void Update(){};
	virtual void FixUpdate(){};

private:
	int NumSubsectionQuad;
	LandscapeComponent* ComponentBelongTo;
};

class LandscapeComponent : public Object
{
public:
	LandscapeComponent();
	~LandscapeComponent();

	void Init(int baseX, 
			  int baseY, 
			  int numSubsection, 
			  int numSubsectionQuad, 
			  Landscape* rootLandscape,
			  std::shared_ptr<LandscapeMaterialUniformData> materialUnifromData, 
			  std::shared_ptr<LandscapeSharedBuffer> sharedBuffer,
			  std::shared_ptr<std::shared_ptr<Material>> landscapeRenderMaterial);

	bool SubsectionsAllHaveSameNeighborLOD();
	int GetBaseX();
	int GetBaseY();

	glm::vec4 HeightTextureScaleBias;
	glm::vec4 WeightTextureScaleBias;
	float WeightTextureSubsectionOffset;
	std::vector<std::shared_ptr<LandscapeSubsection>> Subsections;
	std::shared_ptr<Texture> HeightTexture;
	std::vector<std::shared_ptr<Texture>> WeightTextures;

	void GenerateHeightTextureMipData(int X1ComponentCoord = 0, int X2ComponentCoord = INT_MAX, int Y1ComponentCoord = 0, int Y2ComponentCoord = INT_MAX);
	void UpdateSurroundBox();
	void GenerateWeightTextureMipData(std::shared_ptr<Texture> WeightTexture, int X1ComponentCoord = 0, int X2ComponentCoord = INT_MAX, int Y1ComponentCoord = 0, int Y2ComponentCoord = INT_MAX);

	bool bHaveCalLOD;
	void CalculateLOD();

	virtual void CheckWhetherNeedClip() override;
	virtual void Draw() override;
	virtual void Start() {};
	virtual void InternalUpdate(){};
	virtual void Update(){};
	virtual void FixUpdate(){};
	Landscape* RootLandscape;
	
private:
	int BaseX;
	int BaseY;
	int NumComponentQuad;
	int NumSubsectionQuad;
	int NumSubsection;

	std::shared_ptr<std::shared_ptr<Material>> RenderMaterial;
	std::shared_ptr<LandscapeMaterialUniformData> MaterialUniformData;
	std::shared_ptr<LandscapeSharedBuffer> SharedBuffer;

	float CalculateLODFromScreenSize(float inScreenSizeSquared);
	float CalculateScreenSize(glm::vec3 spereBoxOrigin, float sphereBoxRadius, float maxExtend, Transform & modelTransform);
	bool bDrawWholeComponent;
	void DrawWholeComponent();
	void DrawEachSubsection();
};


