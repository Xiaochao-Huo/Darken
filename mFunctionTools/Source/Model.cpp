#include "Model.h"
#include <glew.h>
#include <glfw3.h>

Model::Model()
{
	
}

Model::Model(std::string fileName, Vector3f scale, Bool bPackToOneMesh)
{
	LoadModelFromAsset(fileName, scale, bPackToOneMesh);
}

Model::~Model()
{	
	for (UInt32 Index = 0; Index < MeshGroup.size(); Index++)
	{
		//glDeleteBuffers(1, &MeshGroup[Index]->VAO);
		//glDeleteBuffers(1, &MeshGroup[Index]->VBO);
		//glDeleteBuffers(1, &MeshGroup[Index]->IBO);
	}
}

void Model::LoadModelFromAsset(std::string fileName, Vector3f scale, Bool bPackToOneMesh)
{
	
	LoadFromAssetWithAssimp(AssetFolderPath, fileName, scale, bPackToOneMesh);
	
}

void Model::LoadFromAssetWithAssimp(std::string folderPath, std::string fileName, Vector3f scale, Bool bPackToOneMesh)
{
	std::string file = folderPath + fileName;
	const aiScene* scene = import.ReadFile(file, aiProcess_Triangulate);
	if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "Error Load" << fileName << std::endl;
		return;
	}
	
	AddNode(scene, scene->mRootNode, Mat4f(1.0));
	RenderGroup = NodeGroup;
}

