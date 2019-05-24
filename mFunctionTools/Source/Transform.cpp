#include "Transform.h"
#include "Quaternion.h"

Transform::Transform(glm::vec3 position, glm::vec3 eulerAngle, glm::vec3 scale)
{
	Position = glm::vec3(position);
	TranslationMatrix = glm::mat4(glm::translate(glm::mat4(1.0), position));

	EulerAngle = glm::vec3(eulerAngle);
	RotationMatrix = glm::rotate(glm::mat4(1.0), glm::radians(eulerAngle.z), ORIGIN_UPWARD) * glm::rotate(glm::mat4(1.0), glm::radians(eulerAngle.x), ORIGIN_FORWARD) * glm::rotate(glm::mat4(1.0), glm::radians(eulerAngle.y), ORIGIN_LEFTWARD);
	
	Scale = glm::vec3(scale);
	ScaleMatrix = glm::scale(glm::mat4(1.0), scale);

	Forward = glm::mat3(RotationMatrix) * ORIGIN_FORWARD;
	
	Upward = glm::mat3(RotationMatrix) * ORIGIN_UPWARD;
	
	Leftward = glm::mat3(RotationMatrix) * ORIGIN_LEFTWARD;

	CalculateModelMatrix();
	ModelMatrix_PreFrame = ModelMatrix;
}

Transform::Transform()
{
	Position = glm::vec3(0.0, 0.0, 0.0);
	TranslationMatrix = glm::mat4(1.0);

	EulerAngle = glm::vec3(0.0, 0.0, 0.0);
	RotationMatrix = glm::mat4(1.0);

	Scale = glm::vec3(1.0, 1.0, 1.0);
	ScaleMatrix = glm::mat4(1.0);
	
	Forward = ORIGIN_FORWARD;

	Upward = ORIGIN_UPWARD;

	Leftward = ORIGIN_LEFTWARD;

	CalculateModelMatrix();
	ModelMatrix_PreFrame = ModelMatrix;
}

void Transform::SetTransform(glm::vec3 position, glm::vec3 eulerAngle, glm::vec3 scale)
{
	Position = glm::vec3(position);
	TranslationMatrix = glm::mat4(glm::translate(glm::mat4(1.0), position));

	EulerAngle = glm::vec3(eulerAngle);
	RotationMatrix = glm::rotate(glm::mat4(1.0), glm::radians(eulerAngle.z), ORIGIN_UPWARD) * glm::rotate(glm::mat4(1.0), glm::radians(eulerAngle.x), ORIGIN_FORWARD) * glm::rotate(glm::mat4(1.0), glm::radians(eulerAngle.y), ORIGIN_LEFTWARD);

	Scale = glm::vec3(scale);
	ScaleMatrix = glm::scale(glm::mat4(1.0), scale);

	Forward = glm::mat3(RotationMatrix) * ORIGIN_FORWARD;

	Upward = glm::mat3(RotationMatrix) * ORIGIN_UPWARD;

	Leftward = glm::mat3(RotationMatrix) * ORIGIN_LEFTWARD;

	CalculateModelMatrix();
}

Transform::~Transform()
{
}

void Transform::SetPosition(glm::vec3 position)
{
	if (Position == position) return;
	Position = position;
	TranslationMatrix = glm::translate(glm::mat4(1.0), position);
	CalculateModelMatrix();
}

void Transform::SetEulerAngle(glm::vec3 eulerAngle)
{
	if (EulerAngle == eulerAngle) return;
	EulerAngle = eulerAngle;
	RotationMatrix = glm::rotate(glm::mat4(1.0), glm::radians(eulerAngle.z), ORIGIN_UPWARD) * glm::rotate(glm::mat4(1.0), glm::radians(eulerAngle.x), ORIGIN_FORWARD) * glm::rotate(glm::mat4(1.0), glm::radians(eulerAngle.y), ORIGIN_LEFTWARD);
	CalculateModelMatrix();
	Forward = glm::mat3(RotationMatrix) * ORIGIN_FORWARD;
	Upward = glm::mat3(RotationMatrix) * ORIGIN_UPWARD;
	Leftward = glm::mat3(RotationMatrix) * ORIGIN_LEFTWARD;
}

void Transform::SetScale(glm::vec3 scale)
{
	if (Scale == scale) return;
	Scale = scale;
	ScaleMatrix = glm::scale(glm::mat4(1.0), scale);
	CalculateModelMatrix();
}

