#pragma once

#include "glm.hpp"
#include "Transform.h"

class Camera
{
public:
	Camera();
	~Camera();
	Camera(glm::vec3 position,
		glm::vec3 eulerAngle,
		glm::float32 fovy,
		glm::float32 aspect,
		glm::float32 nearPlane,
		glm::float32 farPlane,
		glm::ivec2 viewPortSize
		);
	void Init(glm::vec3 position,
		glm::vec3 eulerAngle,
		glm::float32 fovy,
		glm::float32 aspect,
		glm::float32 nearPlane,
		glm::float32 farPlane,
		glm::ivec2 viewPortSize);
	void ModifyProjectionForClipping(glm::vec4 vClipPlane);

	glm::mat4 GetViewMatrix();
	glm::mat4 GetViewMatrix_PreFrame();
	glm::mat4 GetProjectMatrix_PreFrame();
	glm::mat4 GetProjectMatrix();
	glm::mat4 GetVPMatrix();
	glm::mat4 GetVPMatrix_I();
	glm::vec3 GetEulerAngle();
	glm::vec3 GetPosition();
	glm::vec3 GetForward();
	glm::vec3 GetUpward();
	glm::vec3 GetLeftward();
	float GetFOVinRadians();
	float GetAspect();
	float GetFarClipPlaneDis();
	float GetNearClipPlaneDis();
	glm::ivec2 GetViewPortSize();
	void SetPosition(glm::vec3 position);
	void SetPosition(glm::float32 x, glm::float32 y, glm::float32 z);
	void SetEulerAngle(glm::vec3 eulerAngle);
	void SetEulerAngle(glm::float32 x, glm::float32 y, glm::float32 z);
	void SetFovy(glm::float32 fovy);
	void SetAspect(glm::float32 aspect);
	void SetNearPlaneDis(glm::float32 nearDis);
	void SetFarPlaneDis(glm::float32 farDis);
	void SetProjectMatrix(glm::mat4 newMatrix);
	void SetViewMatrix_PreFrame(glm::mat4 newMatrix);
	void SetProjectMatrix_PreFrame(glm::mat4 newMatrix);
	void ReCalculateProjectMatrix();
	void SetDirection(const glm::vec3& forward, const glm::vec3& up);

	void ActiveViewPort();

	void SetNextCamera(std::shared_ptr<Camera> camera);
	std::shared_ptr<Camera> GetNextCamera();
private:
	//glm::vec3 Position;
	//glm::vec3 EulerAngle;
	glm::float32 Fovy;
	glm::float32 Aspect;
	glm::float32 NearPlane;
	glm::float32 FarPlane;
	//glm::vec3 Forward;
	//glm::vec3 Up;
	//glm::vec3 Left;

	glm::vec2 ViewPortSize;

	void CreateViewMatrix();
	void CreateProjectionMatrix();
	void CreateVPMatrix();
	//glm::mat4 RotationMatrix;
	glm::mat4 ViewMatrix;
	glm::mat4 ProjectMatrix;
	glm::mat4 VPMatrix;
	glm::mat4 VPMatrix_I;

	glm::mat4 ViewMatrix_PreFrame;
	glm::mat4 ProjectMatrix_PreFrame;

	Transform CameraTransform;

	float sgn(float a);

	std::shared_ptr<Camera> NextCamera;
};
