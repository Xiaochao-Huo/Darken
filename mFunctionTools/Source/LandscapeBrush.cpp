#include "mLandscapeBrush.h"

LandscapeSculptBrush::LandscapeSculptBrush()
{
}

LandscapeSculptBrush::LandscapeSculptBrush(std::shared_ptr<Landscape> serveLandscape)
{
	ServeLandscape = serveLandscape;
}

LandscapeSculptBrush::~LandscapeSculptBrush()
{
}

void LandscapeSculptBrush::Apply()
{
	LandscapeBrushData BrushData(glm::vec2(glm::inverse(ServeLandscape->ObjectTransform.GetModelMatrix()) * glm::vec4(HitPositionWorld, 1.0)), BrushSize / ServeLandscape->ObjectTransform.GetScale().x, FallOff, ServeLandscape->GetVertX(), ServeLandscape->GetVertY());

	int BrushMinX, BrushMaxX, BrushMinY, BrushMaxY;
	BrushData.GetInclusiveBounds(&BrushMinX, &BrushMaxX, &BrushMinY, &BrushMaxY);

	int MinX = BrushMinX - 1;
	int MaxX = BrushMaxX + 1;
	int MinY = BrushMinY - 1;
	int MaxY = BrushMaxY + 1;

	unsigned short * HeightData = new unsigned short[(MaxX - MinX + 1) * (MaxY - MinY + 1)];
	memset(HeightData, 0, (MaxX - MinX + 1) * (MaxY - MinY + 1) * sizeof(unsigned short));
	ServeLandscape->GetHeightTextureData(MinX, MaxX, MinY, MaxY, HeightData);

	for (int Y = BrushMinY; Y <= BrushMaxY; Y++)
	{
		float * BrushDataScanline = &BrushData.GetDataPtr()[(Y - BrushMinY) * (BrushMaxX - BrushMinX + 1)];
		unsigned short* HeightDataScanline = &HeightData[(Y - MinY) * (MaxX - MinX + 1)];
		for (int X = BrushMinX; X <= BrushMaxX; X++)
		{
			const float BrushData = BrushDataScanline[X - BrushMinX];
			float SculptAmount = BrushData * BrushStrength;
			unsigned short CurrentValue = HeightDataScanline[X - MinX];

			if (isInvertPressed)
			{
				HeightDataScanline[X - MinX] = glm::clamp(CurrentValue - (int)glm::round(SculptAmount * 10.0), 0, 65535);
			}
			else
			{
				HeightDataScanline[X - MinX] = glm::clamp(CurrentValue + (int)glm::round(SculptAmount * 10.0), 0, 65535);
			}
			
		}
	}

	ServeLandscape->SetHeightTextureData(MinX, MaxX, MinY, MaxY, HeightData);
	ServeLandscape->UpdateHeightTextureGPUObject(MinX, MaxX, MinY, MaxY);

	delete[] HeightData;
}

LandscapeFlattenBrush::LandscapeFlattenBrush()
{
}

LandscapeFlattenBrush::LandscapeFlattenBrush(std::shared_ptr<Landscape> serveLandscape)
{
	ServeLandscape = serveLandscape;
}

LandscapeFlattenBrush::~LandscapeFlattenBrush()
{
}

void LandscapeFlattenBrush::SetTargetHeight(unsigned short targetHeight)
{
	TargetHeight = targetHeight;
}

