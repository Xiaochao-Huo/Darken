#include "Model.h"
#include <glew.h>
#include <glfw3.h>

Model::Model()
{
	
}

Model::Model(std::string fileName, glm::vec3 scale, bool bPackToOneMesh)
{
	LoadModelFromAsset(fileName, scale, bPackToOneMesh);
}

Model::~Model()
{	
	for (unsigned int Index = 0; Index < MeshGroup.size(); Index++)
	{
		//glDeleteBuffers(1, &MeshGroup[Index]->VAO);
		//glDeleteBuffers(1, &MeshGroup[Index]->VBO);
		//glDeleteBuffers(1, &MeshGroup[Index]->IBO);
	}
}

void Model::LoadModelFromAsset(std::string fileName, glm::vec3 scale, bool bPackToOneMesh)
{
	
	LoadFromAssetWithAssimp(AssetFolderPath, fileName, scale, bPackToOneMesh);
	
}

void Model::LoadFromAssetWithAssimp(std::string folderPath, std::string fileName, glm::vec3 scale, bool bPackToOneMesh)
{
	std::string file = folderPath + fileName;
	const aiScene* scene = import.ReadFile(file, aiProcess_Triangulate);
	if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "Error Load" << fileName << std::endl;
		return;
	}
	
	AddNode(scene, scene->mRootNode, glm::mat4(1.0));
	RenderGroup = NodeGroup;
}

