#include "Camera.h"
#include "Quaternion.h"
#include "Util.h"
#include <glew.h>
#include <glfw3.h>

Camera::Camera()
{
	
}

Camera::~Camera()
{
}

Camera::Camera(glm::vec3 position, 
	glm::vec3 eulerAngle, 
	glm::float32 fovy, 
	glm::float32 aspect, 
	glm::float32 nearPlane, 
	glm::float32 farPlane, 
	glm::ivec2 viewPortSize)
{
	Init(position, eulerAngle, fovy, aspect, nearPlane, farPlane, viewPortSize);
}

void Camera::Init(glm::vec3 position,
	glm::vec3 eulerAngle,
	glm::float32 fovy,
	glm::float32 aspect,
	glm::float32 nearPlane,
	glm::float32 farPlane,
	glm::ivec2 viewPortSize)
{
	CameraTransform.SetTransform(position, eulerAngle, glm::vec3(1.0));
	ViewPortSize = viewPortSize;
	Fovy = fovy;
	Aspect = aspect;
	NearPlane = nearPlane;
	FarPlane = farPlane;
	CreateViewMatrix();
	ViewMatrix_PreFrame = ViewMatrix;
	CreateProjectionMatrix();
	CreateVPMatrix();
	NextCamera = nullptr;
}

void Camera::CreateViewMatrix()
{
	glm::mat4 viewMatrix = glm::mat4(1.0);
	ViewMatrix = glm::lookAtLH(CameraTransform.GetPosition(), CameraTransform.GetPosition() + CameraTransform.GetForward(), CameraTransform.GetUpward());
}

void Camera::CreateProjectionMatrix()
{
	ProjectMatrix = glm::perspectiveLH(Fovy,
		Aspect,
		NearPlane,
		FarPlane
		);
	ProjectMatrix_PreFrame = ProjectMatrix;
}

void Camera::CreateVPMatrix()
{
	VPMatrix = ProjectMatrix * ViewMatrix;
	VPMatrix_I = glm::inverse(VPMatrix);
}

void Camera::ModifyProjectionForClipping(glm::vec4 vClipPlane)
{
	glm::vec4 vClipPlaneView(vClipPlane * glm::inverse(ViewMatrix));	// put clip plane into view coords
	/*
	Calculate the clip-space corner point opposite the clipping plane
	and transform it into camera space by multiplying it by the inverse
	projection matrix.
	*/
	glm::vec4 vClipSpaceCorner(sgn(vClipPlaneView.x), sgn(vClipPlaneView.y), 1.0f, 1.0f);
	vClipSpaceCorner = vClipSpaceCorner * glm::inverse(ProjectMatrix);

	// Calculate the scaled plane vector
	glm::vec4 vScaledPlane = vClipPlaneView * (2.0f / glm::dot(vClipSpaceCorner, vClipPlaneView));

	// Replace the third row of the matrix
	glm::value_ptr(ProjectMatrix)[2] = vScaledPlane.x;
	glm::value_ptr(ProjectMatrix)[6] = vScaledPlane.y;
	glm::value_ptr(ProjectMatrix)[10] = vScaledPlane.z + 1.0f;
	glm::value_ptr(ProjectMatrix)[14] = vScaledPlane.w;
	CreateVPMatrix();
}

void Camera::SetPosition(glm::vec3 position)
{
	

	if (CameraTransform.GetPosition() == position) return;
	CameraTransform.SetPosition(position);
	CreateViewMatrix();
	CreateVPMatrix();
}

void Camera::SetPosition(glm::float32 x, glm::float32 y, glm::float32 z)
{
	glm::vec3 newPosition(x, y, z);
	if (CameraTransform.GetPosition() == newPosition) return;
	CameraTransform.SetPosition(newPosition);
	CreateViewMatrix();
	CreateVPMatrix();	
}

void Camera::SetEulerAngle(glm::vec3 eulerAngle)
{
	if (CameraTransform.GetEulerAngle() == eulerAngle) return;
	CameraTransform.SetEulerAngle(eulerAngle);
	CreateViewMatrix();
	CreateVPMatrix();
}

