#include "mLandscapeComponent.h"
#include "mLandscape.h"

LandscapeSharedBuffer::LandscapeSharedBuffer()
{
	SharedVertexVBO = 0;
	SharedBarycentricVBO = 0;
}

LandscapeSharedBuffer::~LandscapeSharedBuffer()
{
	if (VertexBuffer)
	{
		delete[] VertexBuffer;
		VertexBuffer = nullptr;
		for (int i = 0; i < IndexBuffers.size(); i++){
			delete[] IndexBuffers[i];
			IndexBuffers[i] = nullptr;
		}
	}
	if (WireframeBarycentricBuffer)
	{
		delete[]WireframeBarycentricBuffer;
		WireframeBarycentricBuffer = nullptr;
	}
	if (SharedBaseVAOs)
	{
		gl::DeleteBuffers(NumMips, SharedBaseVAOs);
		delete[]SharedBaseVAOs;
		SharedBaseVAOs = nullptr;
	}
	if (SharedWireframeVAOs)
	{
		gl::DeleteBuffers(NumMips, SharedWireframeVAOs);
		delete[]SharedWireframeVAOs;
		SharedWireframeVAOs = nullptr;
	}
	if (SharedIBOs)
	{
		gl::DeleteBuffers(NumMips, SharedIBOs);
		delete[]SharedIBOs;
		SharedIBOs = nullptr;
	}
	if (SharedVertexVBO)
	{
		gl::DeleteBuffers(1, &SharedVertexVBO);
	}
	if (SharedBarycentricVBO)
	{
		gl::DeleteBuffers(1, &SharedBarycentricVBO);
	}
}

void LandscapeSharedBuffer::Create(int numSubsection, int numSubsectionQuad)
{
	NumSubsection = numSubsection;
	NumSubsectionQuad = numSubsectionQuad;

	int numVertices = NumSubsection * NumSubsection * (NumSubsectionQuad + 1) * (NumSubsectionQuad + 1);

	VertexBuffer = new GLfloat[numVertices * 4];

	int Vertex = 0;
	for (int SubY = 0; SubY < NumSubsection; SubY++)
	{
		for (int SubX = 0; SubX < NumSubsection; SubX++)
		{
			for (int y = 0; y <= NumSubsectionQuad; y++)
			{
				for (int x = 0; x <= NumSubsectionQuad; x++)
				{
					/*VertexBuffer[Vertex++] = SubX * NumSubsectionQuad + x;
					VertexBuffer[Vertex++] = SubY * NumSubsectionQuad + y;*/
					VertexBuffer[Vertex++] = x;
					VertexBuffer[Vertex++] = y;
					VertexBuffer[Vertex++] = SubX;
					VertexBuffer[Vertex++] = SubY;
				}
			}
		}
	}

	NumMips = glm::log2(static_cast<float>(NumSubsectionQuad + 1));
	for (int Mip = 0; Mip < NumMips; Mip++){
		int NumSubsectionQuadMip = ((NumSubsectionQuad + 1) >> Mip) - 1;
		int numFaces = NumSubsection * NumSubsection * NumSubsectionQuadMip * NumSubsectionQuadMip;
		IndexBuffers.push_back(new GLuint[numFaces * 6]);
		int SubOffset = 0;
		int Index = 0;
		for (int SubY = 0; SubY < NumSubsection; SubY++)
		{
			for (int SubX = 0; SubX < NumSubsection; SubX++)
			{
				for (int y = 0; y < NumSubsectionQuadMip; y++)
				{
					for (int x = 0; x < NumSubsectionQuadMip; x++)
					{
						GLuint i00 = (x + 0) + (y + 0) * (NumSubsectionQuad + 1) + SubOffset;
						GLuint i10 = (x + 1) + (y + 0) * (NumSubsectionQuad + 1) + SubOffset;
						GLuint i01 = (x + 0) + (y + 1) * (NumSubsectionQuad + 1) + SubOffset;
						GLuint i11 = (x + 1) + (y + 1) * (NumSubsectionQuad + 1) + SubOffset;

						IndexBuffers[Mip][Index++] = i00;
						IndexBuffers[Mip][Index++] = i11;
						IndexBuffers[Mip][Index++] = i10;

						IndexBuffers[Mip][Index++] = i00;
						IndexBuffers[Mip][Index++] = i01;
						IndexBuffers[Mip][Index++] = i11;
					}
				}
				SubOffset += (NumSubsectionQuad + 1) * (NumSubsectionQuad + 1);
			}
		}
	}
}