void LandscapeFlattenBrush::Apply()
{
	LandscapeBrushData BrushData(glm::vec2(glm::inverse(ServeLandscape->ObjectTransform.GetModelMatrix()) * glm::vec4(HitPositionWorld, 1.0)), BrushSize / ServeLandscape->ObjectTransform.GetScale().x, FallOff, ServeLandscape->GetVertX(), ServeLandscape->GetVertY());

	int BrushMinX, BrushMaxX, BrushMinY, BrushMaxY;
	BrushData.GetInclusiveBounds(&BrushMinX, &BrushMaxX, &BrushMinY, &BrushMaxY);

	int MinX = BrushMinX - 1;
	int MaxX = BrushMaxX + 1;
	int MinY = BrushMinY - 1;
	int MaxY = BrushMaxY + 1;

	unsigned short * HeightData = new unsigned short[(MaxX - MinX + 1) * (MaxY - MinY + 1)];
	memset(HeightData, 0, (MaxX - MinX + 1) * (MaxY - MinY + 1) * sizeof(unsigned short));
	ServeLandscape->GetHeightTextureData(MinX, MaxX, MinY, MaxY, HeightData);

	for (int Y = BrushMinY; Y <= BrushMaxY; Y++)
	{
		float * BrushDataScanline = &BrushData.GetDataPtr()[(Y - BrushMinY) * (BrushMaxX - BrushMinX + 1)];
		unsigned short* HeightDataScanline = &HeightData[(Y - MinY) * (MaxX - MinX + 1)];
		for (int X = BrushMinX; X <= BrushMaxX; X++)
		{
			const float BrushData = BrushDataScanline[X - BrushMinX];
			float SculptAmount = BrushData * BrushStrength;
			unsigned short CurrentValue = HeightDataScanline[X - MinX];

			HeightDataScanline[X - MinX] = glm::ceil(Math::Lerp((float)CurrentValue, (float)TargetHeight, SculptAmount));
		}
	}

	ServeLandscape->SetHeightTextureData(MinX, MaxX, MinY, MaxY, HeightData);
	ServeLandscape->UpdateHeightTextureGPUObject(MinX, MaxX, MinY, MaxY);

	delete[] HeightData;
}



LandscapePaintBrush::LandscapePaintBrush()
{
}

LandscapePaintBrush::LandscapePaintBrush(std::shared_ptr<Landscape> serverLandscape)
{
	ServeLandscape = serverLandscape;
}

LandscapePaintBrush::~LandscapePaintBrush()
{
}

void LandscapePaintBrush::Apply()
{
	if (CurrentLayer == nullptr)
	{
		CurrentLayer = ServeLandscape->GetLayer(0);
	}

	LandscapeBrushData BrushData(glm::vec2(glm::inverse(ServeLandscape->ObjectTransform.GetModelMatrix()) * glm::vec4(HitPositionWorld, 1.0)), BrushSize / ServeLandscape->ObjectTransform.GetScale().x, FallOff, ServeLandscape->GetVertX(), ServeLandscape->GetVertY());

	int BrushMinX, BrushMaxX, BrushMinY, BrushMaxY;
	BrushData.GetInclusiveBounds(&BrushMinX, &BrushMaxX, &BrushMinY, &BrushMaxY);

	int MinX = BrushMinX;
	int MaxX = BrushMaxX;
	int MinY = BrushMinY;
	int MaxY = BrushMaxY;

	unsigned char * WeightData = new unsigned char[(MaxX - MinX + 1) * (MaxY - MinY + 1)];
	memset(WeightData, 0, (MaxX - MinX + 1) * (MaxY - MinY + 1));
	ServeLandscape->GetWeightTextureData(CurrentLayer, MinX, MaxX, MinY, MaxY, WeightData);

	for (int Y = MinY; Y <= MaxY; Y++)
	{
		float * BrushDataScanline = &BrushData.GetDataPtr()[(Y - BrushMinY) * (BrushMaxX - BrushMinX + 1)];
		unsigned char * WeightDataScanline = &WeightData[(Y - MinY) * (MaxX - MinX + 1)];
		for (int X = MinX; X <= MaxX; X++)
		{
			const float BrushData = BrushDataScanline[X - BrushMinX];
			float PaintAmount = BrushData * BrushStrength * 127.5f;
			const unsigned char CurrentWeightData = WeightDataScanline[X - MinX];
			
			if (isInvertPressed)
			{
				WeightDataScanline[X - MinX] = glm::clamp(CurrentWeightData - (int)glm::round(PaintAmount), 0, 255);
			}
			else
			{
				WeightDataScanline[X - MinX] = glm::clamp(CurrentWeightData + (int)glm::round(PaintAmount), 0, 255);
			}
		}
	}
	ServeLandscape->SetWeightTextureData(CurrentLayer, MinX, MaxX, MinY, MaxY, WeightData);
	ServeLandscape->UpdateWeightTextureGPUObject(MinX, MaxX, MinY, MaxY);

	delete[] WeightData;
}

