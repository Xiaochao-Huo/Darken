#pragma once


#include "Object.h"
#include "MaterialInstance.h"
#include "Animation.h"
#include "Assimp/include/Importer.hpp"
#include "Assimp/include/scene.h"
#include "Assimp/include/postprocess.h"
#include <map>

struct ModelNode
{
	Mat4f ModelMatrix;
	Int32 NumFaces;
	UInt32 VBO;
	UInt32 IBO;
	UInt32 VAO;
	IndexSizeType IBOIndexSizeType;
	SphereBox SurroundSphereBox;
	RectBox SurroundRectBox;
	//Animation AnimationSys;
	Bool bNeedClip;
	ModelNode(){};
};

struct ModelMesh
{
	Int32 NumFaces;
	UInt32 VAO;
	UInt32 VBO;
	UInt32 IBO;
	IndexSizeType IBOIndexSizeType;
	SphereBox SurroundSphereBox;
	RectBox SurroundRectBox;
	aiMesh* AssimpMeshPtr;
	ModelMesh(){};
};

class Model : public Object
{
public:
	Model();
	Model(std::string fileName, Vector3f scale = Vector3f(1.0, 1.0, 1.0), Bool bPackToOneMesh = false);
	~Model();

	void LoadModelFromAsset(std::string fileName, Vector3f scale = Vector3f(1.0, 1.0, 1.0), Bool bPackToOneMesh = false);
	void BindMaterial(std::shared_ptr<MaterialInstance> shadowDepthMaterial, std::shared_ptr<MaterialInstance> lightingMaterial);
	void BindShadowDepthMaterial();
	void BindLightingMaterial();
	void SetAnimationPlaySpeed(Float32 speed);

	virtual void CheckWhetherNeedClip(std::shared_ptr<Camera> camera) final;
	virtual void Draw() final;
	virtual void Start();
	virtual void InternalUpdate() final;
	virtual void Update();
	virtual void FixUpdate();

	void UpdatePreFrameModelMatrix();

	std::shared_ptr<MaterialInstance> GetRenderMaterial();
	std::vector<std::shared_ptr<ModelNode>>& GetModelNodeGroup();

protected:

	struct MaterialData
	{
		Int32 ModelBlockID;
		Int32 ViewBlockID;
		Int32 ModelMatrixID;
		Int32 ModelMatrix_ITID;
		Int32 ModelMatrix_IT_PreID;
		Int32 ModelMatrix_PreID;
		Int32 ViewMatrixID;
		Int32 ViewMatrix_PreID;
		Int32 ProjectMatrixID;
		Int32 ProjectMatrix_PreID;
		Int32 ViewPositionID;

		MaterialData()
		{
			ModelBlockID = MaterialInstance::GetID("Model");
			ViewBlockID = MaterialInstance::GetID("View");
			ModelMatrixID = MaterialInstance::GetID("ModelMatrix");
			ModelMatrix_PreID = MaterialInstance::GetID("ModelMatrix_PreFrame");
			ModelMatrix_ITID = MaterialInstance::GetID("ModelMatrix_IT");
			ModelMatrix_IT_PreID = MaterialInstance::GetID("ModelMatrix_IT_PreFrame");
			ViewMatrixID = MaterialInstance::GetID("ViewMatrix");
			ViewMatrix_PreID = MaterialInstance::GetID("ViewMatrix_PreFrame");
			ProjectMatrixID = MaterialInstance::GetID("ProjectMatrix");
			ProjectMatrix_PreID = MaterialInstance::GetID("ProjectMatrix_PreFrame");
			ViewPositionID = MaterialInstance::GetID("ViewPosition");
		}
	};

	std::shared_ptr<MaterialInstance> RenderMaterial;
	std::shared_ptr<MaterialInstance> ShadowDepthMaterial;
	std::shared_ptr<MaterialInstance> LightingMaterial;
	std::shared_ptr<MaterialData> MaterialInstData;
	std::vector<std::shared_ptr<ModelNode>> RenderGroup;
	std::vector<std::shared_ptr<ModelNode>> NodeGroup;
	std::vector<std::shared_ptr<ModelMesh>> MeshGroup;
	void LoadFromAssetWithAssimp(std::string folderPath, std::string fileName, Vector3f scale = Vector3f(1.0, 1.0, 1.0), Bool bPackToOneMesh = false);
	void AddNode(const aiScene* scene, aiNode* node, Mat4f parentTransformMatrix);
	Assimp::Importer import;
};