void LandscapeSharedBuffer::CreateWholeWireframeDataObject()
{
	int numVertices = NumSubsection * NumSubsection * (NumSubsectionQuad + 1) * (NumSubsectionQuad + 1);

	WireframeBarycentricBuffer = new GLfloat[numVertices * 3];

	int Vertex = 0;
	for (int SubY = 0; SubY < NumSubsection; SubY++)
	{
		for (int SubX = 0; SubX < NumSubsection; SubX++)
		{
			for (int y = 0; y <= NumSubsectionQuad; y++)
			{
				for (int x = 0; x <= NumSubsectionQuad; x++)
				{
					WireframeBarycentricBuffer[Vertex++] = WireframeBarycentricValues[x % 3][y % 3].x;
					WireframeBarycentricBuffer[Vertex++] = WireframeBarycentricValues[x % 3][y % 3].y;
					WireframeBarycentricBuffer[Vertex++] = WireframeBarycentricValues[x % 3][y % 3].z;
				}
			}
		}
	}

	int numVertexBytes = 4;
	int numBarycentricBytes = 3;

	gl::GenBuffers(1, &SharedBarycentricVBO);
	gl::BindBuffer(GL_ARRAY_BUFFER, SharedBarycentricVBO);
	gl::BufferData(GL_ARRAY_BUFFER, numBarycentricBytes * numVertices * sizeof(GLfloat), static_cast<void*>(WireframeBarycentricBuffer), GL_STATIC_DRAW);
	gl::BindBuffer(GL_ARRAY_BUFFER, 0);
	
	SharedWireframeVAOs = new GLuint[NumMips];
	for (int Mip = 0; Mip < NumMips; Mip++){
		int NumSubsectionQuadMip = ((NumSubsectionQuad + 1) >> Mip) - 1;
		int numFaces = NumSubsection * NumSubsection * NumSubsectionQuadMip * NumSubsectionQuadMip;

		GLuint VAO;

		gl::GenVertexArrays(1, &VAO);
		gl::BindVertexArray(VAO);
		gl::BindVertexBuffer(0, SharedVertexVBO, 0, numVertexBytes * sizeof(GLfloat));  //BindingIndex = 0;
		gl::BindVertexBuffer(1, SharedBarycentricVBO, 0, numBarycentricBytes * sizeof(GLfloat));  //BindingIndex = 1;
		gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, SharedIBOs[Mip]);
		gl::EnableVertexAttribArray(vertexLocation);
		gl::EnableVertexAttribArray(barycentricLocation);
		gl::VertexAttribBinding(vertexLocation, 0);  //BindingIndex = 0;
		gl::VertexAttribBinding(barycentricLocation, 1);	//BindingIndex = 1;
		gl::VertexAttribFormat(vertexLocation, numVertexBytes, GL_FLOAT, GL_FALSE, 0);
		gl::VertexAttribFormat(barycentricLocation, numBarycentricBytes, GL_FLOAT, GL_FALSE, 0);
		gl::BindVertexArray(0);

		SharedWireframeVAOs[Mip] = VAO;
	}
}

void LandscapeSharedBuffer::CreateGPUObject()
{
	int numVertices = NumSubsection * NumSubsection * (NumSubsectionQuad + 1) * (NumSubsectionQuad + 1);

	int numVertexBytes = 4;

	gl::GenBuffers(1, &SharedVertexVBO);
	gl::BindBuffer(GL_ARRAY_BUFFER, SharedVertexVBO);
	gl::BufferData(GL_ARRAY_BUFFER, numVertices * numVertexBytes * sizeof(GLfloat), static_cast<void*>(VertexBuffer), GL_STATIC_DRAW);
	gl::BindBuffer(GL_ARRAY_BUFFER, 0);

	SharedIBOs = new GLuint[NumMips];
	SharedBaseVAOs = new GLuint[NumMips];
	for (int Mip = 0; Mip < NumMips; Mip++){
		int NumSubsectionQuadMip = ((NumSubsectionQuad + 1) >> Mip) - 1;
		int numFaces = NumSubsection * NumSubsection * NumSubsectionQuadMip * NumSubsectionQuadMip;

		GLuint VAO;
		GLuint IBO;

		gl::GenBuffers(1, &IBO);
		gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
		gl::BufferData(GL_ELEMENT_ARRAY_BUFFER, numFaces * 6 * sizeof(GLuint), static_cast<void*>(IndexBuffers[Mip]), GL_STATIC_DRAW);
		gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		SharedIBOs[Mip] = IBO;

		gl::GenVertexArrays(1, &VAO);
		gl::BindVertexArray(VAO);
		gl::BindVertexBuffer(0, SharedVertexVBO, 0, numVertexBytes * sizeof(GLfloat));
		gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
		gl::EnableVertexAttribArray(vertexLocation);
		gl::VertexAttribBinding(vertexLocation, 0);
		gl::VertexAttribFormat(vertexLocation, numVertexBytes, GL_FLOAT, GL_FALSE, 0);
		gl::BindVertexArray(0);

		SharedBaseVAOs[Mip] = VAO;
	}
}

void LandscapeSharedBuffer::ConfirmSubsectionIndex()
{
	for (int Mip = 0; Mip < NumMips; Mip++)
	{
		ElementInfoToDraw.push_back(std::vector<std::shared_ptr<SubsectionIndexRange>>());
		int NumSubsectionQuadMip = ((NumSubsectionQuad + 1) >> Mip) - 1;
		int numFaces = NumSubsectionQuadMip * NumSubsectionQuadMip;
		for (int SubY = 0; SubY < NumSubsection; SubY++)
		{
			for (int SubX = 0; SubX < NumSubsection; SubX++)
			{
				int offset = numFaces * 3 * (SubY * NumSubsection + SubX) * 2;
				ElementInfoToDraw[Mip].push_back(std::shared_ptr<SubsectionIndexRange>(new SubsectionIndexRange(numFaces, offset)));
			}
		}
	}
}

void LandscapeSharedBuffer::UseBaseVAOs()
{
	RenderVAOs = SharedBaseVAOs;
}

void LandscapeSharedBuffer::UseTrickWireframeVAOs()
{
	RenderVAOs = SharedWireframeVAOs;
}

LandscapeSubsection::LandscapeSubsection() 
{
}

LandscapeSubsection::~LandscapeSubsection()
{
	ComponentBelongTo = nullptr;
}

LandscapeSubsection::LandscapeSubsection(glm::vec2 &centerPosInComponentCoord, int numSubsectionQuad, LandscapeComponent* componentBelongTo)
: CurrentLOD(-1)
, LODValue(-1.0f)
, MaxLOD(-1)
, LODThreshold(2.0f)
, LODFactor(10.0f)
{
	ComponentBelongTo = componentBelongTo;
	MaxLOD = glm::log2(static_cast<float>(numSubsectionQuad + 1)) - 1;
	NumSubsectionQuad = numSubsectionQuad;
	ObjectTransform = ComponentBelongTo->ObjectTransform;
	glm::vec2 OriginPosInComponentCoord = centerPosInComponentCoord - glm::vec2(numSubsectionQuad / 2.0);
	ObjectTransform.SetPosition(glm::vec3(ObjectTransform.GetModelMatrix() * glm::vec4(OriginPosInComponentCoord, 0.0, 1.0)));
	NeighborSubsection = std::vector<std::weak_ptr<LandscapeSubsection>>(SUBSECTIONNUM);
}

