#pragma once

#include "Transform.h"
#include "MaterialInstance.h"
#include "BufferManager.h"

enum LightType
{
	Direct,
	Point,
	Spot,
	Unknow
};

class Light
{
public:
	Light();
	~Light();

	LightType Type;
	void SetTransform(glm::vec3 position, glm::vec3 eulerAngle);
	void SetColor(glm::vec3 newColor);
	void SetShadowBias(float newBias);
	float GetShadowBias();

	Transform* GetTransform();

	virtual void GetShaderData(LightData &lightBuffer) = 0;

protected:
	Transform LightTransform;

	float ShadowBias;
	glm::vec3 Color;
	float Intensity;
};


class DirectLight : public Light
{
public:
	DirectLight();
	DirectLight(glm::vec3 eulerAngles);
	~DirectLight();

	glm::vec3 GetDirection();
	glm::vec3 GetUpDir();

	virtual void GetShaderData(LightData &lightBuffer) final {};
};

class PointLight : public Light
{
public:
	PointLight();
	~PointLight();

	void SetIntensity(float newIntensity);
	void SetAttenuationRadius(float newRadius);
	void SetSourceRadius(float Radius);
	void SetSoftSourceRadius(float newRadius);
	void SetSourceLength(float newLength);

	float GetAttenuationRadius();

	virtual void GetShaderData(LightData &lightBuffer);
protected:
	float AttenuationRadius;
	float InvRadius;
	float SourceRadius;
	float SoftSourceRadius;
	float SourceLength;

	float FallofExponent;
	float SpecularScale;
	float Brightness;
};

class SpotLight : public PointLight
{
public:
	SpotLight();
	~SpotLight();

	glm::vec3 GetDirection();
	glm::vec3 GetUpDir();
	
	void SetInnerConeAngle(float Degree);
	void SetOuterConeAngle(float Degree);
	
	float GetOutConeAngle();

	virtual void GetShaderData(LightData &lightBuffer) final;

private:
	float CosOuterCone;
	float CosInnerCone;
	float InvCosConeDifference;
	
	float InnerConeAngle;
	float OuterConeAngle;
};

