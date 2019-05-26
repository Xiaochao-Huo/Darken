#include "Transform.h"
#include "Quaternion.h"

Transform::Transform(Vector3f position, Vector3f eulerAngle, Vector3f scale)
{
	Position = Vector3f(position);
	TranslationMatrix = Mat4f(Math::Translate(Mat4f(1.0), position));

	EulerAngle = Vector3f(eulerAngle);
	RotationMatrix = Math::Rotate(Mat4f(1.0), Math::Radians(eulerAngle.z), ORIGIN_UPWARD) * Math::Rotate(Mat4f(1.0), Math::Radians(eulerAngle.x), ORIGIN_FORWARD) * Math::Rotate(Mat4f(1.0), Math::Radians(eulerAngle.y), ORIGIN_LEFTWARD);
	
	Scale = Vector3f(scale);
	ScaleMatrix = Math::Scale(Mat4f(1.0), scale);

	Forward = Mat3f(RotationMatrix) * ORIGIN_FORWARD;
	
	Upward = Mat3f(RotationMatrix) * ORIGIN_UPWARD;
	
	Leftward = Mat3f(RotationMatrix) * ORIGIN_LEFTWARD;

	CalculateModelMatrix();
	ModelMatrix_PreFrame = ModelMatrix;
}

Transform::Transform()
{
	Position = Vector3f(0.0, 0.0, 0.0);
	TranslationMatrix = Mat4f(1.0);

	EulerAngle = Vector3f(0.0, 0.0, 0.0);
	RotationMatrix = Mat4f(1.0);

	Scale = Vector3f(1.0, 1.0, 1.0);
	ScaleMatrix = Mat4f(1.0);
	
	Forward = ORIGIN_FORWARD;

	Upward = ORIGIN_UPWARD;

	Leftward = ORIGIN_LEFTWARD;

	CalculateModelMatrix();
	ModelMatrix_PreFrame = ModelMatrix;
}

void Transform::SetTransform(Vector3f position, Vector3f eulerAngle, Vector3f scale)
{
	Position = Vector3f(position);
	TranslationMatrix = Mat4f(Math::Translate(Mat4f(1.0), position));

	EulerAngle = Vector3f(eulerAngle);
	RotationMatrix = Math::Rotate(Mat4f(1.0), Math::Radians(eulerAngle.z), ORIGIN_UPWARD) * Math::Rotate(Mat4f(1.0), Math::Radians(eulerAngle.x), ORIGIN_FORWARD) * Math::Rotate(Mat4f(1.0), Math::Radians(eulerAngle.y), ORIGIN_LEFTWARD);

	Scale = Vector3f(scale);
	ScaleMatrix = Math::Scale(Mat4f(1.0), scale);

	Forward = Mat3f(RotationMatrix) * ORIGIN_FORWARD;

	Upward = Mat3f(RotationMatrix) * ORIGIN_UPWARD;

	Leftward = Mat3f(RotationMatrix) * ORIGIN_LEFTWARD;

	CalculateModelMatrix();
}

Transform::~Transform()
{
}

void Transform::SetPosition(Vector3f position)
{
	if (Position == position) return;
	Position = position;
	TranslationMatrix = Math::Translate(Mat4f(1.0), position);
	CalculateModelMatrix();
}

void Transform::SetEulerAngle(Vector3f eulerAngle)
{
	if (EulerAngle == eulerAngle) return;
	EulerAngle = eulerAngle;
	RotationMatrix = Math::Rotate(Mat4f(1.0), Math::Radians(eulerAngle.z), ORIGIN_UPWARD) * Math::Rotate(Mat4f(1.0), Math::Radians(eulerAngle.x), ORIGIN_FORWARD) * Math::Rotate(Mat4f(1.0), Math::Radians(eulerAngle.y), ORIGIN_LEFTWARD);
	CalculateModelMatrix();
	Forward = Mat3f(RotationMatrix) * ORIGIN_FORWARD;
	Upward = Mat3f(RotationMatrix) * ORIGIN_UPWARD;
	Leftward = Mat3f(RotationMatrix) * ORIGIN_LEFTWARD;
}

void Transform::SetScale(Vector3f scale)
{
	if (Scale == scale) return;
	Scale = scale;
	ScaleMatrix = Math::Scale(Mat4f(1.0), scale);
	CalculateModelMatrix();
}