glm::vec4 LandscapeSubsection::GetNeighborsLod()
{
	glm::vec4 NeighborLods;
	for (int NeighborIndex = 0; NeighborIndex < SUBSECTIONNUM; NeighborIndex++)
	{
		std::shared_ptr<LandscapeSubsection> Neighbor = NeighborSubsection[NeighborIndex].lock();
		if (Neighbor != nullptr)
		{
			if (Neighbor->bNeedClip && !Neighbor->ComponentBelongTo->bHaveCalLOD)
			{
				Neighbor->ComponentBelongTo->CalculateLOD();
			}
			NeighborLods[NeighborIndex] = glm::max(Neighbor->LODValue, LODValue);
		}
		else
		{
			
			NeighborLods[NeighborIndex] = LODValue;
		}
	}
	return NeighborLods;
}

LandscapeComponent::LandscapeComponent()
{
}

LandscapeComponent::~LandscapeComponent()
{
	RootLandscape = nullptr;
}

void LandscapeComponent::Init(int baseX, 
							  int baseY, 
							  int numSubsection, 
							  int numSubsectionQuad, 
							  Landscape* rootLandscape,
							  std::shared_ptr<LandscapeMaterialUniformData> materialUnifromData, 
							  std::shared_ptr<LandscapeSharedBuffer> sharedBuffer,
							  std::shared_ptr<std::shared_ptr<Material>> landscapeRenderMaterial)
{
	BaseX = baseX;
	BaseY = baseY;
	NumComponentQuad = numSubsection * numSubsectionQuad;
	NumSubsectionQuad = numSubsectionQuad;
	NumSubsection = numSubsection;
	RootLandscape = rootLandscape;
	MaterialUniformData = materialUnifromData;
	SharedBuffer = sharedBuffer;
	RenderMaterial = landscapeRenderMaterial;
	bHaveCalLOD = false;
	bDrawWholeComponent = true;

	ObjectTransform = RootLandscape->ObjectTransform;
	ObjectTransform.SetPosition(glm::vec3(ObjectTransform.GetModelMatrix() * glm::vec4(BaseX, BaseY, 0.0, 1.0)));

	for (int SubY = 0; SubY < NumSubsection; SubY++)
	{
		for (int SubX = 0; SubX < NumSubsection; SubX++)
		{
			glm::vec2 SubsectionCenterPosInComponentCoord = glm::vec2((2 * SubX + 1) * NumSubsectionQuad / 2.0f, (2 * SubY + 1) * NumSubsectionQuad / 2.0f);
			Subsections.push_back(std::shared_ptr<LandscapeSubsection>(new LandscapeSubsection(SubsectionCenterPosInComponentCoord, NumSubsectionQuad, this)));
		}
	}
}

int LandscapeComponent::GetBaseX()
{
	return BaseX;
}

int LandscapeComponent::GetBaseY()
{
	return BaseY;
}

void LandscapeComponent::DrawWholeComponent()
{
	int Mip = glm::min(Subsections[0]->CurrentLOD, Subsections[1]->CurrentLOD);
	Mip = glm::min(Mip, Subsections[2]->CurrentLOD);
	Mip = glm::min(Mip, Subsections[3]->CurrentLOD);
	int NumSubsectionQuadMip = ((NumSubsectionQuad + 1) >> Mip) - 1;
	int numFaces = NumSubsection * NumSubsection * NumSubsectionQuadMip * NumSubsectionQuadMip;

	MaterialUniformData->LodBias = glm::vec4(0.0f);
	MaterialUniformData->LodValues = glm::vec4(Mip, 0.0f, NumSubsectionQuadMip, 1.0f / NumSubsectionQuadMip);
	MaterialUniformData->SectionLods = glm::vec4(Subsections[0]->LODValue, 
												 Subsections[1]->LODValue, 
												 Subsections[2]->LODValue, 
												 Subsections[3]->LODValue);

	MaterialUniformData->NeighborSectionsLod[0] = Subsections[0]->GetNeighborsLod();
	MaterialUniformData->NeighborSectionsLod[1] = Subsections[1]->GetNeighborsLod();
	MaterialUniformData->NeighborSectionsLod[2] = Subsections[2]->GetNeighborsLod();
	MaterialUniformData->NeighborSectionsLod[3] = Subsections[3]->GetNeighborsLod();
	
	(*RenderMaterial)->Draw(SharedBuffer->RenderVAOs[Mip], numFaces * 2, Index32Bits, 0, GLDrawType::OGL_ELEMENT);
}

