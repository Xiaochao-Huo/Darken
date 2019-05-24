#pragma once

#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsGles.h"
#include "mLandscape.h"
#include "mMaterial.h"
#include "mObject.h"

class LandscapeBrushData
{
public:
	LandscapeBrushData(glm::vec2& interactorPositionLocal, float brushSize, float fallOff, int landscapeNumVertsAbstractX, int landscapeNumVertsAbstractY);
	~LandscapeBrushData();
	void GetInclusiveBounds(int * minX, int * maxX, int * minY, int * maxY);
	float * GetDataPtr();
private:
	int MinX;
	int MaxX;
	int MinY;
	int MaxY;
	float * BrushAlpha;
};

class LandscapeBrushBase
{
public:
	LandscapeBrushBase();
	~LandscapeBrushBase();

	void SetHitPositionWorld(glm::vec3& hitPositionWorld);
	void SetBrushSize(float size);
	void SetInvertPressed(bool flag);
	float GetBrushSize();
	glm::vec2 GetBrushParameters_Sizes();

	virtual void Apply() = 0;

protected:
	std::shared_ptr<Landscape> ServeLandscape;

	float BrushSize;
	float FallOff;
	float BrushStrength;

	bool isInvertPressed;
	glm::vec3 HitPositionWorld;
	glm::vec2 BrushParameters_Sizes;
};

class LandscapeSculptBrush : public LandscapeBrushBase
{
public:
	LandscapeSculptBrush();
	LandscapeSculptBrush(std::shared_ptr<Landscape> serveLandscape);
	~LandscapeSculptBrush();

	virtual void Apply() override;
private:

};

class LandscapeFlattenBrush : public LandscapeBrushBase
{
public:
	LandscapeFlattenBrush();
	LandscapeFlattenBrush(std::shared_ptr<Landscape> serveLandscape);
	~LandscapeFlattenBrush();
	void SetTargetHeight(unsigned short targetHeight);

	virtual void Apply() override;
private:
	unsigned short TargetHeight;
};

class LandscapePaintBrush : public LandscapeBrushBase
{
public:
	LandscapePaintBrush();
	LandscapePaintBrush(std::shared_ptr<Landscape> serveLandscape);
	~LandscapePaintBrush();

	void SetSelectedLayer(int index);
	int GetCurrentLayerIndex();

	virtual void Apply() override;
private:
	std::shared_ptr<LandscapePaintLayerInfo> CurrentLayer;
};