void Model::AddNode(const aiScene* scene, aiNode* node, Mat4f parentTransformMatrix)
{
	if (node == nullptr) return;
	if (node->mMeshes == nullptr && node->mNumChildren != 0)
	{
		Mat4f TransformMatrix;
		memcpy(&TransformMatrix[0][0], &node->mTransformation.a1, sizeof(Float32) * 16);
		TransformMatrix = Math::Transpose(TransformMatrix);
		TransformMatrix = TransformMatrix * parentTransformMatrix;

		for (UInt32 ChildIndex = 0; ChildIndex < node->mNumChildren; ChildIndex++)
		{
			AddNode(scene, node->mChildren[ChildIndex], TransformMatrix);
		}
	}
	else
	{
		Mat4f TransformMatrix;
		memcpy(&TransformMatrix[0][0], &node->mTransformation.a1, sizeof(Float32) * 16);
		TransformMatrix = Math::Transpose(TransformMatrix);
		TransformMatrix = TransformMatrix * parentTransformMatrix;

		for (UInt32 MeshIndex = 0; MeshIndex < node->mNumMeshes; MeshIndex++)
		{
			aiMesh * AssimpMesh = scene->mMeshes[node->mMeshes[MeshIndex]];
			Bool AlreadyHaveMesh = false;
			for (UInt32 MeshIndex = 0; MeshIndex < MeshGroup.size(); MeshIndex++)
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
					
				Int32 NumFaces = AssimpMesh->mNumFaces;
				IndexSizeType IndexType = IndexSizeType::Index32Bits;
				UInt32 * IndexData = new UInt32[NumFaces * 3];

				for (Int32 FaceIndex = 0; FaceIndex < NumFaces; FaceIndex++)
				{
					IndexData[FaceIndex * 3 + 0] = AssimpMesh->mFaces[FaceIndex].mIndices[0];
					IndexData[FaceIndex * 3 + 1] = AssimpMesh->mFaces[FaceIndex].mIndices[1];
					IndexData[FaceIndex * 3 + 2] = AssimpMesh->mFaces[FaceIndex].mIndices[2];
				}
				
				UInt32 IBO;
				glGenBuffers(1, &IBO);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, NumFaces * 3 * sizeof(UInt16) * (IndexType + 1), IndexData, GL_STATIC_DRAW);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

				delete[] IndexData;

				Int32 NumVertex = AssimpMesh->mNumVertices;

				UInt32 VAO;
				glGenVertexArrays(1, &VAO);
				glBindVertexArray(VAO);

				Int32 DataWidth = 0;
				if (AssimpMesh->HasPositions())
				{
					DataWidth += sizeof(Float32) * 3;

					Vector3f * PositionData = new Vector3f[NumVertex];
					memcpy(PositionData, &AssimpMesh->mVertices[0].x, sizeof(Vector3f) * NumVertex);
					for (Int32 VertexID = 0; VertexID < NumVertex; VertexID++)
					{
						Vector3f Position = Vector3f(AssimpMesh->mVertices[VertexID].x, AssimpMesh->mVertices[VertexID].y, AssimpMesh->mVertices[VertexID].z);
						Vector4f LocalPosition = TransformMatrix * Vector4f(Position, 1.0);
						LocalPosition.y = -LocalPosition.y;
						PositionData[VertexID] = Vector3f(LocalPosition);
						mMesh->SurroundSphereBox += Vector3f(LocalPosition);
						mMesh->SurroundRectBox += Vector3f(LocalPosition);
					}
					
					UInt32 PositionBuffer;
					glGenBuffers(1, &PositionBuffer);
					glBindBuffer(GL_ARRAY_BUFFER, PositionBuffer);
					glBufferData(GL_ARRAY_BUFFER, NumVertex * 3 * sizeof(Float32), PositionData, GL_STATIC_DRAW);
					glBindBuffer(GL_ARRAY_BUFFER, 0);	
					glBindVertexBuffer(0, PositionBuffer, 0, 3 * sizeof(Float32));
					glEnableVertexAttribArray(0);
					glVertexAttribBinding(0, 0); 
					glVertexAttribFormat(0, 3, GL_FLOAT, false, 0);

					delete[] PositionData;
				}
				if (AssimpMesh->HasNormals())
				{
					Vector3f * NormalData = new Vector3f[NumVertex];
					memcpy(NormalData, &AssimpMesh->mNormals[0].x, sizeof(Vector3f) * NumVertex);

					Mat4f TransformMatrix_IT = Math::Inverse(Math::Transpose(TransformMatrix));
					for (Int32 VertexID = 0; VertexID < NumVertex; VertexID++)
					{
						Vector3f Normal = Vector3f(AssimpMesh->mNormals[VertexID].x, -AssimpMesh->mNormals[VertexID].y, AssimpMesh->mNormals[VertexID].z);
						NormalData[VertexID] = Math::Normalize(Mat3f(TransformMatrix_IT) * Normal);
					}

					DataWidth += sizeof(Float32) * 3;
					UInt32 NormalBuffer;
					glGenBuffers(1, &NormalBuffer);
					glBindBuffer(GL_ARRAY_BUFFER, NormalBuffer);
					glBufferData(GL_ARRAY_BUFFER, NumVertex * 3 * sizeof(Float32), NormalData, GL_STATIC_DRAW);
					glBindBuffer(GL_ARRAY_BUFFER, 0);
					glBindVertexBuffer(1, NormalBuffer, 0, 3 * sizeof(Float32));
					glEnableVertexAttribArray(1);
					glVertexAttribBinding(1, 1);
					glVertexAttribFormat(1, 3, GL_FLOAT, false, 0);

					delete[] NormalData;
				}
				if (AssimpMesh->HasTangentsAndBitangents())
				{
					DataWidth += sizeof(Float32) * 6;
					
					Vector3f * TangentData = new Vector3f[NumVertex];
					memcpy(TangentData, &AssimpMesh->mTangents[0].x, sizeof(Vector3f) * NumVertex);

					Mat4f TransformMatrix_IT = Math::Inverse(Math::Transpose(TransformMatrix));
					for (Int32 VertexID = 0; VertexID < NumVertex; VertexID++)
					{
						Vector3f Tangent = Vector3f(AssimpMesh->mTangents[VertexID].x, -AssimpMesh->mTangents[VertexID].y, AssimpMesh->mTangents[VertexID].z);
						TangentData[VertexID] = Math::Normalize(Mat3f(TransformMatrix_IT) * Tangent);
					}

					UInt32 TangentsBuffer;
					glGenBuffers(1, &TangentsBuffer);
					glBindBuffer(GL_ARRAY_BUFFER, TangentsBuffer);
					glBufferData(GL_ARRAY_BUFFER, NumVertex * 3 * sizeof(Float32), TangentData, GL_STATIC_DRAW);
					glBindBuffer(GL_ARRAY_BUFFER, 0);
					glBindVertexBuffer(2, TangentsBuffer, 0, 3 * sizeof(Float32));
					glEnableVertexAttribArray(2);
					glVertexAttribBinding(2, 2);
					glVertexAttribFormat(2, 3, GL_FLOAT, false, 0);

					delete[] TangentData;

					Vector3f * BitangentData = new Vector3f[NumVertex];
					memcpy(BitangentData, &AssimpMesh->mBitangents[0].x, sizeof(Vector3f) * NumVertex);

					for (Int32 VertexID = 0; VertexID < NumVertex; VertexID++)
					{
						Vector3f Bitangent = Vector3f(-AssimpMesh->mBitangents[VertexID].x, AssimpMesh->mBitangents[VertexID].y, -AssimpMesh->mBitangents[VertexID].z);
						BitangentData[VertexID] = Math::Normalize(Mat3f(TransformMatrix_IT) * Bitangent);
					}

					UInt32 BitangentsBuffer;
					glGenBuffers(1, &BitangentsBuffer);
					glBindBuffer(GL_ARRAY_BUFFER, BitangentsBuffer);
					glBufferData(GL_ARRAY_BUFFER, NumVertex * 3 * sizeof(Float32), BitangentData, GL_STATIC_DRAW);
					glBindBuffer(GL_ARRAY_BUFFER, 0);
					glBindVertexBuffer(3, BitangentsBuffer, 0, 3 * sizeof(Float32));
					glEnableVertexAttribArray(3);
					glVertexAttribBinding(3, 3);
					glVertexAttribFormat(3, 3, GL_FLOAT, false, 0);

					delete[] BitangentData;
				}
				for (Int32 UVindex = 0; UVindex < 3; UVindex++)
				{
					if (AssimpMesh->mNumUVComponents[UVindex] != 0)
					{
						if (AssimpMesh->mNumUVComponents[UVindex] != 2) assert("UVChnenel data size must be 2" == "0");
						DataWidth += sizeof(Float32) * 2;

						Float32* Data = new Float32[NumVertex * 2];
						for (Int32 VertexIndex = 0; VertexIndex < NumVertex; VertexIndex++)
						{
							Data[VertexIndex * 2] = AssimpMesh->mTextureCoords[UVindex][VertexIndex].x;
							Data[VertexIndex * 2 + 1] = AssimpMesh->mTextureCoords[UVindex][VertexIndex].y;
						}

						UInt32 UVBuffer;
						glGenBuffers(1, &UVBuffer);
						glBindBuffer(GL_ARRAY_BUFFER, UVBuffer);
						glBufferData(GL_ARRAY_BUFFER, NumVertex * 2 * sizeof(Float32), Data, GL_STATIC_DRAW);
						glBindBuffer(GL_ARRAY_BUFFER, 0);

						glBindVertexBuffer(4 + UVindex, UVBuffer, 0, 2 * sizeof(Float32));
						glEnableVertexAttribArray(4 + UVindex);
						glVertexAttribBinding(4 + UVindex, 4 + UVindex);
						glVertexAttribFormat(4 + UVindex, 2, GL_FLOAT, false, 0);

						delete[] Data;
					}
					else
					{
						Float32* Data = new Float32[NumVertex * 2];
						for (Int32 VertexIndex = 0; VertexIndex < NumVertex; VertexIndex++)
						{
							Data[VertexIndex * 2] = 0.0f;
							Data[VertexIndex * 2 + 1] = 0.0f;
						}

						UInt32 UVBuffer;
						glGenBuffers(1, &UVBuffer);
						glBindBuffer(GL_ARRAY_BUFFER, UVBuffer);
						glBufferData(GL_ARRAY_BUFFER, NumVertex * 2 * sizeof(Float32), Data, GL_STATIC_DRAW);
						glBindBuffer(GL_ARRAY_BUFFER, 0);

						glBindVertexBuffer(4 + UVindex, UVBuffer, 0, 2 * sizeof(Float32));
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
				mNode->ModelMatrix = Mat4f(1.0);
				mNode->NumFaces = NumFaces;
				mNode->SurroundRectBox = mMesh->SurroundRectBox;
				mNode->SurroundSphereBox = mMesh->SurroundSphereBox;
			}			
		}
		for (UInt32 ChildIndex = 0; ChildIndex < node->mNumChildren; ChildIndex++)
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
	for (UInt32 Index = 0; Index < RenderGroup.size(); Index++)
	{
		//RenderGroup[Index]->bNeedClip = RenderGroup[Index]->SurroundSphereBox.bOutOfCamera(ObjectTransform.GetModelMatrix() * (RenderGroup[Index]->AnimationSys.GetNumFrames() == -1 ? RenderGroup[Index]->ModelMatrix : RenderGroup[Index]->AnimationSys.GetNextFrameAttitudeMatrix()), _Scene->GetCamera(CameraIndex::MainCamera)->GetVPMatrix());
		Vector3f Scale = ObjectTransform.GetScale();
		Float32 MaxScale = Scale.x > Scale.y ? Scale.x : Scale.y;
		MaxScale = MaxScale > Scale.z ? MaxScale : Scale.z;
		RenderGroup[Index]->bNeedClip = RenderGroup[Index]->SurroundSphereBox.bOutOfCamera(ObjectTransform.GetModelMatrix() * RenderGroup[Index]->ModelMatrix, camera->GetVPMatrix(), MaxScale);
	}
}

void Model::Draw()
{
	for (UInt32 Index = 0; Index < RenderGroup.size(); Index++)
	{
		if (RenderGroup[Index]->bNeedClip) continue;

		Mat4f ModelMatrix = ObjectTransform.GetModelMatrix() *  RenderGroup[Index]->ModelMatrix;
		Mat4f Model_ITMatrix = Math::Inverse(Math::Transpose(ObjectTransform.GetModelMatrix() * RenderGroup[Index]->ModelMatrix));
		Mat4f Model_PreMatrix = ObjectTransform.GetModelMatrix_PreFrame() * RenderGroup[Index]->ModelMatrix;
		Mat4f Model_ITPreMatrix = Math::Inverse(Math::Transpose(ObjectTransform.GetModelMatrix_PreFrame() * RenderGroup[Index]->ModelMatrix));
		_GPUBuffers->UpdateModelBuffer(ModelMatrix, Model_ITMatrix, Model_PreMatrix, Model_ITPreMatrix);

		RenderMaterial->SetUniform<Mat4f>(MaterialInstData->ModelMatrixID, ObjectTransform.GetModelMatrix() *  RenderGroup[Index]->ModelMatrix);
		RenderMaterial->SetUniform<Mat4f>(MaterialInstData->ModelMatrix_PreID, ObjectTransform.GetModelMatrix_PreFrame() * RenderGroup[Index]->ModelMatrix);
		RenderMaterial->SetUniform<Mat4f>(MaterialInstData->ModelMatrix_ITID, Math::Inverse(Math::Transpose(ObjectTransform.GetModelMatrix() * RenderGroup[Index]->ModelMatrix)));

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
	/*for (Int32 NodeIndex = 0; NodeIndex < NodeGroup.size(); NodeIndex++)
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

void Model::SetAnimationPlaySpeed(Float32 speed)
{
	/*for (Int32 NodeIndex = 0; NodeIndex < NodeGroup.size(); NodeIndex++)
	{
		NodeGroup[NodeIndex]->AnimationSys.SetPlaySpeed(speed);
	}*/
}

std::shared_ptr<MaterialInstance> Model::GetRenderMaterial()
{
	return RenderMaterial;
}