void LandscapeComponent::DrawEachSubsection()
{
	for (int SubY = 0; SubY < NumSubsection; SubY++)
	{
		for (int SubX = 0; SubX < NumSubsection; SubX++)
		{
			MaterialUniformData->NeighborSectionsLod = glm::mat4(0.0);

			int currentIndex = SubY * NumSubsection + SubX;
			std::shared_ptr<LandscapeSubsection> Subsection = Subsections[currentIndex];
			//Subsection->CurrentLOD = 1;

			MaterialUniformData->LodBias = glm::vec4(0.0f);
			MaterialUniformData->LodValues = glm::vec4(Subsection->CurrentLOD, 0.0f, ((NumSubsectionQuad + 1) >> Subsection->CurrentLOD) - 1, 1.0f / (((NumSubsectionQuad + 1) >> Subsection->CurrentLOD) - 1));
			MaterialUniformData->SectionLods = glm::vec4(Subsection->LODValue);
			MaterialUniformData->NeighborSectionsLod[currentIndex] = Subsection->GetNeighborsLod();

			int Mip = Subsections[currentIndex]->CurrentLOD;
			int NumSubsectionQuadMip = ((NumSubsectionQuad + 1) >> Mip) - 1;
			std::shared_ptr<SubsectionIndexRange> subsectionIndexToDraw = SharedBuffer->ElementInfoToDraw[Mip][currentIndex];
			(*RenderMaterial)->Draw(SharedBuffer->RenderVAOs[Mip], subsectionIndexToDraw->NumFaces * 2, Index32Bits, subsectionIndexToDraw->IndexOffset, GLDrawType::OGL_ELEMENT);
		}
	}
}

void LandscapeComponent::CheckWhetherNeedClip()
{
	bNeedClip = SurroundBox.bOutOfCamera(ObjectTransform.GetModelMatrix(), _Scene->GetCamera(CameraIndex::MainCamera)->GetVPMatrix());
	if (NumSubsection > 1)
	{
		for (int Index = 0; Index < Subsections.size(); Index++)
		{
			Subsections[Index]->bNeedClip = bNeedClip;
		}
	}
}

void LandscapeComponent::Draw()
{
	MaterialUniformData->ModelMatrix = ObjectTransform.GetModelMatrix();
	MaterialUniformData->LandscapeParameters_SubsectionSizeVertsLayerUVPan = glm::vec4(NumSubsectionQuad + 1, 1.0f / NumSubsectionQuad, BaseX, BaseY);
	MaterialUniformData->LandscapeParameters_SubsectionOffsetParams = glm::vec4((float)(NumSubsectionQuad + 1) / (float)HeightTexture->GetWidth(), (float)(NumSubsectionQuad + 1) / (float)HeightTexture->GetHeight(), WeightTextureSubsectionOffset, (float)NumSubsectionQuad);
	MaterialUniformData->LandscapeParameters_HeightTextureUVScaleBias = HeightTextureScaleBias;
	MaterialUniformData->LandscapeParameters_WeightTextureUVScaleBias = WeightTextureScaleBias;
	MaterialUniformData->HeightTextureID = HeightTexture->GPUId;
	MaterialUniformData->WeightTexture0ID = WeightTextures[0]->GPUId;
	MaterialUniformData->WeightTexture1ID = WeightTextures[1]->GPUId;

	if (NumSubsection> 1 && bDrawWholeComponent)
	{
		DrawWholeComponent();
	}
	else
	{
		DrawEachSubsection();
	}

}

void LandscapeComponent::CalculateLOD()
{
	bHaveCalLOD = true;
	float ComponentScreenSize = CalculateScreenSize(SurroundBox.GetCenterPointLocal(), SurroundBox.GetRadius(), NumSubsectionQuad * glm::max(ObjectTransform.GetScale().x, ObjectTransform.GetScale().y), ObjectTransform);

	if (NumSubsection > 1)
	{
		bool AllSubsectionHaveSameScreenRatio = true;
		float CombineScreenRatio = 0.0f;
		for (int SubsectionY = 0; SubsectionY < NumSubsection; SubsectionY++)
		{
			for (int SubsectionX = 0; SubsectionX < NumSubsection; SubsectionX++)
			{
				int SubsectionIndex = SubsectionX + SubsectionY * NumSubsection;
				//Subsections[SubsectionIndex]->MaxLOD = 0;

				glm::vec3 SubsectionBoxOrigin = SurroundBox.GetCenterPointLocal();
				SubsectionBoxOrigin.x = (float)NumSubsectionQuad * (float)SubsectionX + (float)NumSubsectionQuad / 2.0f;
				SubsectionBoxOrigin.y = (float)NumSubsectionQuad * (float)SubsectionY + (float)NumSubsectionQuad / 2.0f;
				
				float SubsectionScreenSize = CalculateScreenSize(SubsectionBoxOrigin, SurroundBox.GetRadius() / 2.0f, (float)NumSubsectionQuad * glm::max(ObjectTransform.GetScale().x, ObjectTransform.GetScale().y) / 2.0f, ObjectTransform);
				float fLOD = CalculateLODFromScreenSize(SubsectionScreenSize);
				Subsections[SubsectionIndex]->LODValue = glm::clamp(fLOD, 0.0f, (float)Subsections[SubsectionIndex]->MaxLOD);
				Subsections[SubsectionIndex]->CurrentLOD = glm::floor(Subsections[SubsectionIndex]->LODValue);

				if (ComponentScreenSize > RootLandscape->ThresholdSectionRatioSquared)
				{
					if (AllSubsectionHaveSameScreenRatio)
					{
						if (CombineScreenRatio >0.0f && (CombineScreenRatio - SubsectionScreenSize) > 0.001)
						{
							AllSubsectionHaveSameScreenRatio = false;
						}

						CombineScreenRatio += SubsectionScreenSize;

						if (SubsectionIndex > 0)
						{
							CombineScreenRatio *= 0.5f;
						}
					}
				}
			}
		}
		if (AllSubsectionHaveSameScreenRatio)
		{
			bDrawWholeComponent = true;
		}
		else
		{
			bDrawWholeComponent = false;
		}
	}
	else
	{
		//Subsections[0]->MaxLOD = 0;
		bDrawWholeComponent = true;
		float fLOD = CalculateLODFromScreenSize(ComponentScreenSize);
		Subsections[0]->LODValue = glm::clamp(fLOD, 0.0f, (float)Subsections[0]->MaxLOD);
		Subsections[0]->CurrentLOD = glm::floor(Subsections[0]->LODValue);
	}
}

