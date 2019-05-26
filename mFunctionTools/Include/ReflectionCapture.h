#pragma once
#include "Object.h"
#include "Camera.h"
#include "DeferRenderPipeline.h"

class SphereReflectionCapture : public Object
{
public:
	SphereReflectionCapture(const Vector3f &position, const Float32 &radius, const Float32& brightness);
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
	UInt32 GetReflectionTextureID();
	Float32 GetInfluenceRaidus();
	Float32 GetBrightness();
	Float32 GetAverageBrightness();

private:
	UInt32 CaptureTexCube;
	UInt32 CaptureTex2D;
	UInt32 CaptureTexSize;
	std::shared_ptr<Camera> CaptureCamera;

	Float32 InfluenceRadius;
	Float32 Brightness;
	Float32 AverageBrightness;
};
