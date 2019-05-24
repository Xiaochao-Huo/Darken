#include "SceneManager.h"
#include "Model.h"

SceneManager::SceneManager()
{
	CurrentEditMode = ModeState::EditMode::CameraRotation;
	CurrentViewMode = ModeState::ViewMode::LightingMode;
}

SceneManager::~SceneManager()
{
	
}

void SceneManager::Start()
{
	for (std::multimap<unsigned int, std::shared_ptr<Object>>::iterator ObjectIterator = SceneObjects.begin(); ObjectIterator != SceneObjects.end(); ObjectIterator++)
	{
		ObjectIterator->second->Start();
	}
}

void SceneManager::InternalUpdate()
{
	for (std::multimap<unsigned int, std::shared_ptr<Object>>::iterator ObjectIterator = SceneObjects.begin(); ObjectIterator != SceneObjects.end(); ObjectIterator++)
	{
		ObjectIterator->second->InternalUpdate();;
	}
}
void SceneManager::Update()
{
	for (std::multimap<unsigned int, std::shared_ptr<Object>>::iterator ObjectIterator = SceneObjects.begin(); ObjectIterator != SceneObjects.end(); ObjectIterator++)
	{
		ObjectIterator->second->Update();
	}
}

void SceneManager::Render(std::shared_ptr<Camera> camera, unsigned int typeFlags)
{
	for (std::multimap<unsigned int, std::shared_ptr<Object>>::iterator ObjectIterator = SceneObjects.begin(); ObjectIterator != SceneObjects.end(); ObjectIterator++)
	{
		if(typeFlags != Default && !(ObjectIterator->first & typeFlags))
		{
			continue;
		}
		if (ObjectIterator->second->bNeedCheckClip)
		{
			ObjectIterator->second->CheckWhetherNeedClip(camera);
		}
		if (ObjectIterator->second->bNeedClip)
		{
			continue;
		}
		ObjectIterator->second->Draw();
	}
}

void SceneManager::AddObj(ObjectType type, std::shared_ptr<Object> obj)
{
	SceneObjects.insert(std::pair<ObjectType, std::shared_ptr<Object>>(type, obj));
}

void SceneManager::RemoveObj(std::shared_ptr<Object> obj)
{
	for (std::multimap<unsigned int, std::shared_ptr<Object>>::iterator it = SceneObjects.begin(); it != SceneObjects.end(); it++)
	{
		if (it->second == obj)
		{
			SceneObjects.erase(it);
			break;
		}
	}
}

void SceneManager::AddCamera(unsigned int index, std::shared_ptr<Camera> camera)
{
	SceneCameras.insert(std::pair<unsigned int, std::shared_ptr<Camera>>(index, camera));
}

void SceneManager::AddLight(std::shared_ptr<Light> light)
{
	SceneLights.push_back(light);
}

std::shared_ptr<Camera> SceneManager::GetCamera(unsigned int index)
{
	return SceneCameras.find(index)->second;
}

std::vector<std::shared_ptr<Light>> SceneManager::GetAllLights()
{
	std::vector<std::shared_ptr<Light>> Lights;
	for (std::vector<std::shared_ptr<Light>>::iterator it = SceneLights.begin(); it != SceneLights.end(); it++)
	{
		Lights.push_back(*it);
	}
	return Lights;
}

std::vector<std::shared_ptr<Object>> SceneManager::GetObjects(unsigned int typeFlag)
{
	std::vector<std::shared_ptr<Object>> Objs;
	for (std::multimap<unsigned int, std::shared_ptr<Object>>::iterator it = SceneObjects.begin(); it != SceneObjects.end(); it++)
	{
		if (it->first & typeFlag)
		{
			Objs.push_back(it->second);
		}
	}

	return Objs;
}

void SceneManager::SetCurrentEditMode(ModeState::EditMode editMode)
{
	CurrentEditMode = editMode;
}

void SceneManager::SetCurrentViewMode(ModeState::ViewMode viewMode)
{
	CurrentViewMode = viewMode;
}

ModeState::EditMode SceneManager::GetCurrentEditMode()
{
	return CurrentEditMode;
}

ModeState::ViewMode SceneManager::GetCurrentViewMode()
{
	return CurrentViewMode;
}

void SceneManager::UpdatePreFrameData()
{
	for (std::multimap<unsigned int, std::shared_ptr<Object>>::iterator ObjectIterator = SceneObjects.begin(); ObjectIterator != SceneObjects.end(); ObjectIterator++)
	{
		if (ObjectIterator->first != ObjectType::StaticMesh && ObjectIterator->first != ObjectType::DynamicMesh) continue;
		Model* M = dynamic_cast<Model*>(ObjectIterator->second.get());
		M->UpdatePreFrameModelMatrix();
	}
	_Scene->GetCamera(CameraIndex::MainCamera)->SetViewMatrix_PreFrame(_Scene->GetCamera(CameraIndex::MainCamera)->GetViewMatrix());
	_Scene->GetCamera(CameraIndex::MainCamera)->SetProjectMatrix_PreFrame(_Scene->GetCamera(CameraIndex::MainCamera)->GetProjectMatrix());
}

void SceneManager::PrepareShadowDepthMaterial()
{
	for(std::multimap<unsigned int, std::shared_ptr<Object>>::iterator ObjectIterator = SceneObjects.begin(); ObjectIterator != SceneObjects.end(); ObjectIterator++)
	{
		if (ObjectIterator->first != ObjectType::StaticMesh && ObjectIterator->first != ObjectType::DynamicMesh) continue;
		Model* M = dynamic_cast<Model*>(ObjectIterator->second.get());
		M->BindShadowDepthMaterial();
	}
}

void SceneManager::PrepareLightingMaterial()
{
	for (std::multimap<unsigned int, std::shared_ptr<Object>>::iterator ObjectIterator = SceneObjects.begin(); ObjectIterator != SceneObjects.end(); ObjectIterator++)
	{
		if (ObjectIterator->first != ObjectType::StaticMesh && ObjectIterator->first != ObjectType::DynamicMesh) continue;
		Model* M = dynamic_cast<Model*>(ObjectIterator->second.get());
		M->BindLightingMaterial();
	}
}