float LandscapeComponent::CalculateLODFromScreenSize(float inScreenSizeSquared)
{
	int LODScreenRatioCount = RootLandscape->LODScreenRatios.size();
	float ScreenSizeSquared = inScreenSizeSquared;
	int CurrentScreenRatioIndex = -1;
	for (int Index = 1; Index < LODScreenRatioCount; Index++)
	{
		CurrentScreenRatioIndex = glm::clamp(Index - 1, 0, LODScreenRatioCount - 1);
		if (ScreenSizeSquared > RootLandscape->LODScreenRatios[Index])
		{
			break;
		}
	}

	float CurrentScreenRatio = RootLandscape->LODScreenRatios[CurrentScreenRatioIndex];
	float NextScreenRatio = RootLandscape->LODScreenRatios[CurrentScreenRatioIndex == (LODScreenRatioCount - 1) ? 0 : (CurrentScreenRatioIndex + 1)];
	if (ScreenSizeSquared < NextScreenRatio)
	{
		NextScreenRatio = ScreenSizeSquared;
	}
	float ScreenRatioRange = CurrentScreenRatio - NextScreenRatio;

	float CurrentLODRatioRange = (ScreenSizeSquared - NextScreenRatio) / ScreenRatioRange;
	float fLOD = (float)CurrentScreenRatioIndex + (1.0f - CurrentLODRatioRange);

	return fLOD;

	/*glm::vec3 CameraPosWorld = _Scene->GetCamera(CameraIndex::MainCamera)->GetPosition();
	glm::vec3 CenterLocal = glm::vec3(NumSubsectionQuad / 2.0f, NumSubsectionQuad / 2.0f, 0.0f);
	glm::vec4 CenterWorld = ObjectTransform.GetModelMatrix() * glm::vec4(CenterLocal, 1.0f);
	LODValue = glm::distance(glm::vec3(CenterWorld), CameraPosWorld) / LODFactor - LODThreshold;
	LODValue = glm::min(LODValue, static_cast<float>(MaxLOD));
	CurrentLOD = LODValue < 0 ? 0 : static_cast<int>(glm::min(static_cast<float>(MaxLOD), LODValue));
	LODValue = glm::max(LODValue, 0.0f);*/
}

float LandscapeComponent::CalculateScreenSize(glm::vec3 spereBoxOrigin, float sphereBoxRadius, float maxExtend, Transform & modelTransform)
{
	glm::vec3 CameraPosWorld = _Scene->GetCamera(CameraIndex::MainCamera)->GetPosition();
	glm::vec3 BoxOriginWorld = glm::vec3(modelTransform.GetModelMatrix() * glm::vec4(spereBoxOrigin, 1.0f));

	glm::vec3 OriginToCamera = glm::abs(CameraPosWorld - BoxOriginWorld);
	glm::vec3 ClosetPoint = glm::vec3(glm::min(OriginToCamera.x, maxExtend), glm::min(OriginToCamera.y, maxExtend), glm::min(OriginToCamera.z, maxExtend));
	float DistanceSquared = pow(glm::length(OriginToCamera - ClosetPoint), 2.0);

	float temp = glm::length(OriginToCamera);
	temp = temp - sphereBoxRadius;
	temp = temp * temp;

	glm::mat4 ProjectMatrix = _Scene->GetCamera(CameraIndex::MainCamera)->GetProjectionMatrix();
	float ScreenMultiple = glm::max(0.5f * ProjectMatrix[0][0], 0.5f * ProjectMatrix[1][1]);
	float SquaredScreenRadius = glm::pow(ScreenMultiple * sphereBoxRadius * glm::max(modelTransform.GetScale().x, modelTransform.GetScale().y), 2.0f) / glm::max(1.0f, temp);

	return glm::min(SquaredScreenRadius * 2.0f, 1.0f);
}

bool LandscapeComponent::SubsectionsAllHaveSameNeighborLOD()
{
	glm::vec4 NeightborLod[4];
	NeightborLod[0] = Subsections[0]->GetNeighborsLod();
	NeightborLod[1] = Subsections[1]->GetNeighborsLod();
	NeightborLod[2] = Subsections[2]->GetNeighborsLod();
	NeightborLod[3] = Subsections[3]->GetNeighborsLod();

	for (int i = 0; i < SUBSECTIONNUM - 1; i++)
	{
		if (NeightborLod[i] != NeightborLod[i+1])
		{
			return false;
		}
	}
	return true;
}

