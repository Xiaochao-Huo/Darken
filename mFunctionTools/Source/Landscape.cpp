#include "mLandscape.h"
#include "mLandscapeBrush.h"

LandscapePaintLayerInfo::LandscapePaintLayerInfo(std::shared_ptr<Texture> paintTexture, bool bWeightNormalBlend)
{
	PaintTexture = paintTexture;
	isWeightNormalBlend = bWeightNormalBlend;
	Index = -1;
	ChannelInWeightTexutre = -1;
}

LandscapePaintLayerInfo::~LandscapePaintLayerInfo()
{
}

std::shared_ptr<Texture> LandscapePaintLayerInfo::GetPaintTexture()
{
	return PaintTexture;
}

LandscapeGBufferRenderPipeline::LandscapeGBufferRenderPipeline()
{
	HitPositionWorld = glm::vec3(0.0, 0.0, 0.0);
}

LandscapeGBufferRenderPipeline::~LandscapeGBufferRenderPipeline()
{
	if (PixelDataGBufferWorldPosition)
	{
		delete[] PixelDataGBufferWorldPosition;
		PixelDataGBufferWorldPosition = nullptr;
	}
}

void LandscapeGBufferRenderPipeline::Init()
{	
	gl::GenFramebuffers(1, &FrameBufferObject);
	gl::BindFramebuffer(GL_FRAMEBUFFER, FrameBufferObject);
	
	gl::GenTextures(1, &Tex_WorldPosition);
	gl::BindTexture(GL_TEXTURE_2D, Tex_WorldPosition);
	gl::TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, _ScreenWidth, _ScreenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	gl::FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Tex_WorldPosition, 0);

	unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0 };

	gl::DrawBuffers(1, attachments);

	gl::GenRenderbuffers(1, &RBO_Depth);
	gl::BindRenderbuffer(GL_RENDERBUFFER, RBO_Depth);
	gl::RenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, _ScreenWidth, _ScreenHeight);
	gl::FramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RBO_Depth);

	if (gl::CheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		pvr::utils::throwOnGlError("FrameBufferError");
	}

	gl::BindFramebuffer(GL_FRAMEBUFFER, 0);

	gl::GenBuffers(1, &PixelBufferObjectForWorldPosition);
	gl::BindBuffer(GL_PIXEL_PACK_BUFFER, PixelBufferObjectForWorldPosition);
	gl::BufferData(GL_PIXEL_PACK_BUFFER, sizeof(float) * 4, NULL, GL_STATIC_READ);
	gl::BindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	PixelDataGBufferWorldPosition = new float[4];
}

