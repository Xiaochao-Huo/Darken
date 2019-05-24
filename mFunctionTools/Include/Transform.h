#pragma once

#include "Util.h"
#include <cmath>

class Transform
{
public:
	Transform(glm::vec3 position, glm::vec3 eulerAngle, glm::vec3 scale);
	Transform();
	~Transform();

	void SetTransform(glm::vec3 position, glm::vec3 eulerAngle, glm::vec3 scale);
	void SetPosition(glm::vec3 position);
	void SetEulerAngle(glm::vec3 eulerAngle);
	void SetScale(glm::vec3 scale);
	void SetForward(glm::vec3 forward);
	void SetLeftward(glm::vec3 leftward);
	void SetUpward(glm::vec3 upward);
	void SetModelMatrix_PreFrame(glm::mat4 modelMatrix);

	glm::vec3 GetForward();
	glm::vec3 GetUpward();
	glm::vec3 GetLeftward();
	glm::vec3 GetPosition();
	glm::vec3 GetEulerAngle();
	glm::vec3 GetScale();
	glm::mat4 GetRotationMatrix();
	glm::mat4 GetTranslationMatrix();
	glm::mat4 GetScaleMatrix();
	glm::mat4 GetModelMatrix();
	glm::mat4 GetModelMatrix_PreFrame();
	glm::mat4 GetModelMatrix_IT();

private:
	glm::vec3 Position;
	glm::vec3 EulerAngle;
	glm::vec3 Scale;
	glm::vec3 Forward;
	glm::vec3 Upward;
	glm::vec3 Leftward;
	glm::mat4 RotationMatrix;
	glm::mat4 TranslationMatrix;
	glm::mat4 ScaleMatrix;
	glm::mat4 ModelMatrix;
	glm::mat4 ModelMatrix_PreFrame;
	glm::mat4 ModelMatrix_IT;

	void CalculateModelMatrix();

};