void LandscapeComponent::GenerateHeightTextureMipData(int X1ComponentCoord, int X2ComponentCoord, int Y1ComponentCoord, int Y2ComponentCoord)
{
	if (X2ComponentCoord == INT_MAX)
	{
		X2ComponentCoord = NumSubsection * NumSubsectionQuad;
	}
	if (Y2ComponentCoord == INT_MAX)
	{
		Y2ComponentCoord = NumSubsection * NumSubsectionQuad;
	}

	int HeightTextureSizeU = HeightTexture->GetRawDataPtr(0)->Width;// HeightTextureMipDatas[0]->Width;
	int HeightTextureSizeV = HeightTexture->GetRawDataPtr(0)->Height; //HeightTextureMipDatas[0]->Height;
	int HeightTextureOffsetX = glm::round(HeightTextureScaleBias.z * (float)HeightTextureSizeU);
	int HeightTextureOffsetY = glm::round(HeightTextureScaleBias.w * (float)HeightTextureSizeV);

	for (int SubsectionY = 0; SubsectionY < NumSubsection; SubsectionY++)
	{
		if ((Y1ComponentCoord > NumSubsectionQuad * (SubsectionY + 1)) || (Y2ComponentCoord < NumSubsectionQuad * SubsectionY)) 
		{
			continue;
		}
		for (int SubsectionX = 0; SubsectionX < NumSubsection; SubsectionX++)
		{
			if ((X1ComponentCoord > NumSubsectionQuad * (SubsectionX + 1)) || (X2ComponentCoord < NumSubsectionQuad * SubsectionX))
			{
				continue;
			}

			int PreMipX1SubCoord = X1ComponentCoord - SubsectionX * NumSubsectionQuad;
			int PreMipX2SubCoord = X2ComponentCoord - SubsectionX * NumSubsectionQuad;
			int PreMipY1SubCoord = Y1ComponentCoord - SubsectionY * NumSubsectionQuad;
			int PreMipY2SubCoord = Y2ComponentCoord - SubsectionY * NumSubsectionQuad;

			int PreMipNumSubsectionQuad = NumSubsectionQuad;
			float InvPreMipNumSubsectionQuad = 1.0f / (float)PreMipNumSubsectionQuad;

			int PreMipHeightTextureOffsetX = HeightTextureOffsetX;
			int PreMipHeightTextureOffsetY = HeightTextureOffsetY;

			int PreMipSizeU = HeightTextureSizeU;
			int PreMipSizeV = HeightTextureSizeV;

			int NumMip = glm::log2(static_cast<float>(NumSubsectionQuad + 1));
			for (int Mip = 1; Mip < NumMip; Mip++)
			{
				int MipNumSubsectionQuad = ((NumSubsectionQuad + 1) >> Mip) - 1;
				float InvMipNumSubsectionQuad = 1.0f / (float)MipNumSubsectionQuad;

				int MipSizeU = HeightTextureSizeU >> Mip;
				int MiPSizeV = HeightTextureSizeV >> Mip;
				int MipHeightTextureOffsetX = HeightTextureOffsetX >> Mip;
				int MipHeightTextureOffsetY = HeightTextureOffsetY >> Mip;

				int MipX1SubCoord = glm::floor((float)MipNumSubsectionQuad * (float)PreMipX1SubCoord * InvPreMipNumSubsectionQuad);
				int MipX2SubCoord = glm::ceil((float)MipNumSubsectionQuad * (float)PreMipX2SubCoord * InvPreMipNumSubsectionQuad);
				int MipY1SubCoord = glm::floor((float)MipNumSubsectionQuad * (float)PreMipY1SubCoord * InvPreMipNumSubsectionQuad);
				int MipY2SubCoord = glm::ceil((float)MipNumSubsectionQuad * (float)PreMipY2SubCoord * InvPreMipNumSubsectionQuad);

				int VertX1 = glm::clamp(MipX1SubCoord, 0, MipNumSubsectionQuad);
				int VertX2 = glm::clamp(MipX2SubCoord, 0, MipNumSubsectionQuad);
				int VertY1 = glm::clamp(MipY1SubCoord, 0, MipNumSubsectionQuad);
				int VertY2 = glm::clamp(MipY2SubCoord, 0, MipNumSubsectionQuad);

				for (int VertY = VertY1; VertY <= VertY2; VertY++)
				{
					for (int VertX = VertX1; VertX <= VertX2; VertX++)
					{
						float PreVertX = (float)PreMipNumSubsectionQuad * VertX * InvMipNumSubsectionQuad;
						float PreVertY = (float)PreMipNumSubsectionQuad * VertY * InvMipNumSubsectionQuad;

						int TexX = MipHeightTextureOffsetX + SubsectionX * (MipNumSubsectionQuad + 1) + VertX;
						int TexY = MipHeightTextureOffsetY + SubsectionY * (MipNumSubsectionQuad + 1) + VertY;

						float fPreMipTexX = (float)PreMipHeightTextureOffsetX + (float)(SubsectionX * (PreMipNumSubsectionQuad + 1)) + PreVertX;
						float fPreMipTexY = (float)PreMipHeightTextureOffsetY + (float)(SubsectionY * (PreMipNumSubsectionQuad + 1)) + PreVertY;

						int PreMipTexX1 = glm::floor(fPreMipTexX);
						float PreMipTexFracX = glm::fract(fPreMipTexX);
						int PreMipTexY1 = glm::floor(fPreMipTexY);
						float PreMipTexFracY = glm::fract(fPreMipTexY);

						int PreMipTexX2 = glm::min(PreMipTexX1 + 1, PreMipSizeU - 1);
						int PreMipTexY2 = glm::min(PreMipTexY1 + 1, PreMipSizeV - 1);

						unsigned char* PreMipTexData00 = HeightTexture->GetRawDataPtr(Mip - 1)->MipData + (PreMipTexX1 + PreMipTexY1 * PreMipSizeU) * 4;
						unsigned char* PreMipTexData01 = HeightTexture->GetRawDataPtr(Mip - 1)->MipData + (PreMipTexX1 + PreMipTexY2 * PreMipSizeU) * 4;
						unsigned char* PreMipTexData10 = HeightTexture->GetRawDataPtr(Mip - 1)->MipData + (PreMipTexX2 + PreMipTexY1 * PreMipSizeU) * 4;
						unsigned char* PreMipTexData11 = HeightTexture->GetRawDataPtr(Mip - 1)->MipData + (PreMipTexX2 + PreMipTexY2 * PreMipSizeU) * 4;

						unsigned short PreMipHeightValue00 = PreMipTexData00[0] << 8 | PreMipTexData00[1];
						unsigned short PreMipHeightValue01 = PreMipTexData01[0] << 8 | PreMipTexData01[1];
						unsigned short PreMipHeightValue10 = PreMipTexData10[0] << 8 | PreMipTexData10[1];
						unsigned short PreMipHeightValue11 = PreMipTexData11[0] << 8 | PreMipTexData11[1];

						unsigned short HeightValue = glm::round(
							Math::Lerp(
							Math::Lerp((float)PreMipHeightValue00, (float)PreMipHeightValue10, PreMipTexFracX),
							Math::Lerp((float)PreMipHeightValue01, (float)PreMipHeightValue11, PreMipTexFracX),
							PreMipTexFracY));

						unsigned char * MipTexData = HeightTexture->GetRawDataPtr(Mip)->MipData + (TexX + TexY * MipSizeU) * 4;
						MipTexData[0] = HeightValue >> 8;
						MipTexData[1] = HeightValue & 255;

						MipTexData[2] = glm::round(
							Math::Lerp(
							Math::Lerp((float)PreMipTexData00[2], (float)PreMipTexData10[2], PreMipTexFracX),
							Math::Lerp((float)PreMipTexData01[2], (float)PreMipTexData11[2], PreMipTexFracX),
							PreMipTexFracY));

						MipTexData[3] = glm::round(
							Math::Lerp(
							Math::Lerp((float)PreMipTexData00[3], (float)PreMipTexData10[3], PreMipTexFracX),
							Math::Lerp((float)PreMipTexData01[3], (float)PreMipTexData11[3], PreMipTexFracX),
							PreMipTexFracY));
					}
				}
				PreMipNumSubsectionQuad = MipNumSubsectionQuad;
				InvPreMipNumSubsectionQuad = InvMipNumSubsectionQuad;
				
				PreMipHeightTextureOffsetX = MipHeightTextureOffsetX;
				PreMipHeightTextureOffsetY = MipHeightTextureOffsetY;
				
				PreMipSizeU = MipSizeU;
				PreMipSizeV = MiPSizeV;

				PreMipX1SubCoord = MipX1SubCoord;
				PreMipX2SubCoord = MipX2SubCoord;
				PreMipY1SubCoord = MipY1SubCoord;
				PreMipY2SubCoord = MipY2SubCoord;
			}
		}
	}
}