void LandscapeGBufferRenderPipeline::HandleOffScreenPixelsAsynBegin()
{
	gl::BindFramebuffer(GL_READ_FRAMEBUFFER, FrameBufferObject);
	gl::ReadBuffer(GL_COLOR_ATTACHMENT0);

	gl::BindBuffer(GL_PIXEL_PACK_BUFFER, PixelBufferObjectForWorldPosition);	
	gl::ReadPixels(_MousePositionX, _ScreenHeight - _MousePositionY, 1, 1, GL_RGBA, GL_FLOAT, NULL); //Asyn ReadPixel to PBO
	int error = gl::GetError(); //不知道是不是接口bug，读取float型的数据会出现error，但是数据还是读到了，务必设置为RGBA格式，否则读不到...
	gl::BindBuffer(GL_PIXEL_PACK_BUFFER, 0);

	gl::BindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

void LandscapeGBufferRenderPipeline::HandleOffScreenPixelsAsynEnd()
{
	gl::BindFramebuffer(GL_READ_FRAMEBUFFER, FrameBufferObject);
	gl::BindBuffer(GL_PIXEL_PACK_BUFFER, PixelBufferObjectForWorldPosition);

	memcpy(PixelDataGBufferWorldPosition, gl::MapBufferRange(GL_PIXEL_PACK_BUFFER, 0, sizeof(float) * 4, GL_MAP_READ_BIT),  sizeof(float) * 4);  //PBO has prepared data done, read them
	gl::UnmapBuffer(GL_PIXEL_PACK_BUFFER);

	gl::BindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	gl::BindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

void LandscapeGBufferRenderPipeline::ActivePipeline()
{
	gl::BindFramebuffer(GL_FRAMEBUFFER, FrameBufferObject);
	gl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void LandscapeGBufferRenderPipeline::ClosePipeline()
{
	gl::BindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::vec3 LandscapeGBufferRenderPipeline::GetMouseHitPosition()
{
	if (PixelDataGBufferWorldPosition[0] > -90000.0f)
	{
		HitPositionWorld = glm::vec3(PixelDataGBufferWorldPosition[0], PixelDataGBufferWorldPosition[1], PixelDataGBufferWorldPosition[2]);
	}
	return HitPositionWorld;
}

Landscape::Landscape(glm::vec3 position,
	glm::vec3 scale,
	glm::vec2 numComponent,
	glm::uint numSubsection,
	glm::uint numSubsectionQuad,
	std::shared_ptr<Material> gBufferMaterial,
	std::shared_ptr<Material> linghtingMaterial,
	std::shared_ptr<Material> brushCircleMaterial,
	std::shared_ptr<Material> wireframeMaterial)
{
	NumComponent = numComponent;
	NumSubsection = numSubsection;
	NumSubsectionQuad = numSubsectionQuad;

	ObjectTransform.SetTransform(position - glm::vec3(numComponent.x * numSubsection * numSubsectionQuad, numComponent.y * numSubsection * numSubsectionQuad, 0.0) / 2.0f * scale, glm::vec3(0.0, 0.0, 0.0), scale);

	//先创建Component需要使用的公共对象，在后面进行初始化
	LandscapeGBufferPipeline = std::unique_ptr<LandscapeGBufferRenderPipeline>(new LandscapeGBufferRenderPipeline());
	LandscapeGBufferMaterial = gBufferMaterial;
	LandscapeLightingMaterial = linghtingMaterial;
	LandscapeBrushCircleMaterial = brushCircleMaterial;
	LandscapeWireframeMaterial = wireframeMaterial;
	MaterialUniformData = std::shared_ptr<LandscapeMaterialUniformData>(new LandscapeMaterialUniformData());
	LandscapeComponentSharedBuffer = std::shared_ptr<LandscapeSharedBuffer>(new LandscapeSharedBuffer());
	LandscapeRenderMaterial = std::make_shared<std::shared_ptr<Material>>(std::shared_ptr<Material>(new Material()));
	
	LandscapeGBufferPipeline->Init();


	glm::vec2 posOffsetXY = glm::vec2(position.x, position.y);
	for (int ComponentY = 0; ComponentY < NumComponent.y; ComponentY++)
	{
		for (int ComponentX = 0; ComponentX < NumComponent.x; ComponentX++)
		{
			const int BaseX = posOffsetXY.x + ComponentX * NumSubsectionQuad * NumSubsection;
			const int BaseY = posOffsetXY.y + ComponentY * NumSubsectionQuad * NumSubsection;
			std::shared_ptr<LandscapeComponent> mLandScapeComponent(new LandscapeComponent());
			//初始化Component时先绑定公共对象，此时公共对象未初始化
			mLandScapeComponent->Init(BaseX, BaseY,
				NumSubsection, NumSubsectionQuad,
				this,
				MaterialUniformData,
				LandscapeComponentSharedBuffer,
				LandscapeRenderMaterial);
			LandScapeComponentMap.insert(std::pair<int, std::shared_ptr<LandscapeComponent>>(GetComponentMapHashKey(ComponentX, ComponentY), mLandScapeComponent));
		}
	}

	for (int ComponentY = 0; ComponentY < NumComponent.y; ComponentY++)
	{
		for (int ComponentX = 0; ComponentX < NumComponent.x; ComponentX++)
		{
			BindSubsectionNeighbors(ComponentX, ComponentY);
		}
	}

	const int VertsX = NumComponent.x * NumSubsection * NumSubsectionQuad + 1;
	const int VertsY = NumComponent.y * NumSubsection * NumSubsectionQuad + 1;
	std::vector<unsigned short> HeightData(VertsX * VertsY);
	for (int i = 0; i < HeightData.size(); ++i)
	{
		HeightData[i] = 32768;
	}

	const int VertsPerComponent = NumSubsection * (NumSubsectionQuad + 1);
	const int NumComponentsPerHeightTexture = HEIGHT_TEXTURE_MAX_SIZE / VertsPerComponent;

	int NumHeightTextureX = glm::ceil(NumComponent.x / (float)NumComponentsPerHeightTexture);
	int NumHeightTextureY = glm::ceil(NumComponent.y / (float)NumComponentsPerHeightTexture);

	int NumFinalTextureComponentX = NumComponent.x - (NumHeightTextureX - 1) * NumComponentsPerHeightTexture;
	int NumFinalTextureComponentY = NumComponent.y - (NumHeightTextureY - 1) * NumComponentsPerHeightTexture;

	std::vector<std::shared_ptr<Texture>> HeightTextures(NumHeightTextureX * NumHeightTextureY);
	for (int HeightTexIndexY = 0; HeightTexIndexY < NumHeightTextureY; HeightTexIndexY++)
	{
		for (int HeightTexIndexX = 0; HeightTexIndexX < NumHeightTextureX; HeightTexIndexX++)
		{
			HeightTextures[HeightTexIndexX + HeightTexIndexY * NumHeightTextureX] = std::shared_ptr<Texture>(new Texture());
			int TexSizeU = (HeightTexIndexX == NumHeightTextureX - 1) ? NumFinalTextureComponentX * VertsPerComponent : HEIGHT_TEXTURE_MAX_SIZE;
			int TexSizeV = (HeightTexIndexY == NumHeightTextureY - 1) ? NumFinalTextureComponentY * VertsPerComponent : HEIGHT_TEXTURE_MAX_SIZE;
			HeightTextures[HeightTexIndexX + HeightTexIndexY * NumHeightTextureX]->InitMipRawTextureData(TexSizeU, TexSizeV, TextureDataTypeFormat::RGBA8);
		}
	}

	std::vector<glm::vec3> VertexNormals(VertsX * VertsY);
	for (int QuadY = 0; QuadY < VertsY - 1; QuadY++)
	{
		for (int QuadX = 0; QuadX < VertsX - 1; QuadX++)
		{
			const glm::vec3 Vert00 = glm::vec3(0.0f, 0.0f, (HeightData[QuadX + 0 + (QuadY + 0) * VertsX] - Landscape_HeightHalfValue) * LANDSCAPE_ZSCALE) * scale;
			const glm::vec3 Vert01 = glm::vec3(0.0f, 1.0f, (HeightData[QuadX + 0 + (QuadY + 1) * VertsX] - Landscape_HeightHalfValue) * LANDSCAPE_ZSCALE) * scale;
			const glm::vec3 Vert10 = glm::vec3(1.0f, 0.0f, (HeightData[QuadX + 1 + (QuadY + 0) * VertsX] - Landscape_HeightHalfValue) * LANDSCAPE_ZSCALE) * scale;
			const glm::vec3 Vert11 = glm::vec3(1.0f, 1.0f, (HeightData[QuadX + 1 + (QuadY + 1) * VertsX] - Landscape_HeightHalfValue) * LANDSCAPE_ZSCALE) * scale;

			const glm::vec3 FaceNormal1 = glm::normalize(glm::cross(Vert00 - Vert10, Vert10 - Vert11));
			const glm::vec3 FaceNormal2 = glm::normalize(glm::cross(Vert11 - Vert01, Vert01 - Vert00));

			VertexNormals[(QuadX + 1 + VertsX * (QuadY + 0))] += FaceNormal1;
			VertexNormals[(QuadX + 0 + VertsX * (QuadY + 1))] += FaceNormal2;
			VertexNormals[(QuadX + 0 + VertsX * (QuadY + 0))] += FaceNormal1 + FaceNormal2;
			VertexNormals[(QuadX + 1 + VertsX * (QuadY + 1))] += FaceNormal1 + FaceNormal2;
		}
	}

	for (int ComponentY = 0; ComponentY < NumComponent.y; ComponentY++)
	{
		const int HeightTexIndexY = ComponentY / NumComponentsPerHeightTexture;
		const int HeightTexOffsetY = (ComponentY - HeightTexIndexY * NumComponentsPerHeightTexture) * NumSubsection * (NumSubsectionQuad + 1);
		for (int ComponentX = 0; ComponentX < NumComponent.x; ComponentX++)
		{
			const int HeightTexIndexX = ComponentX / NumComponentsPerHeightTexture;
			const int HeightTexOffsetX = (ComponentX - HeightTexIndexX * NumComponentsPerHeightTexture) * NumSubsection * (NumSubsectionQuad + 1);

			int TexSizeU = HeightTextures[HeightTexIndexX + HeightTexIndexY * NumHeightTextureX]->GetWidth();
			int TexSizeV = HeightTextures[HeightTexIndexX + HeightTexIndexY * NumHeightTextureX]->GetHeight();
			
			std::shared_ptr<LandscapeComponent> mLandscapeComponent = LandScapeComponentMap[GetComponentMapHashKey(ComponentX, ComponentY)];

			mLandscapeComponent->HeightTextureScaleBias = glm::vec4(1.0f / (float)TexSizeU, 1.0f / (float)TexSizeV, (float)HeightTexOffsetX / (float)TexSizeU, (float)HeightTexOffsetY / (float)TexSizeV);
			mLandscapeComponent->HeightTexture = HeightTextures[HeightTexIndexX + HeightTexIndexY * NumHeightTextureX];

			for (int SubsectionY = 0; SubsectionY < NumSubsection; SubsectionY++)
			{
				for (int SubsectionX = 0; SubsectionX < NumSubsection; SubsectionX++)
				{
					for (int SubY = 0; SubY <= NumSubsectionQuad; SubY++)
					{
						for (int SubX = 0; SubX <= NumSubsectionQuad; SubX++)
						{
							//In ComponentCoord
							const int CoordX = SubsectionX * NumSubsectionQuad + SubX;
							const int CoordY = SubsectionY * NumSubsectionQuad + SubY;
							const int TexX = SubsectionX * (NumSubsectionQuad + 1) + SubX;
							const int TexY = SubsectionY * (NumSubsectionQuad + 1) + SubY;

							const int HeightTexDataIndex = ((HeightTexOffsetX + TexX) + (HeightTexOffsetY + TexY) * TexSizeU) * 4;
							const unsigned short HeightValue = HeightData[VertsX * (mLandscapeComponent->GetBaseY() + CoordY) + mLandscapeComponent->GetBaseX() + CoordX];
							const glm::vec3 Normal = glm::normalize(VertexNormals[VertsX * (mLandscapeComponent->GetBaseY() + CoordY) + mLandscapeComponent->GetBaseX() + CoordX]);
							const unsigned char R = HeightValue >> 8;
							const unsigned char G = HeightValue & 255;
							const unsigned char B = static_cast<unsigned char>(std::round(127.5f * (Normal.x + 1.0f)));
							const unsigned char A = static_cast<unsigned char>(std::round(127.5f * (Normal.y + 1.0f)));

							mLandscapeComponent->HeightTexture->GetRawDataPtr(0)->MipData[HeightTexDataIndex + 0] = R;
							mLandscapeComponent->HeightTexture->GetRawDataPtr(0)->MipData[HeightTexDataIndex + 1] = G;
							mLandscapeComponent->HeightTexture->GetRawDataPtr(0)->MipData[HeightTexDataIndex + 2] = B;
							mLandscapeComponent->HeightTexture->GetRawDataPtr(0)->MipData[HeightTexDataIndex + 3] = A;
							
							mLandscapeComponent->SurroundBox += glm::vec3(CoordX, CoordY, (HeightValue - Landscape_HeightHalfValue) * LANDSCAPE_ZSCALE);
						}
					}
				}
			}
		}
	}

	for (std::hash_map<int, std::shared_ptr<LandscapeComponent>>::iterator it = LandScapeComponentMap.begin(); it != LandScapeComponentMap.end(); it++)
	{
		it->second->GenerateHeightTextureMipData();
		it->second->HeightTexture->CreateGPUObject(TextureParameter::Linear_Mip_Nearest, TextureParameter::Linear, TextureParameter::Clamp_To_Edge, TextureParameter::Clamp_To_Edge);
	}

	//初始化公共对象SharedBuffer
	LandscapeComponentSharedBuffer->Create(NumSubsection, NumSubsectionQuad);
	LandscapeComponentSharedBuffer->CreateGPUObject();
	LandscapeComponentSharedBuffer->ConfirmSubsectionIndex();

	LandscapeComponentSharedBuffer->CreateWholeWireframeDataObject();

	//初始化公共对象MaterialUniformDataPtr
	BindMaterialDataPtr();

	//初始化设置其它值
	LOD0DistributionSetting = 1.75f;
	OtherLODDistributionSetting = 2.0f;
	ThresholdSectionRatioSquared = glm::pow(0.6f, 2.0f);
	float CurrentScreenRatio = 1.0f;
	float ScreenRatioDivider = LOD0DistributionSetting;

	int MaxLOD = glm::log2((float)(NumSubsectionQuad + 1));
	LODScreenRatios.reserve(MaxLOD);
	LODScreenRatios.resize(MaxLOD);
	LODScreenRatios[0] = glm::pow(CurrentScreenRatio, 2.0f);

	CurrentScreenRatio /= ScreenRatioDivider;
	ScreenRatioDivider = OtherLODDistributionSetting;

	for (int LODIndex = 1; LODIndex < MaxLOD; LODIndex++)
	{
		LODScreenRatios[LODIndex] = glm::pow(CurrentScreenRatio, 2.0f);
		CurrentScreenRatio /= ScreenRatioDivider;
	}
}

void Landscape::BindBrush(std::weak_ptr<LandscapeBrushBase> brush)
{
	LandscapeBrush = brush;
}

std::shared_ptr<LandscapeBrushBase> Landscape::GetCurrentBrush()
{
	return LandscapeBrush.lock();
}

Landscape::Landscape()
{
}

Landscape::~Landscape()
{
}

void Landscape::CheckWhetherNeedClip()
{
	for (std::hash_map<int, std::shared_ptr<LandscapeComponent>>::iterator it = LandScapeComponentMap.begin(); it != LandScapeComponentMap.end(); it++)
	{
		it->second->CheckWhetherNeedClip();
	}
}

void Landscape::CalculateLOD()
{
	for (std::hash_map<int, std::shared_ptr<LandscapeComponent>>::iterator it = LandScapeComponentMap.begin(); it != LandScapeComponentMap.end(); it++)
	{
		if (!it->second->bNeedClip)
		{
			it->second->CalculateLOD();
		}
		else
		{
			it->second->bHaveCalLOD = false;
		}
	}
}

void Landscape::RenderGBufferPipeline()
{
	LandscapeGBufferPipeline->ActivePipeline();
	LandscapeComponentSharedBuffer->UseBaseVAOs();
	*LandscapeRenderMaterial = LandscapeGBufferMaterial;
	for (std::hash_map<int, std::shared_ptr<LandscapeComponent>>::iterator it = LandScapeComponentMap.begin(); it != LandScapeComponentMap.end(); it++)
	{
		if (!it->second->bNeedClip)
		{
			it->second->Draw();
		}
	}
	LandscapeGBufferPipeline->ClosePipeline();
}

void Landscape::RenderBasePassPipeline()
{
	if (_Scene->GetCurrentViewMode() == ModeState::ViewMode::LightingMode)
	{
		LandscapeComponentSharedBuffer->UseBaseVAOs();
		*LandscapeRenderMaterial = LandscapeLightingMaterial;

		MaterialUniformData->LightColor = _Scene->GetLight(LightIndex::DirectionalLight0)->GetColor();
		MaterialUniformData->LightDirWorld = _Scene->GetLight(LightIndex::DirectionalLight0)->GetDirection();
		MaterialUniformData->ViewPosition = _Scene->GetCamera(CameraIndex::MainCamera)->GetPosition();

		MaterialUniformData->PixelShaderPaintTexChannelIndex0 = glm::vec4(1.0, 0.0, 0.0, 0.0);
		MaterialUniformData->PixelShaderPaintTexChannelIndex1 = glm::vec4(0.0, 1.0, 0.0, 0.0);
		MaterialUniformData->PixelShaderPaintTexChannelIndex2 = glm::vec4(0.0, 0.0, 1.0, 0.0);
		MaterialUniformData->PixelShaderPaintTexChannelIndex3 = glm::vec4(0.0, 0.0, 0.0, 1.0);
		MaterialUniformData->PixelShaderPaintTexChannelIndex4 = glm::vec4(1.0, 0.0, 0.0, 0.0);
		MaterialUniformData->PixelShaderPaintTexChannelIndex5 = glm::vec4(0.0, 1.0, 0.0, 0.0);

		MaterialUniformData->PaintTexture1ID = Layers[1]->GetPaintTexture()->GPUId;
		MaterialUniformData->PaintTexture2ID = Layers[2]->GetPaintTexture()->GPUId;
		MaterialUniformData->PaintTexture3ID = Layers[3]->GetPaintTexture()->GPUId;
		MaterialUniformData->PaintTexture4ID = Layers[4]->GetPaintTexture()->GPUId;
		MaterialUniformData->PaintTexture5ID = Layers[5]->GetPaintTexture()->GPUId;
	}
	else if (_Scene->GetCurrentViewMode() == ModeState::ViewMode::WireframeMode)
	{
		LandscapeComponentSharedBuffer->UseTrickWireframeVAOs();
		*LandscapeRenderMaterial = LandscapeWireframeMaterial;
	}

	for (std::hash_map<int, std::shared_ptr<LandscapeComponent>>::iterator it = LandScapeComponentMap.begin(); it != LandScapeComponentMap.end(); it++)
	{
		if (!it->second->bNeedClip)
		{
			it->second->Draw();
		}
	}
}

void Landscape::RenderBrushCirclePipeline()
{
	gl::Enable(GL_BLEND);
	gl::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	LandscapeComponentSharedBuffer->UseBaseVAOs();
	*LandscapeRenderMaterial = LandscapeBrushCircleMaterial;

	MaterialUniformData->HitPositionWorld = LandscapeGBufferPipeline->GetMouseHitPosition();
	LandscapeBrush.lock()->SetHitPositionWorld(MaterialUniformData->HitPositionWorld);
	MaterialUniformData->BrushParameters_Sizes = LandscapeBrush.lock()->GetBrushParameters_Sizes();
	
	for (std::hash_map<int, std::shared_ptr<LandscapeComponent>>::iterator it = LandScapeComponentMap.begin(); it != LandScapeComponentMap.end(); it++)
	{
		if (!it->second->bNeedClip)
		{
			it->second->Draw();
		}
	}

	gl::Disable(GL_BLEND);
}

void Landscape::Draw()
{
	MaterialUniformData->ViewMatrix = _Scene->GetCamera(CameraIndex::MainCamera)->GetViewMatrix();
	MaterialUniformData->ProjectMatrix = _Scene->GetCamera(CameraIndex::MainCamera)->GetProjectionMatrix();

	CalculateLOD();

	RenderGBufferPipeline();

	LandscapeGBufferPipeline->HandleOffScreenPixelsAsynBegin(); //Asyn DMA processing， now do something else

	RenderBasePassPipeline();

	LandscapeGBufferPipeline->HandleOffScreenPixelsAsynEnd(); //Asyn DMA may be done, handle them

	RenderBrushCirclePipeline();
}

void Landscape::BindSubsectionNeighbors(int ComponentX, int ComponentY)
{
	std::shared_ptr<LandscapeComponent> mLandscapeComponent = LandScapeComponentMap[GetComponentMapHashKey(ComponentX, ComponentY)];

	if (NumSubsection > 1)
	{
		for (int SubY = 0; SubY < NumSubsection; SubY++)
		{
			for (int SubX = 0; SubX < NumSubsection; SubX++)
			{
				int currentSubsection = SubY * NumSubsection + SubX;
				for (int SubsectionIndex = 0; SubsectionIndex < SUBSECTIONNUM; SubsectionIndex++)
				{
					int subSectionOffsetX = SubSectionValues[currentSubsection][SubsectionIndex].SubSectionOffsetX;
					int subSectionOffsetY = SubSectionValues[currentSubsection][SubsectionIndex].SubSectionOffsetY;
					int desireSubsectionX = subSectionOffsetX + SubX;
					int desireSubsectionY = subSectionOffsetY + SubY;
					bool isInsideComponent = SubSectionValues[currentSubsection][SubsectionIndex].InsideComponent;
					if (!isInsideComponent)
					{
						int desireComponentX = ComponentX;
						int desireComponentY = ComponentY;
						switch (SubsectionIndex)
						{
						case 0: desireComponentY -= 1; break;
						case 1: desireComponentX -= 1; break;
						case 2: desireComponentX += 1; break;
						case 3: desireComponentY += 1; break;
						default:
							break;
						}
						std::hash_map<int, std::shared_ptr<LandscapeComponent>>::iterator it = LandScapeComponentMap.find(GetComponentMapHashKey(desireComponentX, desireComponentY));
						if (it != LandScapeComponentMap.end())
						{
							std::shared_ptr<LandscapeComponent> mLandscapeNeighborComponent = LandScapeComponentMap[GetComponentMapHashKey(desireComponentX, desireComponentY)];
							mLandscapeComponent->Subsections[currentSubsection]->NeighborSubsection[SubsectionIndex] = mLandscapeNeighborComponent->Subsections[desireSubsectionY * NumSubsection + desireSubsectionX];
						}
					}
					else
					{
						mLandscapeComponent->Subsections[currentSubsection]->NeighborSubsection[SubsectionIndex] = mLandscapeComponent->Subsections[desireSubsectionY * NumSubsection + desireSubsectionX];
					}
				}
			}
		}
	}
	else
	{
		for (int SubsectionIndex = 0; SubsectionIndex < SUBSECTIONNUM; SubsectionIndex++)
		{
			int desireComponentX = ComponentX;
			int desireComponentY = ComponentY;
			switch (SubsectionIndex)
			{
			case 0: desireComponentY -= 1; break;
			case 1: desireComponentX -= 1; break;
			case 2: desireComponentX += 1; break;
			case 3: desireComponentY += 1; break;
			default:
				break;
			}
			std::hash_map<int, std::shared_ptr<LandscapeComponent>>::iterator it = LandScapeComponentMap.find(GetComponentMapHashKey(desireComponentX, desireComponentY));
			if (it != LandScapeComponentMap.end())
			{
				std::shared_ptr<LandscapeComponent> mLandscapeNeighborComponent = LandScapeComponentMap[GetComponentMapHashKey(desireComponentX, desireComponentY)];
				mLandscapeComponent->Subsections[0]->NeighborSubsection[SubsectionIndex] = mLandscapeNeighborComponent->Subsections[0];
			}
		}
	}
}

void Landscape::BindMaterialDataPtr()
{
	LandscapeGBufferMaterial->MaterialProgram->Uniforms_Texture["HeightTexture"]->IDPtr = &MaterialUniformData->HeightTextureID;
	LandscapeGBufferMaterial->MaterialProgram->Uniforms_NotTexture["ModelMatrix"]->DataPtr = glm::value_ptr(MaterialUniformData->ModelMatrix);
	LandscapeGBufferMaterial->MaterialProgram->Uniforms_NotTexture["ViewMatrix"]->DataPtr = glm::value_ptr(MaterialUniformData->ViewMatrix);
	LandscapeGBufferMaterial->MaterialProgram->Uniforms_NotTexture["ProjectMatrix"]->DataPtr = glm::value_ptr(MaterialUniformData->ProjectMatrix);
	LandscapeGBufferMaterial->MaterialProgram->Uniforms_NotTexture["LodBias"]->DataPtr = glm::value_ptr(MaterialUniformData->LodBias);
	LandscapeGBufferMaterial->MaterialProgram->Uniforms_NotTexture["LodValues"]->DataPtr = glm::value_ptr(MaterialUniformData->LodValues);
	LandscapeGBufferMaterial->MaterialProgram->Uniforms_NotTexture["SectionLods"]->DataPtr = glm::value_ptr(MaterialUniformData->SectionLods);
	LandscapeGBufferMaterial->MaterialProgram->Uniforms_NotTexture["NeighborSectionsLod"]->DataPtr = glm::value_ptr(MaterialUniformData->NeighborSectionsLod);
	LandscapeGBufferMaterial->MaterialProgram->Uniforms_NotTexture["LandscapeParameters_SubsectionSizeVertsLayerUVPan"]->DataPtr = glm::value_ptr(MaterialUniformData->LandscapeParameters_SubsectionSizeVertsLayerUVPan);
	LandscapeGBufferMaterial->MaterialProgram->Uniforms_NotTexture["LandscapeParameters_SubsectionOffsetParams"]->DataPtr = glm::value_ptr(MaterialUniformData->LandscapeParameters_SubsectionOffsetParams);
	LandscapeGBufferMaterial->MaterialProgram->Uniforms_NotTexture["LandscapeParameters_HeightTextureUVScaleBias"]->DataPtr = glm::value_ptr(MaterialUniformData->LandscapeParameters_HeightTextureUVScaleBias);

	LandscapeBrushCircleMaterial->MaterialProgram->Uniforms_Texture["HeightTexture"]->IDPtr = &MaterialUniformData->HeightTextureID;
	LandscapeBrushCircleMaterial->MaterialProgram->Uniforms_NotTexture["ModelMatrix"]->DataPtr = glm::value_ptr(MaterialUniformData->ModelMatrix);
	LandscapeBrushCircleMaterial->MaterialProgram->Uniforms_NotTexture["ViewMatrix"]->DataPtr = glm::value_ptr(MaterialUniformData->ViewMatrix);
	LandscapeBrushCircleMaterial->MaterialProgram->Uniforms_NotTexture["ProjectMatrix"]->DataPtr = glm::value_ptr(MaterialUniformData->ProjectMatrix);
	LandscapeBrushCircleMaterial->MaterialProgram->Uniforms_NotTexture["LodBias"]->DataPtr = glm::value_ptr(MaterialUniformData->LodBias);
	LandscapeBrushCircleMaterial->MaterialProgram->Uniforms_NotTexture["LodValues"]->DataPtr = glm::value_ptr(MaterialUniformData->LodValues);
	LandscapeBrushCircleMaterial->MaterialProgram->Uniforms_NotTexture["SectionLods"]->DataPtr = glm::value_ptr(MaterialUniformData->SectionLods);
	LandscapeBrushCircleMaterial->MaterialProgram->Uniforms_NotTexture["NeighborSectionsLod"]->DataPtr = glm::value_ptr(MaterialUniformData->NeighborSectionsLod);
	LandscapeBrushCircleMaterial->MaterialProgram->Uniforms_NotTexture["LandscapeParameters_SubsectionSizeVertsLayerUVPan"]->DataPtr = glm::value_ptr(MaterialUniformData->LandscapeParameters_SubsectionSizeVertsLayerUVPan);
	LandscapeBrushCircleMaterial->MaterialProgram->Uniforms_NotTexture["LandscapeParameters_SubsectionOffsetParams"]->DataPtr = glm::value_ptr(MaterialUniformData->LandscapeParameters_SubsectionOffsetParams);
	LandscapeBrushCircleMaterial->MaterialProgram->Uniforms_NotTexture["LandscapeParameters_HeightTextureUVScaleBias"]->DataPtr = glm::value_ptr(MaterialUniformData->LandscapeParameters_HeightTextureUVScaleBias);
	LandscapeBrushCircleMaterial->MaterialProgram->Uniforms_NotTexture["HitWorldPosition"]->DataPtr = glm::value_ptr(MaterialUniformData->HitPositionWorld);
	LandscapeBrushCircleMaterial->MaterialProgram->Uniforms_NotTexture["BrushParameters_Sizes"]->DataPtr = glm::value_ptr(MaterialUniformData->BrushParameters_Sizes);

	LandscapeWireframeMaterial->MaterialProgram->Uniforms_Texture["HeightTexture"]->IDPtr = &MaterialUniformData->HeightTextureID;
	LandscapeWireframeMaterial->MaterialProgram->Uniforms_NotTexture["ModelMatrix"]->DataPtr = glm::value_ptr(MaterialUniformData->ModelMatrix);
	LandscapeWireframeMaterial->MaterialProgram->Uniforms_NotTexture["ViewMatrix"]->DataPtr = glm::value_ptr(MaterialUniformData->ViewMatrix);
	LandscapeWireframeMaterial->MaterialProgram->Uniforms_NotTexture["ProjectMatrix"]->DataPtr = glm::value_ptr(MaterialUniformData->ProjectMatrix);
	LandscapeWireframeMaterial->MaterialProgram->Uniforms_NotTexture["LodBias"]->DataPtr = glm::value_ptr(MaterialUniformData->LodBias);
	LandscapeWireframeMaterial->MaterialProgram->Uniforms_NotTexture["LodValues"]->DataPtr = glm::value_ptr(MaterialUniformData->LodValues);
	LandscapeWireframeMaterial->MaterialProgram->Uniforms_NotTexture["SectionLods"]->DataPtr = glm::value_ptr(MaterialUniformData->SectionLods);
	LandscapeWireframeMaterial->MaterialProgram->Uniforms_NotTexture["NeighborSectionsLod"]->DataPtr = glm::value_ptr(MaterialUniformData->NeighborSectionsLod);
	LandscapeWireframeMaterial->MaterialProgram->Uniforms_NotTexture["LandscapeParameters_SubsectionSizeVertsLayerUVPan"]->DataPtr = glm::value_ptr(MaterialUniformData->LandscapeParameters_SubsectionSizeVertsLayerUVPan);
	LandscapeWireframeMaterial->MaterialProgram->Uniforms_NotTexture["LandscapeParameters_SubsectionOffsetParams"]->DataPtr = glm::value_ptr(MaterialUniformData->LandscapeParameters_SubsectionOffsetParams);
	LandscapeWireframeMaterial->MaterialProgram->Uniforms_NotTexture["LandscapeParameters_HeightTextureUVScaleBias"]->DataPtr = glm::value_ptr(MaterialUniformData->LandscapeParameters_HeightTextureUVScaleBias);

	LandscapeLightingMaterial->MaterialProgram->Uniforms_Texture["HeightTexture"]->IDPtr = &MaterialUniformData->HeightTextureID;
	LandscapeLightingMaterial->MaterialProgram->Uniforms_NotTexture["ViewPosition"]->DataPtr = glm::value_ptr(MaterialUniformData->ViewPosition);
	LandscapeLightingMaterial->MaterialProgram->Uniforms_NotTexture["LightDirWorld"]->DataPtr = glm::value_ptr(MaterialUniformData->LightDirWorld);
	LandscapeLightingMaterial->MaterialProgram->Uniforms_NotTexture["LightColor"]->DataPtr = glm::value_ptr(MaterialUniformData->LightColor);
	LandscapeLightingMaterial->MaterialProgram->Uniforms_NotTexture["ModelMatrix"]->DataPtr = glm::value_ptr(MaterialUniformData->ModelMatrix);
	LandscapeLightingMaterial->MaterialProgram->Uniforms_NotTexture["ViewMatrix"]->DataPtr = glm::value_ptr(MaterialUniformData->ViewMatrix);
	LandscapeLightingMaterial->MaterialProgram->Uniforms_NotTexture["ProjectMatrix"]->DataPtr = glm::value_ptr(MaterialUniformData->ProjectMatrix);
	LandscapeLightingMaterial->MaterialProgram->Uniforms_NotTexture["LodBias"]->DataPtr = glm::value_ptr(MaterialUniformData->LodBias);
	LandscapeLightingMaterial->MaterialProgram->Uniforms_NotTexture["LodValues"]->DataPtr = glm::value_ptr(MaterialUniformData->LodValues);
	LandscapeLightingMaterial->MaterialProgram->Uniforms_NotTexture["SectionLods"]->DataPtr = glm::value_ptr(MaterialUniformData->SectionLods);
	LandscapeLightingMaterial->MaterialProgram->Uniforms_NotTexture["NeighborSectionsLod"]->DataPtr = glm::value_ptr(MaterialUniformData->NeighborSectionsLod);
	LandscapeLightingMaterial->MaterialProgram->Uniforms_NotTexture["LandscapeParameters_SubsectionSizeVertsLayerUVPan"]->DataPtr = glm::value_ptr(MaterialUniformData->LandscapeParameters_SubsectionSizeVertsLayerUVPan);
	LandscapeLightingMaterial->MaterialProgram->Uniforms_NotTexture["LandscapeParameters_SubsectionOffsetParams"]->DataPtr = glm::value_ptr(MaterialUniformData->LandscapeParameters_SubsectionOffsetParams);
	LandscapeLightingMaterial->MaterialProgram->Uniforms_NotTexture["LandscapeParameters_HeightTextureUVScaleBias"]->DataPtr = glm::value_ptr(MaterialUniformData->LandscapeParameters_HeightTextureUVScaleBias);
	LandscapeLightingMaterial->MaterialProgram->Uniforms_NotTexture["LandscapeParameters_WeightTextureUVScaleBias"]->DataPtr = glm::value_ptr(MaterialUniformData->LandscapeParameters_WeightTextureUVScaleBias);
	LandscapeLightingMaterial->MaterialProgram->Uniforms_Texture["WeightTex0"]->IDPtr = &MaterialUniformData->WeightTexture0ID;
	LandscapeLightingMaterial->MaterialProgram->Uniforms_Texture["WeightTex1"]->IDPtr = &MaterialUniformData->WeightTexture1ID;
	LandscapeLightingMaterial->MaterialProgram->Uniforms_Texture["PaintTex0"]->IDPtr = &MaterialUniformData->PaintTexture0ID;
	LandscapeLightingMaterial->MaterialProgram->Uniforms_Texture["PaintTex1"]->IDPtr = &MaterialUniformData->PaintTexture1ID;
	LandscapeLightingMaterial->MaterialProgram->Uniforms_Texture["PaintTex2"]->IDPtr = &MaterialUniformData->PaintTexture2ID;
	LandscapeLightingMaterial->MaterialProgram->Uniforms_Texture["PaintTex3"]->IDPtr = &MaterialUniformData->PaintTexture3ID;
	LandscapeLightingMaterial->MaterialProgram->Uniforms_Texture["PaintTex4"]->IDPtr = &MaterialUniformData->PaintTexture4ID;
	LandscapeLightingMaterial->MaterialProgram->Uniforms_Texture["PaintTex5"]->IDPtr = &MaterialUniformData->PaintTexture5ID;
	LandscapeLightingMaterial->MaterialProgram->Uniforms_NotTexture["PaintChTex0"]->DataPtr = glm::value_ptr(MaterialUniformData->PixelShaderPaintTexChannelIndex0);
	LandscapeLightingMaterial->MaterialProgram->Uniforms_NotTexture["PaintChTex1"]->DataPtr = glm::value_ptr(MaterialUniformData->PixelShaderPaintTexChannelIndex1);
	LandscapeLightingMaterial->MaterialProgram->Uniforms_NotTexture["PaintChTex2"]->DataPtr = glm::value_ptr(MaterialUniformData->PixelShaderPaintTexChannelIndex2);
	LandscapeLightingMaterial->MaterialProgram->Uniforms_NotTexture["PaintChTex3"]->DataPtr = glm::value_ptr(MaterialUniformData->PixelShaderPaintTexChannelIndex3);
	LandscapeLightingMaterial->MaterialProgram->Uniforms_NotTexture["PaintChTex4"]->DataPtr = glm::value_ptr(MaterialUniformData->PixelShaderPaintTexChannelIndex4);
	LandscapeLightingMaterial->MaterialProgram->Uniforms_NotTexture["PaintChTex5"]->DataPtr = glm::value_ptr(MaterialUniformData->PixelShaderPaintTexChannelIndex5);	
}

int Landscape::GetVertX()
{
	return NumComponent.x * NumSubsection * NumSubsectionQuad + 1;
}

int Landscape::GetVertY()
{
	return NumComponent.y * NumSubsection * NumSubsectionQuad + 1;
}

void Landscape::GetHeightTextureData(int MinX, int MaxX, int MinY, int MaxY, unsigned short * DataStore)
{
	int ComponentIndexMinX, ComponentIndexMaxX, ComponentIndexMinY, ComponentIndexMaxY;
	CalculateInclusiveComponentIndexOverlap(MinX, MaxX, MinY, MaxY, &ComponentIndexMinX, &ComponentIndexMaxX, &ComponentIndexMinY, &ComponentIndexMaxY);
	int NumComponentQuad = NumSubsection * NumSubsectionQuad;
	bool bMissHeightData = false;

	for (int ComponentY = ComponentIndexMinY; ComponentY <= ComponentIndexMaxY; ComponentY++)
	{
		for (int ComponentX = ComponentIndexMinX; ComponentX <= ComponentIndexMaxX; ComponentX++)
		{
			std::hash_map<int, std::shared_ptr<LandscapeComponent>>::iterator Component = LandScapeComponentMap.find(GetComponentMapHashKey(ComponentX, ComponentY));
			if (Component != LandScapeComponentMap.end())
			{
				int TexSizeU = Component->second->HeightTexture->GetWidth();
				int TexSizeV = Component->second->HeightTexture->GetHeight();
				int HeightTextureOffsetU = Component->second->HeightTextureScaleBias.z * TexSizeU;
				int HeightTextureOffsetV = Component->second->HeightTextureScaleBias.w * TexSizeV;

				int MinXComponentCoord = glm::clamp(MinX - ComponentX * NumComponentQuad, 0, NumComponentQuad);
				int MaxXComponentCoord = glm::clamp(MaxX - ComponentX * NumComponentQuad, 0, NumComponentQuad);
				int MinYComponentCoord = glm::clamp(MinY - ComponentY * NumComponentQuad, 0, NumComponentQuad);
				int MaxYComponentCoord = glm::clamp(MaxY - ComponentY * NumComponentQuad, 0, NumComponentQuad);

				int SubsectionRangeMinX = glm::clamp((MinXComponentCoord - 1) / NumSubsectionQuad, 0, NumSubsection - 1);
				int SubsectionRangeMaxX = glm::clamp(MaxXComponentCoord / NumSubsectionQuad, 0, NumSubsection - 1);
				int SubsectionRangeMinY = glm::clamp((MinYComponentCoord - 1) / NumSubsectionQuad, 0, NumSubsection - 1);
				int SubsectionRangeMaxY = glm::clamp(MaxYComponentCoord / NumSubsectionQuad, 0, NumSubsection - 1);

				for (int SubsectionIndexY = SubsectionRangeMinY; SubsectionIndexY <= SubsectionRangeMaxY; SubsectionIndexY++)
				{
					for (int SubsectionIndexX = SubsectionRangeMinX; SubsectionIndexX <= SubsectionRangeMaxX; SubsectionIndexX++)
					{
						int MinXSubsectionCoord = glm::clamp(MinXComponentCoord - SubsectionIndexX * NumSubsectionQuad, 0, NumSubsectionQuad);
						int MaxXSubsectionCoord = glm::clamp(MaxXComponentCoord - SubsectionIndexX * NumSubsectionQuad, 0, NumSubsectionQuad);
						int MinYSubsectionCoord = glm::clamp(MinYComponentCoord - SubsectionIndexY * NumSubsectionQuad, 0, NumSubsectionQuad);
						int MaxYSubsectionCoord = glm::clamp(MaxYComponentCoord - SubsectionIndexY * NumSubsectionQuad, 0, NumSubsectionQuad);

						for (int Y = MinYSubsectionCoord; Y <= MaxYSubsectionCoord; Y++)
						{
							for (int X = MinXSubsectionCoord; X <= MaxXSubsectionCoord; X++)
							{
								int XLandscapeCoord = SubsectionIndexX * NumSubsectionQuad + ComponentX * NumComponentQuad + X;
								int YLandscapeCoord = SubsectionIndexY * NumSubsectionQuad + ComponentY * NumComponentQuad + Y;

								int TexX = HeightTextureOffsetU + (NumSubsectionQuad + 1) * SubsectionIndexX + X;
								int TexY = HeightTextureOffsetV + (NumSubsectionQuad + 1) * SubsectionIndexY + Y;

								unsigned char * TexData = Component->second->HeightTexture->GetRawDataPtr(0)->MipData + (TexX + (TexY * TexSizeU)) * 4; //RGBA8
								unsigned char HeightTexDataR = TexData[0]; 
								unsigned char HeightTexDataG = TexData[1];
								unsigned short HeightData = ((unsigned short)HeightTexDataR) << 8 | HeightTexDataG;

								DataStore[(XLandscapeCoord - MinX) + (YLandscapeCoord - MinY) * (MaxX - MinX + 1)] = HeightData;
							}
						}
					}
				}
			}
			else
			{
				/*if (MinX < 0 || MinY < 0 || MaxX >= GetVertXY() || MaxY >= GetVertXY())
				{*/
					bMissHeightData = true;
				//}
			}
		}
	}

	int NumLandscapeQuadX = GetVertX() - 1;
	int NumLandscapeQuadY = GetVertY() - 1;
	if (bMissHeightData)
	{
		for (int Y = MinY; Y <= MaxY; Y++)
		{
			for (int X = MinX; X <= MaxX; X++)
			{
				if (X < 0)
				{
					if (Y < 0)
					{
						DataStore[(X - MinX) + (Y - MinY) * (MaxX - MinX + 1)] = DataStore[(X - MinX + 1) + (Y - MinY + 1) * (MaxX - MinX + 1)];
					}
					else if (Y > NumLandscapeQuadY)
					{
						DataStore[(X - MinX) + (Y - MinY) * (MaxX - MinX + 1)] = DataStore[(X - MinX + 1) + (Y - MinY - 1) * (MaxX - MinX + 1)];
					}
					else
					{
						DataStore[(X - MinX) + (Y - MinY) * (MaxX - MinX + 1)] = DataStore[(X - MinX + 1) + (Y - MinY) * (MaxX - MinX + 1)];
					}
				}
				else if (X > NumLandscapeQuadX)
				{
					if (Y < 0)
					{
						DataStore[(X - MinX) + (Y - MinY) * (MaxX - MinX + 1)] = DataStore[(X - MinX - 1) + (Y - MinY + 1) * (MaxX - MinX + 1)];
					}
					else if (Y > NumLandscapeQuadY)
					{
						DataStore[(X - MinX) + (Y - MinY) * (MaxX - MinX + 1)] = DataStore[(X - MinX - 1) + (Y - MinY - 1) * (MaxX - MinX + 1)];
					}
					else
					{
						DataStore[(X - MinX) + (Y - MinY) * (MaxX - MinX + 1)] = DataStore[(X - MinX - 1) + (Y - MinY) * (MaxX - MinX + 1)];
					}
				}
				else if (Y < 0)
				{
					if (X < 0)
					{
						DataStore[(X - MinX) + (Y - MinY) * (MaxX - MinX + 1)] = DataStore[(X - MinX + 1) + (Y - MinY + 1) * (MaxX - MinX + 1)];
					}
					else if (X > NumLandscapeQuadX)
					{
						DataStore[(X - MinX) + (Y - MinY) * (MaxX - MinX + 1)] = DataStore[(X - MinX - 1) + (Y - MinY + 1) * (MaxX - MinX + 1)];
					}
					else
					{
						DataStore[(X - MinX) + (Y - MinY) * (MaxX - MinX + 1)] = DataStore[(X - MinX) + (Y - MinY + 1) * (MaxX - MinX + 1)];
					}
				}
				else if (Y > NumLandscapeQuadY)
				{
					if (X < 0)
					{
						DataStore[(X - MinX) + (Y - MinY) * (MaxX - MinX + 1)] = DataStore[(X - MinX + 1) + (Y - MinY - 1) * (MaxX - MinX + 1)];
					}
					else if (X > NumLandscapeQuadX)
					{
						DataStore[(X - MinX) + (Y - MinY) * (MaxX - MinX + 1)] = DataStore[(X - MinX - 1) + (Y - MinY - 1) * (MaxX - MinX + 1)];
					}
					else
					{
						DataStore[(X - MinX) + (Y - MinY) * (MaxX - MinX + 1)] = DataStore[(X - MinX) + (Y - MinY - 1) * (MaxX - MinX + 1)];
					}
				}
			}
		}
	}
}

void Landscape::SetHeightTextureData(int MinX, int MaxX, int MinY, int MaxY, unsigned short * DataWrite)
{
	int NumVertX = MaxX - MinX + 1;
	int NumVertY = MaxY - MinY + 1;

	std::vector<glm::vec3> VertexNormals(NumVertX * NumVertY);
	for (int Y = 0; Y < NumVertY - 1; Y++)
	{
		for (int X = 0; X < NumVertX - 1; X++)
		{
			const glm::vec3 Vert00 = glm::vec3(X + 0, Y + 0, ((float)DataWrite[(X + 0) + (Y + 0) * NumVertX] - 32768.0f) * LANDSCAPE_ZSCALE) * ObjectTransform.GetScale();
			const glm::vec3 Vert10 = glm::vec3(X + 1, Y + 0, ((float)DataWrite[(X + 1) + (Y + 0) * NumVertX] - 32768.0f) * LANDSCAPE_ZSCALE) * ObjectTransform.GetScale();
			const glm::vec3 Vert01 = glm::vec3(X + 0, Y + 1, ((float)DataWrite[(X + 0) + (Y + 1) * NumVertX] - 32768.0f) * LANDSCAPE_ZSCALE) * ObjectTransform.GetScale();
			const glm::vec3 Vert11 = glm::vec3(X + 1, Y + 1, ((float)DataWrite[(X + 1) + (Y + 1) * NumVertX] - 32768.0f) * LANDSCAPE_ZSCALE) * ObjectTransform.GetScale();

			const glm::vec3 FaceNormal1 = glm::normalize(glm::cross(Vert00 - Vert10, Vert10 - Vert11));
			const glm::vec3 FaceNormal2 = glm::normalize(glm::cross(Vert11 - Vert01, Vert01 - Vert00));

			VertexNormals[(X + 1 + NumVertX * (Y + 0))] += FaceNormal1;
			VertexNormals[(X + 0 + NumVertX * (Y + 1))] += FaceNormal2;
			VertexNormals[(X + 0 + NumVertX * (Y + 0))] += FaceNormal1 + FaceNormal2;
			VertexNormals[(X + 1 + NumVertX * (Y + 1))] += FaceNormal1 + FaceNormal2;
		}
	}

	int ComponentIndexMinX, ComponentIndexMaxX, ComponentIndexMinY, ComponentIndexMaxY;
	CalculateInclusiveComponentIndexOverlap(MinX, MaxX, MinY, MaxY, &ComponentIndexMinX, &ComponentIndexMaxX, &ComponentIndexMinY, &ComponentIndexMaxY);
	int NumComponentQuad = NumSubsection * NumSubsectionQuad;
	for (int ComponentY = ComponentIndexMinY; ComponentY <= ComponentIndexMaxY; ComponentY++)
	{
		for (int ComponentX = ComponentIndexMinX; ComponentX <= ComponentIndexMaxX; ComponentX++)
		{
			std::hash_map<int, std::shared_ptr<LandscapeComponent>>::iterator ComponentIt = LandScapeComponentMap.find(GetComponentMapHashKey(ComponentX, ComponentY));

			if (ComponentIt != LandScapeComponentMap.end()){
				std::shared_ptr<LandscapeComponent> Component = ComponentIt->second;

				int TexSizeU = Component->HeightTexture->GetWidth();
				int TexSizeV = Component->HeightTexture->GetHeight();
				int HeightTextureOffsetU = Component->HeightTextureScaleBias.z * TexSizeU;
				int HeightTextureOffsetV = Component->HeightTextureScaleBias.w * TexSizeV;

				int MinXComponentCoord = glm::clamp(MinX - ComponentX * NumComponentQuad, 0, NumComponentQuad);
				int MaxXComponentCoord = glm::clamp(MaxX - ComponentX * NumComponentQuad, 0, NumComponentQuad);
				int MinYComponentCoord = glm::clamp(MinY - ComponentY * NumComponentQuad, 0, NumComponentQuad);
				int MaxYComponentCoord = glm::clamp(MaxY - ComponentY * NumComponentQuad, 0, NumComponentQuad);

				int SubsectionRangeMinX = glm::clamp((MinXComponentCoord - 1) / NumSubsectionQuad, 0, NumSubsection - 1);
				int SubsectionRangeMaxX = glm::clamp(MaxXComponentCoord / NumSubsectionQuad, 0, NumSubsection - 1);
				int SubsectionRangeMinY = glm::clamp((MinYComponentCoord - 1) / NumSubsectionQuad, 0, NumSubsection - 1);
				int SubsectionRangeMaxY = glm::clamp(MaxYComponentCoord / NumSubsectionQuad, 0, NumSubsection - 1);

				for (int SubsectionIndexY = SubsectionRangeMinY; SubsectionIndexY <= SubsectionRangeMaxY; SubsectionIndexY++)
				{
					for (int SubsectionIndexX = SubsectionRangeMinX; SubsectionIndexX <= SubsectionRangeMaxX; SubsectionIndexX++)
					{
						int MinXSubsectionCoord = glm::clamp(MinXComponentCoord - SubsectionIndexX * NumSubsectionQuad, 0, NumSubsectionQuad);
						int MaxXSubsectionCoord = glm::clamp(MaxXComponentCoord - SubsectionIndexX * NumSubsectionQuad, 0, NumSubsectionQuad);
						int MinYSubsectionCoord = glm::clamp(MinYComponentCoord - SubsectionIndexY * NumSubsectionQuad, 0, NumSubsectionQuad);
						int MaxYSubsectionCoord = glm::clamp(MaxYComponentCoord - SubsectionIndexY * NumSubsectionQuad, 0, NumSubsectionQuad);

						for (int Y = MinYSubsectionCoord; Y <= MaxYSubsectionCoord; Y++)
						{
							for (int X = MinXSubsectionCoord; X <= MaxXSubsectionCoord; X++)
							{
								int XLandscapeCoord = SubsectionIndexX * NumSubsectionQuad + ComponentX * NumComponentQuad + X;
								int YLandscapeCoord = SubsectionIndexY * NumSubsectionQuad + ComponentY * NumComponentQuad + Y;

								const unsigned short& Height = DataWrite[XLandscapeCoord - MinX + (YLandscapeCoord - MinY) * (MaxX - MinX + 1)];
								int TexX = HeightTextureOffsetU + (NumSubsectionQuad + 1) * SubsectionIndexX + X;
								int TexY = HeightTextureOffsetV + (NumSubsectionQuad + 1) * SubsectionIndexY + Y;

								unsigned char * TexData = Component->HeightTexture->GetRawDataPtr(0)->MipData + (TexX + (TexY * TexSizeU)) * 4;
								TexData[0] = Height >> 8;
								TexData[1] = Height & 255;

								if (XLandscapeCoord > MinX && XLandscapeCoord < MaxX && YLandscapeCoord > MinY && YLandscapeCoord < MaxY)
								{
									glm::vec3 Normal = glm::normalize(VertexNormals[XLandscapeCoord - MinX + (YLandscapeCoord - MinY) * (MaxX - MinX + 1)]);
									TexData[2] = glm::round(127.5f * (Normal.x + 1.0f));
									TexData[3] = glm::round(127.5f * (Normal.y + 1.0f));
								}
							}
						}
					}
				}
				Component->GenerateHeightTextureMipData(MinXComponentCoord, MaxXComponentCoord, MinYComponentCoord, MaxYComponentCoord);
				Component->UpdateSurroundBox();
			}
		}
	}
}

void Landscape::UpdateHeightTextureGPUObject(int MinX, int MaxX, int MinY, int MaxY)
{
	int ComponentIndexMinX, ComponentIndexMaxX, ComponentIndexMinY, ComponentIndexMaxY;
	CalculateInclusiveComponentIndexOverlap(MinX, MaxX, MinY, MaxY, &ComponentIndexMinX, &ComponentIndexMaxX, &ComponentIndexMinY, &ComponentIndexMaxY);

	for (int ComponentY = ComponentIndexMinY; ComponentY <= ComponentIndexMaxY; ComponentY++)
	{
		for (int ComponentX = ComponentIndexMinX; ComponentX <= ComponentIndexMaxX; ComponentX++)
		{
			std::hash_map<int, std::shared_ptr<LandscapeComponent>>::iterator it = LandScapeComponentMap.find(GetComponentMapHashKey(ComponentX, ComponentY));
			if (it != LandScapeComponentMap.end())
			{
				std::shared_ptr<LandscapeComponent> Component = it->second;
				Component->HeightTexture->UpdateGPUObjectData();
			}
		}
	}
}

void Landscape::GetWeightTextureData(std::shared_ptr<LandscapePaintLayerInfo> layerInfo, int MinX, int MaxX, int MinY, int MaxY, unsigned char* DataStore)
{
	int ComponentIndexMinX, ComponentIndexMaxX, ComponentIndexMinY, ComponentIndexMaxY;
	CalculateInclusiveComponentIndexNoOverlap(MinX, MaxX, MinY, MaxY, &ComponentIndexMinX, &ComponentIndexMaxX, &ComponentIndexMinY, &ComponentIndexMaxY);

	int NumComponentQuad = NumSubsection * NumSubsectionQuad;
	for (int ComponentY = ComponentIndexMinY; ComponentY <= ComponentIndexMaxY; ComponentY++)
	{
		for (int ComponentX = ComponentIndexMinX; ComponentX <= ComponentIndexMaxX; ComponentX++)
		{
			std::hash_map<int, std::shared_ptr<LandscapeComponent>>::iterator it = LandScapeComponentMap.find(GetComponentMapHashKey(ComponentX, ComponentY));
			if (it != LandScapeComponentMap.end())
			{
				std::shared_ptr<LandscapeComponent> Component = it->second;
				std::shared_ptr<Texture> WeightTexture = Component->WeightTextures[layerInfo->Index / 4];

				int MinXComponentCoord = glm::clamp(MinX - ComponentX * NumComponentQuad, 0, NumComponentQuad);
				int MaxXComponentCoord = glm::clamp(MaxX - ComponentX * NumComponentQuad, 0, NumComponentQuad);
				int MinYComponentCoord = glm::clamp(MinY - ComponentY * NumComponentQuad, 0, NumComponentQuad);
				int MaxYComponentCoord = glm::clamp(MaxY - ComponentY * NumComponentQuad, 0, NumComponentQuad);

				int SubsectionIndexMinX = glm::clamp((MinXComponentCoord - 1) / NumSubsectionQuad, 0, NumSubsection - 1);
				int SubsectionIndexMaxX = glm::clamp(MaxXComponentCoord / NumSubsectionQuad, 0, NumSubsection - 1);
				int SubsectionIndexMinY = glm::clamp((MinYComponentCoord - 1) / NumSubsectionQuad, 0, NumSubsection - 1);
				int SubsectionIndexMaxY = glm::clamp(MaxYComponentCoord / NumSubsectionQuad, 0, NumSubsection - 1);

				for (int SubsectionIndexY = SubsectionIndexMinY; SubsectionIndexY <= SubsectionIndexMaxY; SubsectionIndexY++)
				{
					for (int SubsectionIndexX = SubsectionIndexMinX; SubsectionIndexX <= SubsectionIndexMaxX; SubsectionIndexX++)
					{
						int MinXSubsectionCoord = glm::clamp(MinXComponentCoord - SubsectionIndexX * NumSubsectionQuad, 0, NumSubsectionQuad);
						int MaxXSubsectionCoord = glm::clamp(MaxXComponentCoord - SubsectionIndexX * NumSubsectionQuad, 0, NumSubsectionQuad);
						int MinYSubsectionCoord = glm::clamp(MinYComponentCoord - SubsectionIndexY * NumSubsectionQuad, 0, NumSubsectionQuad);
						int MaxYSubsectionCoord = glm::clamp(MaxYComponentCoord - SubsectionIndexY * NumSubsectionQuad, 0, NumSubsectionQuad);

						for (int Y = MinYSubsectionCoord; Y <= MaxYSubsectionCoord; Y++)
						{
							for (int X = MinXSubsectionCoord; X <= MaxXSubsectionCoord; X++)
							{
								int LandscapeX = SubsectionIndexX * NumSubsectionQuad + ComponentX * NumComponentQuad + X;
								int LandscapeY = SubsectionIndexY * NumSubsectionQuad + ComponentY * NumComponentQuad + Y;

								int TexSizeU = WeightTexture->GetWidth();
								int TexSizeV = WeightTexture->GetHeight();

								int WeightTextureOffsetX = Component->WeightTextureScaleBias.z * (float)TexSizeU;
								int WeightTextureOffsetY = Component->WeightTextureScaleBias.w * (float)TexSizeV;
								int TexX = WeightTextureOffsetX + (NumSubsectionQuad + 1) * SubsectionIndexX + X;
								int TexY = WeightTextureOffsetY + (NumSubsectionQuad + 1) * SubsectionIndexY + Y;

								unsigned char * WeightData = WeightTexture->GetRawDataPtr(0)->MipData + (TexX + TexY * TexSizeU) * 4;

								DataStore[(LandscapeX - MinX) + (LandscapeY - MinY) * (MaxX - MinX + 1)] = WeightData[layerInfo->ChannelInWeightTexutre];
							}
						}
					}
				}
			}
		}
	}
}

void Landscape::SetWeightTextureData(std::shared_ptr<LandscapePaintLayerInfo> layerInfo, int MinX, int MaxX, int MinY, int MaxY, unsigned char* DataWrite)
{
	int ComponentIndexMinX, ComponentIndexMaxX, ComponentIndexMinY, ComponentIndexMaxY;
	CalculateInclusiveComponentIndexOverlap(MinX, MaxX, MinY, MaxY, &ComponentIndexMinX, &ComponentIndexMaxX, &ComponentIndexMinY, &ComponentIndexMaxY);
	
	std::map<int, unsigned char *> LayerDataPtr;

	int NumComponentQuad = NumSubsection * NumSubsectionQuad;
	for (int ComponentY = ComponentIndexMinY; ComponentY <= ComponentIndexMaxY; ComponentY++)
	{
		for (int ComponentX = ComponentIndexMinX; ComponentX <= ComponentIndexMaxX; ComponentX++)
		{
			std::hash_map<int, std::shared_ptr<LandscapeComponent>>::iterator it = LandScapeComponentMap.find(GetComponentMapHashKey(ComponentX, ComponentY));
			if (it != LandScapeComponentMap.end())
			{
				std::shared_ptr<LandscapeComponent> Component = it->second;

				std::shared_ptr<Texture> WeightTexture = Component->WeightTextures[layerInfo->Index / 4];

				for (int LayerIndex = 0; LayerIndex < Layers.size(); LayerIndex++)
				{
					LayerDataPtr[LayerIndex] = Component->WeightTextures[Layers[LayerIndex]->Index / 4]->GetRawDataPtr(0)->MipData + Layers[LayerIndex]->ChannelInWeightTexutre;
				}

				int MinXComponentCoord = glm::clamp(MinX - ComponentX * NumComponentQuad, 0, NumComponentQuad);
				int MaxXComponentCoord = glm::clamp(MaxX - ComponentX * NumComponentQuad, 0, NumComponentQuad);
				int MinYComponentCoord = glm::clamp(MinY - ComponentY * NumComponentQuad, 0, NumComponentQuad);
				int MaxYComponentCoord = glm::clamp(MaxY - ComponentY * NumComponentQuad, 0, NumComponentQuad);

				int SubsectionIndexMinX = glm::clamp((MinXComponentCoord - 1) / NumSubsectionQuad, 0, NumSubsection - 1);
				int SubsectionIndexMaxX = glm::clamp(MaxXComponentCoord / NumSubsectionQuad, 0, NumSubsection - 1);
				int SubsectionIndexMinY = glm::clamp((MinYComponentCoord - 1) / NumSubsectionQuad, 0, NumSubsection - 1);
				int SubsectionIndexMaxY = glm::clamp(MaxYComponentCoord / NumSubsectionQuad, 0, NumSubsection - 1);

				for (int SubsectionIndexY = SubsectionIndexMinY; SubsectionIndexY <= SubsectionIndexMaxY; SubsectionIndexY++)
				{
					for (int SubsectionIndexX = SubsectionIndexMinX; SubsectionIndexX <= SubsectionIndexMaxX; SubsectionIndexX++)
					{
						int MinXSubsectionCoord = glm::clamp(MinXComponentCoord - SubsectionIndexX * NumSubsectionQuad, 0, NumSubsectionQuad);
						int MaxXSubsectionCoord = glm::clamp(MaxXComponentCoord - SubsectionIndexX * NumSubsectionQuad, 0, NumSubsectionQuad);
						int MinYSubsectionCoord = glm::clamp(MinYComponentCoord - SubsectionIndexY * NumSubsectionQuad, 0, NumSubsectionQuad);
						int MaxYSubsectionCoord = glm::clamp(MaxYComponentCoord - SubsectionIndexY * NumSubsectionQuad, 0, NumSubsectionQuad);

						for (int Y = MinYSubsectionCoord; Y <= MaxYSubsectionCoord; Y++)
						{
							for (int X = MinXSubsectionCoord; X <= MaxXSubsectionCoord; X++)
							{
								int LandscapeX = SubsectionIndexX * NumSubsectionQuad + ComponentX * NumComponentQuad + X;
								int LandscapeY = SubsectionIndexY * NumSubsectionQuad + ComponentY * NumComponentQuad + Y;

								int TexSizeU = WeightTexture->GetWidth();
								int TexSizeV = WeightTexture->GetHeight();

								int WeightTextureOffsetX = Component->WeightTextureScaleBias.z * (float)TexSizeU;
								int WeightTextureOffsetY = Component->WeightTextureScaleBias.w * (float)TexSizeV;
								int TexX = WeightTextureOffsetX + (NumSubsectionQuad + 1) * SubsectionIndexX + X;
								int TexY = WeightTextureOffsetY + (NumSubsectionQuad + 1) * SubsectionIndexY + Y;
								int TexDataIndex = (TexX + TexY * TexSizeU) * 4;

								unsigned char * WeightData = WeightTexture->GetRawDataPtr(0)->MipData + TexDataIndex;
								unsigned char CurrentWeight = WeightData[layerInfo->ChannelInWeightTexutre];
								unsigned char NewWeight = DataWrite[(LandscapeX - MinX) + (LandscapeY - MinY) * (MaxX - MinX + 1)];

								if (CurrentWeight == NewWeight)
								{
									continue;
								}

								int OtherLayersWeightSum = 0;
								if (layerInfo->isWeightNormalBlend)
								{
									for (std::map<int, unsigned char*>::iterator it = LayerDataPtr.begin(); it != LayerDataPtr.end(); it++)
									{
										if (Layers[it->first]->isWeightNormalBlend && Layers[it->first] != layerInfo)
										{
											OtherLayersWeightSum += it->second[TexDataIndex];
										}
									}

									if (OtherLayersWeightSum == 0 && NewWeight < 255)
									{
										if (NewWeight < CurrentWeight)
										{
											std::shared_ptr<LandscapePaintLayerInfo> ReplaceLayer = Layers[(layerInfo->Index + 1) >= Layers.size() ? 0 : (layerInfo->Index + 1)];
											LayerDataPtr[ReplaceLayer->Index][TexDataIndex] = 255 - NewWeight;
											LayerDataPtr[layerInfo->Index][TexDataIndex] = NewWeight;
										}
										else
										{
											LayerDataPtr[layerInfo->Index][TexDataIndex] = 255;
										}
									}
									else
									{
										for (std::map<int, unsigned char*>::iterator it = LayerDataPtr.begin(); it != LayerDataPtr.end(); it++)
										{
											if (Layers[it->first] == layerInfo)
											{
												it->second[TexDataIndex] = NewWeight;
											}
											else
											{
												if (Layers[it->first]->isWeightNormalBlend)
												{
													it->second[TexDataIndex] = glm::clamp((int)glm::round((float)(255 - NewWeight) * (float)it->second[TexDataIndex] / (float)OtherLayersWeightSum), 0, 255);
												}
											}
										}
									}
								}
								else
								{
									LayerDataPtr[layerInfo->Index][TexDataIndex] = NewWeight;
								}
							}
						}
					}
				}
				for (int WeightTextureIndex = 0; WeightTextureIndex < Component->WeightTextures.size(); WeightTextureIndex++)
				{
					Component->GenerateWeightTextureMipData(Component->WeightTextures[WeightTextureIndex], MinXComponentCoord, MaxXComponentCoord, MinYComponentCoord, MaxYComponentCoord);
				}
			}
		}
	}
}

void Landscape::UpdateWeightTextureGPUObject(int MinX, int MaxX, int MinY, int MaxY)
{
	int ComponentIndexMinX, ComponentIndexMaxX, ComponentIndexMinY, ComponentIndexMaxY;
	CalculateInclusiveComponentIndexOverlap(MinX, MaxX, MinY, MaxY, &ComponentIndexMinX, &ComponentIndexMaxX, &ComponentIndexMinY, &ComponentIndexMaxY);

	for (int ComponentY = ComponentIndexMinY; ComponentY <= ComponentIndexMaxY; ComponentY++)
	{
		for (int ComponentX = ComponentIndexMinX; ComponentX <= ComponentIndexMaxX; ComponentX++)
		{
			std::hash_map<int, std::shared_ptr<LandscapeComponent>>::iterator it = LandScapeComponentMap.find(GetComponentMapHashKey(ComponentX, ComponentY));
			if (it != LandScapeComponentMap.end())
			{
				std::shared_ptr<LandscapeComponent> Component = it->second;
				for (int WeightTextureIndex = 0; WeightTextureIndex < Component->WeightTextures.size(); WeightTextureIndex++)
				{
					Component->WeightTextures[WeightTextureIndex]->UpdateGPUObjectData();
				}
				
			}
		}
	}
}

void Landscape::CalculateInclusiveComponentIndexOverlap(int MinX, int MaxX, int MinY, int MaxY, int * ComponentIndexX1, int * ComponentIndexX2, int * ComponentIndexY1, int * ComponentIndexY2)
{
	int ComponentSizeQuads = NumSubsection * NumSubsectionQuad;
	*ComponentIndexX1 = (MinX - 1 >= 0) ? (MinX - 1) / ComponentSizeQuads : (MinX) / ComponentSizeQuads - 1;	// -1 because we need to pick up vertices shared between components
	*ComponentIndexY1 = (MinY - 1 >= 0) ? (MinY - 1) / ComponentSizeQuads : (MinY) / ComponentSizeQuads - 1;
	*ComponentIndexX2 = (MaxX >= 0) ? MaxX / ComponentSizeQuads : (MaxX + 1) / ComponentSizeQuads - 1;
	*ComponentIndexY2 = (MaxY >= 0) ? MaxY / ComponentSizeQuads : (MaxY + 1) / ComponentSizeQuads - 1;
}

void Landscape::CalculateInclusiveComponentIndexNoOverlap(int MinX, int MaxX, int MinY, int MaxY, int * ComponentIndexX1, int * ComponentIndexX2, int * ComponentIndexY1, int * ComponentIndexY2)
{
	int ComponentSizeQuads = NumSubsection * NumSubsectionQuad;
	// Find component range for this block of data
	*ComponentIndexX1 = (MinX >= 0) ? MinX / ComponentSizeQuads : (MinX + 1) / ComponentSizeQuads - 1;	// -1 because we need to pick up vertices shared between components
	*ComponentIndexY1 = (MinY >= 0) ? MinY / ComponentSizeQuads : (MinY + 1) / ComponentSizeQuads - 1;
	*ComponentIndexX2 = (MaxX - 1 >= 0) ? (MaxX - 1) / ComponentSizeQuads : (MaxX) / ComponentSizeQuads - 1;
	*ComponentIndexY2 = (MaxY - 1 >= 0) ? (MaxY - 1) / ComponentSizeQuads : (MaxY) / ComponentSizeQuads - 1;
	// Shrink indices for shared values
	if (*ComponentIndexX2 < *ComponentIndexX1)
	{
		*ComponentIndexX2 = *ComponentIndexX1;
	}
	if (*ComponentIndexY2 < *ComponentIndexY1)
	{
		*ComponentIndexY2 = *ComponentIndexY1;
	}
}

void Landscape::BindPaintLayers(std::vector<LandscapePaintLayerInfo>& layers)
{
	for (int LayerIndex = 0; LayerIndex < layers.size(); LayerIndex++)
	{
		if (layers[LayerIndex].GetPaintTexture()->GPUId == 0)
		{
			layers[LayerIndex].GetPaintTexture()->CreateGPUObject(TextureParameter::Linear_Mip_Nearest, TextureParameter::Linear, TextureParameter::Repeat, TextureParameter::Repeat);
		}
		layers[LayerIndex].Index = LayerIndex;
	}
	Layers.reserve(layers.size());
	Layers.resize(layers.size());
	for (int LayerIndex = 0; LayerIndex < layers.size(); LayerIndex++)
	{
		Layers[LayerIndex] = std::shared_ptr<LandscapePaintLayerInfo>(new LandscapePaintLayerInfo(layers[LayerIndex]));
		Layers[LayerIndex]->ChannelInWeightTexutre = LayerIndex % 4;
	}

	MaterialUniformData->PaintTexture0ID = layers[0].GetPaintTexture()->GPUId;
	MaterialUniformData->PaintTexture1ID = layers[1].GetPaintTexture()->GPUId;
	MaterialUniformData->PaintTexture2ID = layers[2].GetPaintTexture()->GPUId;
	MaterialUniformData->PaintTexture3ID = layers[3].GetPaintTexture()->GPUId;
	MaterialUniformData->PaintTexture4ID = layers[4].GetPaintTexture()->GPUId;
	MaterialUniformData->PaintTexture5ID = layers[5].GetPaintTexture()->GPUId;

	std::vector<std::vector<std::vector<unsigned char>>> WeightValues;
	
	for (int ComponentY = 0; ComponentY < NumComponent.y; ComponentY++)
	{
		for (int ComponentX = 0; ComponentX < NumComponent.x; ComponentX++)
		{
			std::shared_ptr<LandscapeComponent> Component = LandScapeComponentMap.find(GetComponentMapHashKey(ComponentX, ComponentY))->second;
			int NumWeightTexture = glm::ceil((float)Layers.size() / 4.0f);
			Component->WeightTextures.reserve(NumWeightTexture);
			Component->WeightTextures.resize(NumWeightTexture);
			for (int WeightTextureIndex = 0; WeightTextureIndex < NumWeightTexture; WeightTextureIndex++)
			{
				Component->WeightTextures[WeightTextureIndex] = std::shared_ptr<Texture>(new Texture());
				Component->WeightTextures[WeightTextureIndex]->InitMipRawTextureData((NumSubsectionQuad + 1) * NumSubsection, (NumSubsectionQuad + 1) * NumSubsection, TextureDataTypeFormat::RGBA8);
			}
			Component->WeightTextureScaleBias = glm::vec4(1.0f / (float)Component->WeightTextures[0]->GetWidth(), 1.0f / (float)Component->WeightTextures[0]->GetHeight(), 0.5f / (float)Component->WeightTextures[0]->GetWidth(), 0.5f / (float)Component->WeightTextures[0]->GetWidth());
			Component->WeightTextureSubsectionOffset = (float)(NumSubsectionQuad + 1) / (float)Component->WeightTextures[0]->GetWidth(); //remeber that WeightTexutre Widht = Height

			//Prepare WeightTexture Data
			//Set WeightTexture First layer data = 255
			for (int WeightTextureIndex = 0; WeightTextureIndex < 1; WeightTextureIndex++)
			{
				for (int PixelLoc = 0; PixelLoc < Component->WeightTextures[WeightTextureIndex]->GetWidth() * Component->WeightTextures[WeightTextureIndex]->GetHeight(); PixelLoc++)
				{
					*(Component->WeightTextures[WeightTextureIndex]->GetRawDataPtr(0)->MipData + PixelLoc * 4) = 255;
				}
			}

			for (int WeightTextureIndex = 0; WeightTextureIndex < Component->WeightTextures.size(); WeightTextureIndex++)
			{
				Component->GenerateWeightTextureMipData(Component->WeightTextures[WeightTextureIndex]);
				Component->WeightTextures[WeightTextureIndex]->CreateGPUObject(TextureParameter::Linear_Mip_Nearest, TextureParameter::Linear, TextureParameter::Clamp_To_Edge, TextureParameter::Clamp_To_Edge);
			}
		}
	}	
}

std::shared_ptr<LandscapePaintLayerInfo> Landscape::GetLayer(int index)
{
	if (index >= Layers.size())
	{
		return nullptr;
	}
	else
	{
		return Layers[index];
	}	
}

std::vector<std::shared_ptr<LandscapePaintLayerInfo>>& Landscape::GetLayers()
{
	return Layers;
}

int Landscape::GetComponentMapHashKey(int ComponentX, int ComponentY)
{
	return ComponentX << 16 | ComponentY;
}

int Landscape::GetNumComponentX()
{
	return glm::floor(NumComponent.x);
}

int Landscape::GetNumComponentY()
{
	return glm::floor(NumComponent.y);
}

int Landscape::GetNumSubsection()
{
	return NumSubsection;
}

int Landscape::GetNumSubsectionQuad()
{
	return NumSubsectionQuad;
}

std::shared_ptr<Texture> Landscape::CreateAbstractHeightTex()
{
	int TexWidth = NumComponent.x * NumSubsection * NumSubsectionQuad + 1;
	int TexHeight = NumComponent.y * NumSubsection * NumSubsectionQuad + 1;
	if (!AbstractHeightTex)
	{
		AbstractHeightTex = std::shared_ptr<Texture>(new Texture());
		AbstractHeightTex->InitNoMipRawTextureData(TexWidth, TexHeight, TextureDataTypeFormat::RG8);
	}
	unsigned short * HeightTexData = new unsigned short[TexWidth * TexHeight];
	GetHeightTextureData(0, TexWidth - 1, 0, TexHeight - 1, HeightTexData);
	memcpy(AbstractHeightTex->GetRawDataPtr(0)->MipData, HeightTexData, TexWidth * TexHeight * 2);
	delete[]HeightTexData;
	
	return AbstractHeightTex;
}