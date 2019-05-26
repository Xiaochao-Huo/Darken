#pragma once

#include "Transform.h"

class Camera
{
public:
	Camera();
	~Camera();
	Camera(Vector3f position, Vector3f eulerAngle, Float32 fovy, Float32 aspect, Float32 nearPlane, Float32 farPlane, Vector2i viewPortSize);
	void Init(Vector3f position, Vector3f eulerAngle, Float32 fovy, Float32 aspect, Float32 nearPlane, Float32 farPlane, Vector2i viewPortSize);
	//void ModifyProjectionForClipping(Vector4f vClipPlane);

	Mat4f GetViewMatrix();
	Mat4f GetViewMatrix_PreFrame();
	Mat4f GetProjectMatrix_PreFrame();
	Mat4f GetProjectMatrix();
	Mat4f GetVPMatrix();
	Mat4f GetVPMatrix_I();
	Vector3f GetEulerAngle();
	Vector3f GetPosition();
	Vector3f GetForward();
	Vector3f GetUpward();
	Vector3f GetLeftward();
	Float32 GetFOVinRadians();
	Float32 GetAspect();
	Float32 GetFarClipPlaneDis();
	Float32 GetNearClipPlaneDis();
	Vector2i GetViewPortSize();
	void SetPosition(Vector3f position);
	void SetPosition(Float32 x, Float32 y, Float32 z);
	void SetEulerAngle(Vector3f eulerAngle);
	void SetEulerAngle(Float32 x, Float32 y, Float32 z);
	void SetFovy(Float32 fovy);
	void SetAspect(Float32 aspect);
	void SetNearPlaneDis(Float32 nearDis);
	void SetFarPlaneDis(Float32 farDis);
	void SetProjectMatrix(Mat4f newMatrix);
	void SetViewMatrix_PreFrame(Mat4f newMatrix);
	void SetProjectMatrix_PreFrame(Mat4f newMatrix);
	void ReCalculateProjectMatrix();
	void SetDirection(const Vector3f& forward, const Vector3f& up);

	void ActiveViewPort();

	void SetNextCamera(std::shared_ptr<Camera> camera);
	std::shared_ptr<Camera> GetNextCamera();
private:

	Float32 Fovy;
	Float32 Aspect;
	Float32 NearPlane;
	Float32 FarPlane;

	Vector2i ViewPortSize;

	void CreateViewMatrix();
	void CreateProjectionMatrix();
	void CreateVPMatrix();

	Mat4f ViewMatrix;
	Mat4f ProjectMatrix;
	Mat4f VPMatrix;
	Mat4f VPMatrix_I;

	Mat4f ViewMatrix_PreFrame;
	Mat4f ProjectMatrix_PreFrame;

	Transform CameraTransform;

	Float32 sgn(Float32 a);

	std::shared_ptr<Camera> NextCamera;
};