void Transform::SetForward(Vector3f forward)
{
	if (Math::IsNearlyEqual(Forward, forward, 0.1f)) return;
	Vector3f C = Math::Cross(Forward, forward);
	Float32 angle = Math::Degrees(Math::aCos(Math::Dot(Forward, forward) / (Math::Length(Forward) * Math::Length(forward))));
	Mat4f newRotateMatrix = mQuaternion::AxisAngle(Math::Radians(angle), C);
	Float32 EulerX = Math::Degrees(std::atan2(newRotateMatrix[2][1], Math::Sqrt(newRotateMatrix[2][0] * newRotateMatrix[2][0] + newRotateMatrix[2][2] * newRotateMatrix[2][2])));
	Float32 EulerY = Math::Degrees(std::atan2(-newRotateMatrix[2][0] / Math::Cos(Math::Radians(EulerX)), newRotateMatrix[2][2] / Math::Cos(Math::Radians(EulerX))));
	Float32 EulerZ = Math::Degrees(std::atan2(-newRotateMatrix[0][1] / Math::Cos(Math::Radians(EulerX)), newRotateMatrix[1][1] / Math::Cos(Math::Radians(EulerX))));

	SetEulerAngle(Vector3f(EulerX, -EulerY, EulerZ) + EulerAngle);
}

void Transform::SetLeftward(Vector3f leftward)
{
	if (Math::IsNearlyEqual(Leftward, leftward, 0.1f)) return;
	Vector3f C = Math::Cross(Leftward, leftward);
	Float32 angle = Math::Degrees(Math::aCos(Math::Dot(Leftward, leftward) / (Math::Length(Leftward) * Math::Length(leftward))));
	Mat4f newRotateMatrix = mQuaternion::AxisAngle(Math::Radians(angle), C);
	Float32 EulerX = Math::Degrees(std::atan2(newRotateMatrix[2][1], Math::Sqrt(newRotateMatrix[2][0] * newRotateMatrix[2][0] + newRotateMatrix[2][2] * newRotateMatrix[2][2])));
	Float32 EulerY = Math::Degrees(std::atan2(-newRotateMatrix[2][0] / Math::Cos(Math::Radians(EulerX)), newRotateMatrix[2][2] / Math::Cos(Math::Radians(EulerX))));
	Float32 EulerZ = Math::Degrees(std::atan2(-newRotateMatrix[0][1] / Math::Cos(Math::Radians(EulerX)), newRotateMatrix[1][1] / Math::Cos(Math::Radians(EulerX))));

	SetEulerAngle(Vector3f(EulerX, -EulerY, EulerZ) + EulerAngle);
}

void Transform::SetUpward(Vector3f upward)
{
	if (Math::IsNearlyEqual(Upward, upward, 0.1f)) return;
	Vector3f C = Math::Cross(Upward, upward);
	Float32 angle = Math::Degrees(Math::aCos(Math::Dot(Upward, upward) / (Math::Length(Upward) * Math::Length(upward))));
	Mat4f newRotateMatrix = mQuaternion::AxisAngle(Math::Radians(angle), C);
	Float32 EulerX = Math::Degrees(std::atan2(newRotateMatrix[2][1], Math::Sqrt(newRotateMatrix[2][0] * newRotateMatrix[2][0] + newRotateMatrix[2][2] * newRotateMatrix[2][2])));
	Float32 EulerY = Math::Degrees(std::atan2(-newRotateMatrix[2][0] / Math::Cos(Math::Radians(EulerX)), newRotateMatrix[2][2] / Math::Cos(Math::Radians(EulerX))));
	Float32 EulerZ = Math::Degrees(std::atan2(-newRotateMatrix[0][1] / Math::Cos(Math::Radians(EulerX)), newRotateMatrix[1][1] / Math::Cos(Math::Radians(EulerX))));

	SetEulerAngle(Vector3f(EulerX, -EulerY, EulerZ) + EulerAngle);
}

void Transform::SetModelMatrix_PreFrame(Mat4f newMatrix)
{
	ModelMatrix_PreFrame = newMatrix;
}

Vector3f Transform::GetForward()
{
	return Forward;
}

Vector3f Transform::GetUpward()
{
	return Upward;
}

Vector3f Transform::GetLeftward()
{
	return Leftward;
}

Vector3f Transform::GetPosition()
{
	return Position;
}

Vector3f Transform::GetEulerAngle()
{
	return EulerAngle;
}

Vector3f Transform::GetScale()
{
	return Scale;
}

Mat4f Transform::GetTranslationMatrix()
{
	return TranslationMatrix;
}

Mat4f Transform::GetRotationMatrix()
{
	return RotationMatrix;
}

Mat4f Transform::GetScaleMatrix()
{
	return ScaleMatrix;
}

Mat4f Transform::GetModelMatrix()
{
	return ModelMatrix;
}

Mat4f Transform::GetModelMatrix_PreFrame()
{
	return ModelMatrix_PreFrame;
}

Mat4f Transform::GetModelMatrix_IT()
{
	return ModelMatrix_IT;
}

void Transform::CalculateModelMatrix()
{
	ModelMatrix = TranslationMatrix * ScaleMatrix * RotationMatrix;
	ModelMatrix_IT = Math::Inverse(Math::Transpose(ModelMatrix));
}