void Transform::SetForward(glm::vec3 forward)
{
	if (Math::IsNearlyEqual(Forward, forward, 0.1f)) return;
	glm::vec3 C = glm::cross(Forward, forward);
	float angle = glm::degrees(glm::acos(glm::dot(Forward, forward) / (glm::length(Forward) * glm::length(forward))));
	glm::mat4 newRotateMatrix = mQuaternion::AxisAngle(glm::radians(angle), C);
	float EulerX = glm::degrees(std::atan2(newRotateMatrix[2][1], glm::sqrt(newRotateMatrix[2][0] * newRotateMatrix[2][0] + newRotateMatrix[2][2] * newRotateMatrix[2][2])));
	float EulerY = glm::degrees(std::atan2(-newRotateMatrix[2][0] / glm::cos(glm::radians(EulerX)), newRotateMatrix[2][2] / glm::cos(glm::radians(EulerX))));
	float EulerZ = glm::degrees(std::atan2(-newRotateMatrix[0][1] / glm::cos(glm::radians(EulerX)), newRotateMatrix[1][1] / glm::cos(glm::radians(EulerX))));

	SetEulerAngle(glm::vec3(EulerX, -EulerY, EulerZ) + EulerAngle);
}

void Transform::SetLeftward(glm::vec3 leftward)
{
	if (Math::IsNearlyEqual(Leftward, leftward, 0.1f)) return;
	glm::vec3 C = glm::cross(Leftward, leftward);
	float angle = glm::degrees(glm::acos(glm::dot(Leftward, leftward) / (glm::length(Leftward) * glm::length(leftward))));
	glm::mat4 newRotateMatrix = mQuaternion::AxisAngle(glm::radians(angle), C);
	float EulerX = glm::degrees(std::atan2(newRotateMatrix[2][1], glm::sqrt(newRotateMatrix[2][0] * newRotateMatrix[2][0] + newRotateMatrix[2][2] * newRotateMatrix[2][2])));
	float EulerY = glm::degrees(std::atan2(-newRotateMatrix[2][0] / glm::cos(glm::radians(EulerX)), newRotateMatrix[2][2] / glm::cos(glm::radians(EulerX))));
	float EulerZ = glm::degrees(std::atan2(-newRotateMatrix[0][1] / glm::cos(glm::radians(EulerX)), newRotateMatrix[1][1] / glm::cos(glm::radians(EulerX))));

	SetEulerAngle(glm::vec3(EulerX, -EulerY, EulerZ) + EulerAngle);
}

void Transform::SetUpward(glm::vec3 upward)
{
	if (Math::IsNearlyEqual(Upward, upward, 0.1f)) return;
	glm::vec3 C = glm::cross(Upward, upward);
	float angle = glm::degrees(glm::acos(glm::dot(Upward, upward) / (glm::length(Upward) * glm::length(upward))));
	glm::mat4 newRotateMatrix = mQuaternion::AxisAngle(glm::radians(angle), C);
	float EulerX = glm::degrees(std::atan2(newRotateMatrix[2][1], glm::sqrt(newRotateMatrix[2][0] * newRotateMatrix[2][0] + newRotateMatrix[2][2] * newRotateMatrix[2][2])));
	float EulerY = glm::degrees(std::atan2(-newRotateMatrix[2][0] / glm::cos(glm::radians(EulerX)), newRotateMatrix[2][2] / glm::cos(glm::radians(EulerX))));
	float EulerZ = glm::degrees(std::atan2(-newRotateMatrix[0][1] / glm::cos(glm::radians(EulerX)), newRotateMatrix[1][1] / glm::cos(glm::radians(EulerX))));

	SetEulerAngle(glm::vec3(EulerX, -EulerY, EulerZ) + EulerAngle);
}

void Transform::SetModelMatrix_PreFrame(glm::mat4 newMatrix)
{
	ModelMatrix_PreFrame = newMatrix;
}

glm::vec3 Transform::GetForward()
{
	return Forward;
}

glm::vec3 Transform::GetUpward()
{
	return Upward;
}

glm::vec3 Transform::GetLeftward()
{
	return Leftward;
}

glm::vec3 Transform::GetPosition()
{
	return Position;
}

glm::vec3 Transform::GetEulerAngle()
{
	return EulerAngle;
}

glm::vec3 Transform::GetScale()
{
	return Scale;
}

glm::mat4 Transform::GetTranslationMatrix()
{
	return TranslationMatrix;
}

glm::mat4 Transform::GetRotationMatrix()
{
	return RotationMatrix;
}

glm::mat4 Transform::GetScaleMatrix()
{
	return ScaleMatrix;
}

glm::mat4 Transform::GetModelMatrix()
{
	return ModelMatrix;
}

glm::mat4 Transform::GetModelMatrix_PreFrame()
{
	return ModelMatrix_PreFrame;
}

glm::mat4 Transform::GetModelMatrix_IT()
{
	return ModelMatrix_IT;
}

void Transform::CalculateModelMatrix()
{
	ModelMatrix = TranslationMatrix * ScaleMatrix * RotationMatrix;
	ModelMatrix_IT = glm::inverse(glm::transpose(ModelMatrix));
}
