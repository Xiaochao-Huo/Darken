#pragma once
#include "Object.h"
#include "Camera.h"
#include "DeferRenderPipeline.h"

class SphereReflectionCapture : public Object
{
public:
	SphereReflectionCapture(const glm::vec3 &position, const float &radius, const float& brightness);
	~SphereReflectionCapture();

	virtual void Start() {};
	virtual void InternalUpdate() {};
	virtual void Update() {};
	virtual void FixUpdate() {};
	virtual void Draw() {};
	virtual void CheckWhetherNeedClip(std::shared_ptr<Camera> camera) {};

	void CaptureWithPipeLine(std::shared_ptr<DeferRenderPipeline> Pipeline);
	void CalReflectionCubeTexAvgBrightness();

	void CreateCaptureResources();
	void Create6FacesCameraList();
	unsigned int GetReflectionTextureID();
	float GetInfluenceRaidus();
	float GetBrightness();
	float GetAverageBrightness();

private:
	unsigned int CaptureTexCube;
	unsigned int CaptureTex2D;
	unsigned int CaptureTexSize;
	std::shared_ptr<Camera> CaptureCamera;

	float InfluenceRadius;
	float Brightness;
	float AverageBrightness;
};