void Model::AddNode(const aiScene* scene, aiNode* node, glm::mat4 parentTransformMatrix)
{
	if (node == nullptr) return;
	if (node->mMeshes == nullptr && node->mNumChildren != 0)
	{
		glm::mat4 TransformMatrix;
		memcpy(&TransformMatrix[0][0], &node->mTransformation.a1, sizeof(float) * 16);
		TransformMatrix = glm::transpose(TransformMatrix);
		TransformMatrix = TransformMatrix * parentTransformMatrix;

		for (unsigned int ChildIndex = 0; ChildIndex < node->mNumChildren; ChildIndex++)
		{
			AddNode(scene, node->mChildren[ChildIndex], TransformMatrix);
		}
	}
	else
	{
		glm::mat4 TransformMatrix;
		memcpy(&TransformMatrix[0][0], &node->mTransformation.a1, sizeof(float) * 16);
		TransformMatrix = glm::transpose(TransformMatrix);
		TransformMatrix = TransformMatrix * parentTransformMatrix;

		for (unsigned int MeshIndex = 0; MeshIndex < node->mNumMeshes; MeshIndex++)
		{
			aiMesh * AssimpMesh = scene->mMeshes[node->mMeshes[MeshIndex]];
			bool AlreadyHaveMesh = false;
			for (unsigned int MeshIndex = 0; MeshIndex < MeshGroup.size(); MeshIndex++)
			{
				if (AssimpMesh == MeshGroup[MeshIndex]->AssimpMeshPtr)
				{
					AlreadyHaveMesh = true;
					break;
				}
			}

			if (AlreadyHaveMesh)
			{
				std::shared_ptr<ModelNode> mNode = std::shared_ptr<ModelNode>(new ModelNode());
				mNode->VAO = MeshGroup[MeshIndex]->VAO;
				mNode->VBO = MeshGroup[MeshIndex]->VBO;
				mNode->IBO = MeshGroup[MeshIndex]->IBO;
				mNode->IBOIndexSizeType = MeshGroup[MeshIndex]->IBOIndexSizeType;
				mNode->NumFaces = MeshGroup[MeshIndex]->NumFaces;
				mNode->SurroundSphereBox = MeshGroup[MeshIndex]->SurroundSphereBox;
				mNode->SurroundRectBox = MeshGroup[MeshIndex]->SurroundRectBox;
				mNode->ModelMatrix = TransformMatrix;
				NodeGroup.push_back(mNode);
			} 
			else
			{
				std::shared_ptr<ModelMesh> mMesh = std::shared_ptr<ModelMesh>(new ModelMesh());
				MeshGroup.push_back(mMesh);
					
				int NumFaces = AssimpMesh->mNumFaces;
				IndexSizeType IndexType = IndexSizeType::Index32Bits;
				unsigned int * IndexData = new unsigned int[NumFaces * 3];

				for (int FaceIndex = 0; FaceIndex < NumFaces; FaceIndex++)
				{
					IndexData[FaceIndex * 3 + 0] = AssimpMesh->mFaces[FaceIndex].mIndices[0];
					IndexData[FaceIndex * 3 + 1] = AssimpMesh->mFaces[FaceIndex].mIndices[1];
					IndexData[FaceIndex * 3 + 2] = AssimpMesh->mFaces[FaceIndex].mIndices[2];
				}
				
				unsigned int IBO;
				glGenBuffers(1, &IBO);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, NumFaces * 3 * sizeof(unsigned short) * (IndexType + 1), IndexData, GL_STATIC_DRAW);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

				delete[] IndexData;

				int NumVertex = AssimpMesh->mNumVertices;

				unsigned int VAO;
				glGenVertexArrays(1, &VAO);
				glBindVertexArray(VAO);

				int DataWidth = 0;
				if (AssimpMesh->HasPositions())
				{
					DataWidth += sizeof(float) * 3;

					glm::vec3 * PositionData = new glm::vec3[NumVertex];
					memcpy(PositionData, &AssimpMesh->mVertices[0].x, sizeof(glm::vec3) * NumVertex);
					for (int VertexID = 0; VertexID < NumVertex; VertexID++)
					{
						glm::vec3 Position = glm::vec3(AssimpMesh->mVertices[VertexID].x, AssimpMesh->mVertices[VertexID].y, AssimpMesh->mVertices[VertexID].z);
						glm::vec4 LocalPosition = TransformMatrix * glm::vec4(Position, 1.0);
						LocalPosition.y = -LocalPosition.y;
						PositionData[VertexID] = glm::vec3(LocalPosition);
						mMesh->SurroundSphereBox += glm::vec3(LocalPosition);
						mMesh->SurroundRectBox += glm::vec3(LocalPosition);
					}
					
					unsigned int PositionBuffer;
					glGenBuffers(1, &PositionBuffer);
					glBindBuffer(GL_ARRAY_BUFFER, PositionBuffer);
					glBufferData(GL_ARRAY_BUFFER, NumVertex * 3 * sizeof(float), PositionData, GL_STATIC_DRAW);
					glBindBuffer(GL_ARRAY_BUFFER, 0);	
					glBindVertexBuffer(0, PositionBuffer, 0, 3 * sizeof(float));
					glEnableVertexAttribArray(0);
					glVertexAttribBinding(0, 0); 
					glVertexAttribFormat(0, 3, GL_FLOAT, false, 0);

					delete[] PositionData;
				}
				if (AssimpMesh->HasNormals())
				{
					glm::vec3 * NormalData = new glm::vec3[NumVertex];
					memcpy(NormalData, &AssimpMesh->mNormals[0].x, sizeof(glm::vec3) * NumVertex);

					glm::mat4 TransformMatrix_IT = glm::inverse(glm::transpose(TransformMatrix));
					for (int VertexID = 0; VertexID < NumVertex; VertexID++)
					{
						glm::vec3 Normal = glm::vec3(AssimpMesh->mNormals[VertexID].x, -AssimpMesh->mNormals[VertexID].y, AssimpMesh->mNormals[VertexID].z);
						NormalData[VertexID] = glm::normalize(glm::mat3(TransformMatrix_IT) * Normal);
					}

					DataWidth += sizeof(float) * 3;
					unsigned int NormalBuffer;
					glGenBuffers(1, &NormalBuffer);
					glBindBuffer(GL_ARRAY_BUFFER, NormalBuffer);
					glBufferData(GL_ARRAY_BUFFER, NumVertex * 3 * sizeof(float), NormalData, GL_STATIC_DRAW);
					glBindBuffer(GL_ARRAY_BUFFER, 0);
					glBindVertexBuffer(1, NormalBuffer, 0, 3 * sizeof(float));
					glEnableVertexAttribArray(1);
					glVertexAttribBinding(1, 1);
					glVertexAttribFormat(1, 3, GL_FLOAT, false, 0);

					delete[] NormalData;
				}
				if (AssimpMesh->HasTangentsAndBitangents())
				{
					DataWidth += sizeof(float) * 6;
					
					glm::vec3 * TangentData = new glm::vec3[NumVertex];
					memcpy(TangentData, &AssimpMesh->mTangents[0].x, sizeof(glm::vec3) * NumVertex);

					glm::mat4 TransformMatrix_IT = glm::inverse(glm::transpose(TransformMatrix));
					for (int VertexID = 0; VertexID < NumVertex; VertexID++)
					{
						glm::vec3 Tangent = glm::vec3(AssimpMesh->mTangents[VertexID].x, -AssimpMesh->mTangents[VertexID].y, AssimpMesh->mTangents[VertexID].z);
						TangentData[VertexID] = glm::normalize(glm::mat3(TransformMatrix_IT) * Tangent);
					}

					unsigned int TangentsBuffer;
					glGenBuffers(1, &TangentsBuffer);
					glBindBuffer(GL_ARRAY_BUFFER, TangentsBuffer);
					glBufferData(GL_ARRAY_BUFFER, NumVertex * 3 * sizeof(float), TangentData, GL_STATIC_DRAW);
					glBindBuffer(GL_ARRAY_BUFFER, 0);
					glBindVertexBuffer(2, TangentsBuffer, 0, 3 * sizeof(float));
					glEnableVertexAttribArray(2);
					glVertexAttribBinding(2, 2);
					glVertexAttribFormat(2, 3, GL_FLOAT, false, 0);

					delete[] TangentData;

					glm::vec3 * BitangentData = new glm::vec3[NumVertex];
					memcpy(BitangentData, &AssimpMesh->mBitangents[0].x, sizeof(glm::vec3) * NumVertex);

					for (int VertexID = 0; VertexID < NumVertex; VertexID++)
					{
						glm::vec3 Bitangent = glm::vec3(-AssimpMesh->mBitangents[VertexID].x, AssimpMesh->mBitangents[VertexID].y, -AssimpMesh->mBitangents[VertexID].z);
						BitangentData[VertexID] = glm::normalize(glm::mat3(TransformMatrix_IT) * Bitangent);
					}

					unsigned int BitangentsBuffer;
					glGenBuffers(1, &BitangentsBuffer);
					glBindBuffer(GL_ARRAY_BUFFER, BitangentsBuffer);
					glBufferData(GL_ARRAY_BUFFER, NumVertex * 3 * sizeof(float), BitangentData, GL_STATIC_DRAW);
					glBindBuffer(GL_ARRAY_BUFFER, 0);
					glBindVertexBuffer(3, BitangentsBuffer, 0, 3 * sizeof(float));
					glEnableVertexAttribArray(3);
					glVertexAttribBinding(3, 3);
					glVertexAttribFormat(3, 3, GL_FLOAT, false, 0);

					delete[] BitangentData;
				}
				for (int UVindex = 0; UVindex < 3; UVindex++)
				{
					if (AssimpMesh->mNumUVComponents[UVindex] != 0)
					{
						if (AssimpMesh->mNumUVComponents[UVindex] != 2) assert("UVChnenel data size must be 2" == "0");
						DataWidth += sizeof(float) * 2;

						float* Data = new float[NumVertex * 2];
						for (int VertexIndex = 0; VertexIndex < NumVertex; VertexIndex++)
						{
							Data[VertexIndex * 2] = AssimpMesh->mTextureCoords[UVindex][VertexIndex].x;
							Data[VertexIndex * 2 + 1] = AssimpMesh->mTextureCoords[UVindex][VertexIndex].y;
						}

						unsigned int UVBuffer;
						glGenBuffers(1, &UVBuffer);
						glBindBuffer(GL_ARRAY_BUFFER, UVBuffer);
						glBufferData(GL_ARRAY_BUFFER, NumVertex * 2 * sizeof(float), Data, GL_STATIC_DRAW);
						glBindBuffer(GL_ARRAY_BUFFER, 0);

						glBindVertexBuffer(4 + UVindex, UVBuffer, 0, 2 * sizeof(float));
						glEnableVertexAttribArray(4 + UVindex);
						glVertexAttribBinding(4 + UVindex, 4 + UVindex);
						glVertexAttribFormat(4 + UVindex, 2, GL_FLOAT, false, 0);

						delete[] Data;
					}
					else
					{
						float* Data = new float[NumVertex * 2];
						for (int VertexIndex = 0; VertexIndex < NumVertex; VertexIndex++)
						{
							Data[VertexIndex * 2] = 0.0f;
							Data[VertexIndex * 2 + 1] = 0.0f;
						}

						unsigned int UVBuffer;
						glGenBuffers(1, &UVBuffer);
						glBindBuffer(GL_ARRAY_BUFFER, UVBuffer);
						glBufferData(GL_ARRAY_BUFFER, NumVertex * 2 * sizeof(float), Data, GL_STATIC_DRAW);
						glBindBuffer(GL_ARRAY_BUFFER, 0);

						glBindVertexBuffer(4 + UVindex, UVBuffer, 0, 2 * sizeof(float));
						glEnableVertexAttribArray(4 + UVindex);
						glVertexAttribBinding(4 + UVindex, 4 + UVindex);
						glVertexAttribFormat(4 + UVindex, 2, GL_FLOAT, false, 0);

						delete[] Data;
					}
				}

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
				glBindVertexArray(0);

				mMesh->VAO = VAO;
				mMesh->VBO = 0;
				mMesh->IBO = IBO;
				mMesh->IBOIndexSizeType = IndexType;
				mMesh->NumFaces = NumFaces;
				mMesh->AssimpMeshPtr = AssimpMesh;
				
				std::shared_ptr<ModelNode> mNode = std::shared_ptr<ModelNode>(new ModelNode());
				NodeGroup.push_back(mNode);
				mNode->VAO = VAO;
				mNode->VBO = 0;
				mNode->IBO = IBO;
				mNode->IBOIndexSizeType = IndexType;
				mNode->ModelMatrix = glm::mat4(1.0);
				mNode->NumFaces = NumFaces;
				mNode->SurroundRectBox = mMesh->SurroundRectBox;
				mNode->SurroundSphereBox = mMesh->SurroundSphereBox;
			}			
		}
		for (unsigned int ChildIndex = 0; ChildIndex < node->mNumChildren; ChildIndex++)
		{
			AddNode(scene, node->mChildren[ChildIndex], TransformMatrix);
		}
	}
}