void LandscapePaintBrush::SetSelectedLayer(int index)
{
	int preIndex = CurrentLayer->Index;
	CurrentLayer = ServeLandscape->GetLayer(index);
	if (CurrentLayer == nullptr)
	{
		CurrentLayer = ServeLandscape->GetLayer(preIndex);
	}
}

int LandscapePaintBrush::GetCurrentLayerIndex()
{
	if (CurrentLayer == nullptr)
	{
		CurrentLayer = ServeLandscape->GetLayer(0);
	}
	return CurrentLayer->Index;
}

LandscapeBrushData::LandscapeBrushData(glm::vec2& interactorPositionLocal, float brushSize, float fallOff, int landscapeNumVertsAbstractX, int landscapeNumVertsAbstractY)
{
	MinX = glm::floor(interactorPositionLocal.x - brushSize);
	MaxX = glm::ceil(interactorPositionLocal.x + brushSize);
	MinY = glm::floor(interactorPositionLocal.y - brushSize);
	MaxY = glm::ceil(interactorPositionLocal.y + brushSize);
	
	MinX = glm::max(MinX, 0);
	MaxX = glm::min(MaxX, landscapeNumVertsAbstractX);
	MinY = glm::max(MinY, 0);
	MaxY = glm::min(MaxY, landscapeNumVertsAbstractY);

	BrushAlpha = new float[(MaxX - MinX) * (MaxY - MinY)];
	memset(BrushAlpha, 0, (MaxX - MinX) * (MaxY - MinY) * sizeof(float));

	float TotalRadius = brushSize;
	float Radius = (1.0f - fallOff) * TotalRadius;
	float FalloffRadius = fallOff * TotalRadius;

	for (int Y = MinY; Y < MaxY; Y++)
	{
		float * Scanline = &BrushAlpha[(Y - MinY) * (MaxX - MinX)];
		for (int X = MinX; X < MaxX; X++)
		{
			float PreAmount = Scanline[X - MinX];
			if (PreAmount < 1.0f)
			{
				float MouseDist = glm::sqrt((interactorPositionLocal.x - X) * (interactorPositionLocal.x - X) + (interactorPositionLocal.y - Y) * (interactorPositionLocal.y - Y));

				float y = MouseDist < Radius ? 1.0f :
					FalloffRadius > 0.0f ? glm::max(0.0f, 1.0f - (MouseDist - Radius) / FalloffRadius) : 0.0f;

				float PaintAmount = y*y*(3.0f - 2.0f * y);

				if (PaintAmount > 0.0f && PaintAmount > PreAmount)
				{
					Scanline[X - MinX] = PaintAmount;
				}
			}
		}	
	}
}

LandscapeBrushData::~LandscapeBrushData()
{
	if (BrushAlpha)
	{
		delete[] BrushAlpha;
		BrushAlpha = nullptr;
	}
}

void LandscapeBrushData::GetInclusiveBounds(int * minX, int * maxX, int * minY, int * maxY)
{
	*minX = MinX;
	*maxX = MaxX - 1;
	*minY = MinY;
	*maxY = MaxY - 1;
}

float* LandscapeBrushData::GetDataPtr()
{
	return BrushAlpha;
}

LandscapeBrushBase::LandscapeBrushBase()
{
	BrushSize = 5.0f;
	FallOff = 0.4f;
	BrushStrength = 1.0f;

	isInvertPressed = false;
	HitPositionWorld = glm::vec3(0.0f, 0.0f, 0.0f);
	BrushParameters_Sizes = glm::vec2(BrushSize - BrushSize * FallOff, BrushSize * FallOff);
}

LandscapeBrushBase::~LandscapeBrushBase()
{
}

void LandscapeBrushBase::SetHitPositionWorld(glm::vec3& hitPositionWorld)
{
	HitPositionWorld = hitPositionWorld;
}

void LandscapeBrushBase::SetInvertPressed(bool flag)
{
	isInvertPressed = flag;
}

glm::vec2 LandscapeBrushBase::GetBrushParameters_Sizes()
{
	return BrushParameters_Sizes;
}

float LandscapeBrushBase::GetBrushSize()
{
	return BrushSize;
}

void LandscapeBrushBase::SetBrushSize(float size)
{
	BrushSize = glm::max(size, 0.0f);
	BrushParameters_Sizes = glm::vec2(BrushSize - BrushSize * FallOff, BrushSize * FallOff);
}