void LandscapeComponent::UpdateSurroundBox()
{
	SurroundBox.Reset();
	int NumComponentQuad = NumSubsection * NumSubsectionQuad;
	for (int VertY = 0; VertY <= NumComponentQuad; VertY++)
	{
		for (int VertX = 0; VertX <= NumComponentQuad; VertX++)
		{
			int TexX = (NumSubsection > 1) ? ((VertX - NumSubsectionQuad) >= 0 ? (VertX + 1) : VertX) : VertX;
			int TexY = (NumSubsection > 1) ? ((VertY - NumSubsectionQuad) >= 0 ? (VertY + 1) : VertY) : VertY;

			int TexSizeU = HeightTexture->GetWidth();
			int TexSizeV = HeightTexture->GetHeight();
			int HeightTexOffsetX = HeightTextureScaleBias.z * TexSizeU;
			int HeightTexOffsetY = HeightTextureScaleBias.w * TexSizeV;

			unsigned char* HeightTexData = HeightTexture->GetRawDataPtr(0)->MipData + (HeightTexOffsetX + HeightTexOffsetY * TexSizeU) * 4 + (TexX + TexY * TexSizeU) * 4;
			unsigned short HeightData = HeightTexData[0] << 8 | HeightTexData[1];
			float HeightDataLocal = ((float)HeightData - 32768.0f) * LANDSCAPE_ZSCALE;
			SurroundBox += glm::vec3(VertX, VertY, HeightDataLocal);
		}
	}
}