void Model::BindMaterial(std::shared_ptr<MaterialInstance> shadowDepthMaterial, std::shared_ptr<MaterialInstance> lightingMaterial)
{
	ShadowDepthMaterial = shadowDepthMaterial;
	LightingMaterial = lightingMaterial;
	MaterialInstData = std::shared_ptr<MaterialData>(new MaterialData());
}

void Model::BindShadowDepthMaterial()
{
	RenderMaterial = ShadowDepthMaterial;
}

void Model::BindLightingMaterial()
{
	RenderMaterial = LightingMaterial;
}

void Model::CheckWhetherNeedClip(std::shared_ptr<Camera> camera)
{
	for (unsigned int Index = 0; Index < RenderGroup.size(); Index++)
	{
		//RenderGroup[Index]->bNeedClip = RenderGroup[Index]->SurroundSphereBox.bOutOfCamera(ObjectTransform.GetModelMatrix() * (RenderGroup[Index]->AnimationSys.GetNumFrames() == -1 ? RenderGroup[Index]->ModelMatrix : RenderGroup[Index]->AnimationSys.GetNextFrameAttitudeMatrix()), _Scene->GetCamera(CameraIndex::MainCamera)->GetVPMatrix());
		glm::vec3 Scale = ObjectTransform.GetScale();
		float MaxScale = Scale.x > Scale.y ? Scale.x : Scale.y;
		MaxScale = MaxScale > Scale.z ? MaxScale : Scale.z;
		RenderGroup[Index]->bNeedClip = RenderGroup[Index]->SurroundSphereBox.bOutOfCamera(ObjectTransform.GetModelMatrix() * RenderGroup[Index]->ModelMatrix, camera->GetVPMatrix(), MaxScale);
	}
}

