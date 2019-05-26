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
//	Vector3f CameraPosWorld = _Scene->GetCamera(CameraIndex::MainCamera)->GetPosition();
//	Vector3f CenterWorld = ObjectTransform.GetPosition();
//	LODValue = glm::distance(Vector3f(CenterWorld), CameraPosWorld) / LODFactor - LODThreshold;
//	LODValue = glm::min(LODValue, static_cast<Float32>(MaxLOD));
//	CurrentLOD = LODValue < 0 ? 0 : static_cast<Int32>(glm::min(static_cast<Float32>(MaxLOD), LODValue));
//	LODValue = glm::max(LODValue, 0.0f);
//}