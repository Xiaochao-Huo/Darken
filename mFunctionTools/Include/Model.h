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
	glm::mat4 ModelMatrix;
	int NumFaces;
	unsigned int VBO;
	unsigned int IBO;
	unsigned int VAO;
	IndexSizeType IBOIndexSizeType;
	SphereBox SurroundSphereBox;
	RectBox SurroundRectBox;
	//Animation AnimationSys;
	bool bNeedClip;
	ModelNode(){};
};

struct ModelMesh
{
	int NumFaces;
	unsigned int VAO;
	unsigned int VBO;
	unsigned int IBO;
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
	Model(std::string fileName, glm::vec3 scale = glm::vec3(1.0, 1.0, 1.0), bool bPackToOneMesh = false);
	~Model();

	void LoadModelFromAsset(std::string fileName, glm::vec3 scale = glm::vec3(1.0, 1.0, 1.0), bool bPackToOneMesh = false);
	void BindMaterial(std::shared_ptr<MaterialInstance> shadowDepthMaterial, std::shared_ptr<MaterialInstance> lightingMaterial);
	void BindShadowDepthMaterial();
	void BindLightingMaterial();
	void SetAnimationPlaySpeed(float speed);

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
		int ModelBlockID;
		int ViewBlockID;
		int ModelMatrixID;
		int ModelMatrix_ITID;
		int ModelMatrix_IT_PreID;
		int ModelMatrix_PreID;
		int ViewMatrixID;
		int ViewMatrix_PreID;
		int ProjectMatrixID;
		int ProjectMatrix_PreID;
		int ViewPositionID;

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
	void LoadFromAssetWithAssimp(std::string folderPath, std::string fileName, glm::vec3 scale = glm::vec3(1.0, 1.0, 1.0), bool bPackToOneMesh = false);
	void AddNode(const aiScene* scene, aiNode* node, glm::mat4 parentTransformMatrix);
	Assimp::Importer import;
};