void Model::Draw()
{
	for (unsigned int Index = 0; Index < RenderGroup.size(); Index++)
	{
		if (RenderGroup[Index]->bNeedClip) continue;

		glm::mat4 ModelMatrix = ObjectTransform.GetModelMatrix() *  RenderGroup[Index]->ModelMatrix;
		glm::mat4 Model_ITMatrix = glm::inverse(glm::transpose(ObjectTransform.GetModelMatrix() * RenderGroup[Index]->ModelMatrix));
		glm::mat4 Model_PreMatrix = ObjectTransform.GetModelMatrix_PreFrame() * RenderGroup[Index]->ModelMatrix;
		glm::mat4 Model_ITPreMatrix = glm::inverse(glm::transpose(ObjectTransform.GetModelMatrix_PreFrame() * RenderGroup[Index]->ModelMatrix));
		_GPUBuffers->UpdateModelBuffer(ModelMatrix, Model_ITMatrix, Model_PreMatrix, Model_ITPreMatrix);

		RenderMaterial->SetUniform<glm::mat4>(MaterialInstData->ModelMatrixID, ObjectTransform.GetModelMatrix() *  RenderGroup[Index]->ModelMatrix);
		RenderMaterial->SetUniform<glm::mat4>(MaterialInstData->ModelMatrix_PreID, ObjectTransform.GetModelMatrix_PreFrame() * RenderGroup[Index]->ModelMatrix);
		RenderMaterial->SetUniform<glm::mat4>(MaterialInstData->ModelMatrix_ITID, glm::inverse(glm::transpose(ObjectTransform.GetModelMatrix() * RenderGroup[Index]->ModelMatrix)));

		RenderMaterial->GetParent()->Draw(RenderGroup[Index]->VAO, RenderGroup[Index]->NumFaces, RenderGroup[Index]->IBOIndexSizeType, 0, OGL_ELEMENT);
	}	
}

void Model::UpdatePreFrameModelMatrix()
{
	ObjectTransform.SetModelMatrix_PreFrame(ObjectTransform.GetModelMatrix());
}

void Model::Start()
{

}

void Model::Update()
{

}

void Model::InternalUpdate()
{
	/*for (int NodeIndex = 0; NodeIndex < NodeGroup.size(); NodeIndex++)
	{
		ModelNode* Node = NodeGroup[NodeIndex].get();
		if (Node->AnimationSys.GetNumFrames() == -1)
		{
			continue;
		}
		else
		{
			Node->ModelMatrix = Node->AnimationSys.GetNextFrameAttitudeMatrix();
		}
	}*/
}

void Model::FixUpdate()
{

}

std::vector<std::shared_ptr<ModelNode>>& Model::GetModelNodeGroup()
{
	return RenderGroup;
}

void Model::SetAnimationPlaySpeed(float speed)
{
	/*for (int NodeIndex = 0; NodeIndex < NodeGroup.size(); NodeIndex++)
	{
		NodeGroup[NodeIndex]->AnimationSys.SetPlaySpeed(speed);
	}*/
}

std::shared_ptr<MaterialInstance> Model::GetRenderMaterial()
{
	return RenderMaterial;
}