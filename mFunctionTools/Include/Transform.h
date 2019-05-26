#pragma once

#include "Util.h"
#include <cmath>

class Transform
{
public:
	Transform(Vector3f position, Vector3f eulerAngle, Vector3f scale);
	Transform();
	~Transform();

	void SetTransform(Vector3f position, Vector3f eulerAngle, Vector3f scale);
	void SetPosition(Vector3f position);
	void SetEulerAngle(Vector3f eulerAngle);
	void SetScale(Vector3f scale);
	void SetForward(Vector3f forward);
	void SetLeftward(Vector3f leftward);
	void SetUpward(Vector3f upward);
	void SetModelMatrix_PreFrame(Mat4f modelMatrix);

	Vector3f GetForward();
	Vector3f GetUpward();
	Vector3f GetLeftward();
	Vector3f GetPosition();
	Vector3f GetEulerAngle();
	Vector3f GetScale();
	Mat4f GetRotationMatrix();
	Mat4f GetTranslationMatrix();
	Mat4f GetScaleMatrix();
	Mat4f GetModelMatrix();
	Mat4f GetModelMatrix_PreFrame();
	Mat4f GetModelMatrix_IT();

private:
	Vector3f Position;
	Vector3f EulerAngle;
	Vector3f Scale;
	Vector3f Forward;
	Vector3f Upward;
	Vector3f Leftward;
	Mat4f RotationMatrix;
	Mat4f TranslationMatrix;
	Mat4f ScaleMatrix;
	Mat4f ModelMatrix;
	Mat4f ModelMatrix_PreFrame;
	Mat4f ModelMatrix_IT;

	void CalculateModelMatrix();

};

