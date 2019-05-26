#pragma once

#include "Camera.h"
#include "Light.h"
#include <vector>
#include <algorithm>
#include <map>
#include <iostream>
#include <fstream>
using namespace std;

class Object;
class Model;

namespace ModeState{
	enum EditMode
	{
		CameraRotation,
		CameraTranslation,
		LandscapeSculpt,
		LandscapeFlatten,
		LandscapePaint,
		NumMode
	};

	enum ViewMode
	{
		LightingMode,
		WireframeMode
	};
}

enum CameraIndex
{
	MainCamera = 0,
	ShadowDepthCamera = 1,
	ReflectionCaptureCamera = 100
};

enum ObjectType
{
	Default =			0x00000000,
	StaticMesh =		0x00000001,
	DynamicMesh =		0x00000002,
	LandscapeMesh =		0x00000004,
	AbstractActor =		0x00000008,
	NavigationSystem =	0x000000F0
};

class SceneManager
{
public:
	SceneManager();
	~SceneManager();
	
	void Start();
	void InternalUpdate();
	void Update();
	void Render(std::shared_ptr<Camera> camera, UInt32 typeFlags = Default);
	void AddObj(ObjectType type, std::shared_ptr<Object> obj);
	void RemoveObj(std::shared_ptr<Object> obj);
	void AddCamera(UInt32 index, std::shared_ptr<Camera> camera);
	void AddLight(std::shared_ptr<Light> light);

	void PrepareShadowDepthMaterial();
	void PrepareLightingMaterial();

	std::shared_ptr<Camera> GetCamera(UInt32 index);
	std::vector<std::shared_ptr<Light>> GetAllLights();
	std::vector<std::shared_ptr<Object>> GetObjects(UInt32 typeFlag);

	ModeState::EditMode GetCurrentEditMode();
	ModeState::ViewMode GetCurrentViewMode();
	void SetCurrentEditMode(ModeState::EditMode editMode);
	void SetCurrentViewMode(ModeState::ViewMode viewMode);

	void UpdatePreFrameData();

private:
	std::vector<std::shared_ptr<Light>> SceneLights;
	std::multimap<UInt32, std::shared_ptr<Camera>> SceneCameras;
	std::multimap<UInt32, std::shared_ptr<Object>> SceneObjects;

	ModeState::EditMode CurrentEditMode;
	ModeState::ViewMode CurrentViewMode;
};