void Camera::SetEulerAngle(glm::float32 x, glm::float32 y, glm::float32 z)
{
	glm::vec3 newEulerAngle(x, y, z);
	if (CameraTransform.GetEulerAngle() == newEulerAngle) return;
	CameraTransform.SetEulerAngle(newEulerAngle);
	CreateViewMatrix();
	CreateVPMatrix();
}

void Camera::SetFovy(glm::float32 fovy)
{
	fovy = fovy;
	CreateProjectionMatrix();
	CreateVPMatrix();
}

void Camera::SetAspect(glm::float32 aspect)
{
	aspect = aspect;
	CreateProjectionMatrix();
	CreateVPMatrix();
}

void Camera::SetNearPlaneDis(glm::float32 nearDis)
{
	NearPlane = nearDis;
	CreateProjectionMatrix();
	CreateVPMatrix();
}

void Camera::SetFarPlaneDis(glm::float32 farDis)
{
	FarPlane = farDis;
	CreateProjectionMatrix();
	CreateVPMatrix();
}

void Camera::SetProjectMatrix(glm::mat4 newMatrix)
{
	ProjectMatrix = newMatrix;
	CreateVPMatrix();
}

void Camera::SetViewMatrix_PreFrame(glm::mat4 newMatrix)
{
	ViewMatrix_PreFrame = newMatrix;
}

void Camera::SetProjectMatrix_PreFrame(glm::mat4 newMatrix)
{
	ProjectMatrix_PreFrame = newMatrix;
}

void Camera::SetDirection(const glm::vec3& forward, const glm::vec3& up)
{
	CameraTransform.SetForward(forward);
	CameraTransform.SetUpward(up);
	CreateViewMatrix();
	CreateVPMatrix();
}

glm::mat4 Camera::GetViewMatrix()
{
	return ViewMatrix;
}

glm::mat4 Camera::GetViewMatrix_PreFrame()
{
	return ViewMatrix_PreFrame;
}

glm::mat4 Camera::GetProjectMatrix()
{
	return ProjectMatrix;
}

glm::mat4 Camera::GetProjectMatrix_PreFrame() 
{
	return ProjectMatrix_PreFrame;
}

glm::mat4 Camera::GetVPMatrix()
{
	return VPMatrix;
}

glm::mat4 Camera::GetVPMatrix_I()
{
	return VPMatrix_I;
}

glm::vec3 Camera::GetEulerAngle()
{
	return CameraTransform.GetEulerAngle();
}

glm::vec3 Camera::GetPosition()
{
	return CameraTransform.GetPosition();
}

glm::vec3 Camera::GetForward()
{
	return CameraTransform.GetForward();
}

glm::vec3 Camera::GetUpward()
{
	return CameraTransform.GetUpward();
}

glm::vec3 Camera::GetLeftward()
{
	return CameraTransform.GetLeftward();
}

float Camera::GetFOVinRadians()
{
	return Fovy;
}

float Camera::GetAspect()
{
	return Aspect;
}

float Camera::GetNearClipPlaneDis()
{
	return NearPlane;
}

float Camera::GetFarClipPlaneDis()
{
	return FarPlane;
}

void Camera::ReCalculateProjectMatrix()
{
	CreateProjectionMatrix();
}

float Camera::sgn(float a)
{
	if (a > 0.0f) return(1.0f);
	if (a < 0.0f) return(-1.0f);
	return 0.0f;
}

glm::ivec2 Camera::GetViewPortSize()
{
	return ViewPortSize;
}

void Camera::ActiveViewPort()
{
	glViewport(0, 0, ViewPortSize.x, ViewPortSize.y);
}

void Camera::SetNextCamera(std::shared_ptr<Camera> camera)
{
	NextCamera = camera;
}

std::shared_ptr<Camera> Camera::GetNextCamera()
{
	return NextCamera;
}