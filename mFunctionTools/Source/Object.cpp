#include "Object.h"

Object::Object() : bNeedCheckClip(true)
, bNeedClip(false)
{
}

Object::~Object()
{
}

//void Object::Draw()
//{
//
//}

//void Object::CalculateLOD()
//{
//	glm::vec3 CameraPosWorld = _Scene->GetCamera(CameraIndex::MainCamera)->GetPosition();
//	glm::vec3 CenterWorld = ObjectTransform.GetPosition();
//	LODValue = glm::distance(glm::vec3(CenterWorld), CameraPosWorld) / LODFactor - LODThreshold;
//	LODValue = glm::min(LODValue, static_cast<float>(MaxLOD));
//	CurrentLOD = LODValue < 0 ? 0 : static_cast<int>(glm::min(static_cast<float>(MaxLOD), LODValue));
//	LODValue = glm::max(LODValue, 0.0f);
//}