void LandscapeComponent::GenerateWeightTextureMipData(std::shared_ptr<Texture> WeightTexture, int X1ComponentCoord, int X2ComponentCoord, int Y1ComponentCoord, int Y2ComponentCoord)
{
	if (X2ComponentCoord == INT_MAX)
	{
		X2ComponentCoord = NumSubsectionQuad * NumSubsection;
	}
	if (Y2ComponentCoord == INT_MAX)
	{
		Y2ComponentCoord = NumSubsectionQuad * NumSubsection;
	}

	int WeightTexSizeU = WeightTexture->GetWidth();
	int WeightTexSizeV = WeightTexture->GetHeight();

	for (int SubsectionY = 0; SubsectionY < NumSubsection; SubsectionY++)
	{
		if ((Y1ComponentCoord > NumSubsectionQuad * (SubsectionY + 1)) || (Y2ComponentCoord < NumSubsectionQuad * SubsectionY))
		{
			continue;
		}
		for (int SubsectionX = 0; SubsectionX < NumSubsection; SubsectionX++)
		{
			if ((X1ComponentCoord > NumSubsectionQuad * (SubsectionY + 1)) || (X2ComponentCoord < NumSubsectionQuad * SubsectionX))
			{
				continue;
			}

			int PreMipTexSizeU = WeightTexSizeU;
			int PreMipTexSizeV = WeightTexSizeV;

			int PreMipX1SubsectionCoord = X1ComponentCoord - NumSubsectionQuad * SubsectionX;
			int PreMipX2SubsectionCoord = X2ComponentCoord - NumSubsectionQuad * SubsectionX;
			int PreMipY1SubsectionCoord = Y1ComponentCoord - NumSubsectionQuad * SubsectionY;
			int PreMipY2SubsectionCoord = Y2ComponentCoord - NumSubsectionQuad * SubsectionY;

			int PreMipNumSubsectionQuad = NumSubsectionQuad;
			float InvPreMipNumsubsectionQuad = 1.0f / (float)PreMipNumSubsectionQuad;

			int NumMip = log2(NumSubsectionQuad + 1);
			for (int Mip = 1; Mip < NumMip; Mip++)
			{
				int MipTexSizeU = PreMipTexSizeU >> Mip;
				int MipTexSizeV = PreMipTexSizeV >> Mip;

				int MipNumSubsectionQuad = ((NumSubsectionQuad + 1) >> Mip) - 1;
				float InvMipNumSubsectionQuad = 1.0f / (float)MipNumSubsectionQuad;
				int MipX1SubsectionCoord = glm::floor((float)PreMipX1SubsectionCoord * (float)MipNumSubsectionQuad * InvPreMipNumsubsectionQuad);
				int MipX2SubsectionCoord = glm::ceil((float)PreMipX2SubsectionCoord * (float)MipNumSubsectionQuad * InvPreMipNumsubsectionQuad);
				int MipY1SubsectionCoord = glm::floor((float)PreMipY1SubsectionCoord * (float)MipNumSubsectionQuad * InvPreMipNumsubsectionQuad);
				int MipY2SubsectionCoord = glm::ceil((float)PreMipY2SubsectionCoord * (float)MipNumSubsectionQuad * InvPreMipNumsubsectionQuad);

				int VertX1 = glm::clamp(MipX1SubsectionCoord, 0, MipNumSubsectionQuad);
				int VertX2 = glm::clamp(MipX2SubsectionCoord, 0, MipNumSubsectionQuad);
				int VertY1 = glm::clamp(MipY1SubsectionCoord, 0, MipNumSubsectionQuad);
				int VertY2 = glm::clamp(MipY2SubsectionCoord, 0, MipNumSubsectionQuad);

				for (int VertY = VertY1; VertY <= VertY2; VertY++)
				{
					for (int VertX = VertX1; VertX <= VertX2; VertX++)
					{
						float PreMipVertX = (float)VertX * (float)InvMipNumSubsectionQuad * (float)PreMipNumSubsectionQuad;
						float PreMipVertY = (float)VertY * (float)InvMipNumSubsectionQuad * (float)PreMipNumSubsectionQuad;

						int PreMipTexX1 = glm::floor((float)SubsectionX * (float)(PreMipNumSubsectionQuad + 1) + PreMipVertX);
						float PreMipTexFracX1 = glm::fract((float)SubsectionX * (float)(PreMipNumSubsectionQuad + 1) + PreMipVertX);
						int PreMipTexY1 = glm::floor((float)SubsectionY * (float)(PreMipNumSubsectionQuad + 1) + PreMipVertY);
						float PreMipTexFracY1 = glm::fract((float)SubsectionY * (float)(PreMipNumSubsectionQuad + 1) + PreMipVertY);

						int MipTexX = (MipNumSubsectionQuad + 1) * SubsectionX + VertX;
						int MipTexY = (MipNumSubsectionQuad + 1) * SubsectionY + VertY;

						int PreMipTexX2 = glm::min(PreMipTexX1 + 1, PreMipTexSizeU - 1);
						int PreMipTexY2 = glm::min(PreMipTexY1 + 1, PreMipTexSizeV - 1);

						unsigned char* PreMipWeightData00 = WeightTexture->GetRawDataPtr(Mip - 1)->MipData + (PreMipTexY1 * PreMipTexSizeU + PreMipTexX1) * 4;
						unsigned char* PreMipWeightData10 = WeightTexture->GetRawDataPtr(Mip - 1)->MipData + (PreMipTexY1 * PreMipTexSizeU + PreMipTexX2) * 4;
						unsigned char* PreMipWeightData01 = WeightTexture->GetRawDataPtr(Mip - 1)->MipData + (PreMipTexY2 * PreMipTexSizeU + PreMipTexX1) * 4;
						unsigned char* PreMipWeightData11 = WeightTexture->GetRawDataPtr(Mip - 1)->MipData + (PreMipTexY2 * PreMipTexSizeU + PreMipTexX2) * 4;

						unsigned char* TexData = WeightTexture->GetRawDataPtr(Mip)->MipData + (MipTexY * MipTexSizeU + MipTexX) * 4;
						TexData[0] = glm::round(
							Math::Lerp(
							Math::Lerp((float)PreMipWeightData00[0], (float)PreMipWeightData10[0], PreMipTexFracX1),
							Math::Lerp((float)PreMipWeightData01[0], (float)PreMipWeightData11[0], PreMipTexFracX1),
							PreMipTexFracY1));
						TexData[1] = glm::round(
							Math::Lerp(
							Math::Lerp((float)PreMipWeightData00[1], (float)PreMipWeightData10[1], PreMipTexFracX1),
							Math::Lerp((float)PreMipWeightData10[1], (float)PreMipWeightData11[1], PreMipTexFracX1),
							PreMipTexFracY1));
						TexData[2] = glm::round(
							Math::Lerp(
							Math::Lerp((float)PreMipWeightData00[2], (float)PreMipWeightData10[2], PreMipTexFracX1),
							Math::Lerp((float)PreMipWeightData10[2], (float)PreMipWeightData11[2], PreMipTexFracX1),
							PreMipTexFracY1));
						TexData[3] = glm::round(
							Math::Lerp(
							Math::Lerp((float)PreMipWeightData00[3], (float)PreMipWeightData10[3], PreMipTexFracX1),
							Math::Lerp((float)PreMipWeightData10[3], (float)PreMipWeightData11[3], PreMipTexFracX1),
							PreMipTexFracY1));
					}
				}
				PreMipTexSizeU = MipTexSizeU;
				PreMipTexSizeV = MipTexSizeV;

				PreMipX1SubsectionCoord = MipX1SubsectionCoord;
				PreMipX2SubsectionCoord = MipX2SubsectionCoord;
				PreMipY1SubsectionCoord = MipY1SubsectionCoord;
				PreMipY2SubsectionCoord = MipY2SubsectionCoord;

				PreMipNumSubsectionQuad = MipNumSubsectionQuad;
				InvPreMipNumsubsectionQuad = InvMipNumSubsectionQuad;
			}
		}
	}
}