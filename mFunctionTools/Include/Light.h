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
	void SetTransform(Vector3f position, Vector3f eulerAngle);
	void SetColor(Vector3f newColor);
	void SetShadowBias(Float32 newBias);
	Float32 GetShadowBias();

	Transform* GetTransform();

	virtual void GetShaderData(LightData &lightBuffer) = 0;

protected:
	Transform LightTransform;

	Float32 ShadowBias;
	Vector3f Color;
	Float32 Intensity;
};


class DirectLight : public Light
{
public:
	DirectLight();
	DirectLight(Vector3f eulerAngles);
	~DirectLight();

	Vector3f GetDirection();
	Vector3f GetUpDir();

	virtual void GetShaderData(LightData &lightBuffer) final {};
};

class PointLight : public Light
{
public:
	PointLight();
	~PointLight();

	void SetIntensity(Float32 newIntensity);
	void SetAttenuationRadius(Float32 newRadius);
	void SetSourceRadius(Float32 Radius);
	void SetSoftSourceRadius(Float32 newRadius);
	void SetSourceLength(Float32 newLength);

	Float32 GetAttenuationRadius();

	virtual void GetShaderData(LightData &lightBuffer);
protected:
	Float32 AttenuationRadius;
	Float32 InvRadius;
	Float32 SourceRadius;
	Float32 SoftSourceRadius;
	Float32 SourceLength;

	Float32 FallofExponent;
	Float32 SpecularScale;
	Float32 Brightness;
};

class SpotLight : public PointLight
{
public:
	SpotLight();
	~SpotLight();

	Vector3f GetDirection();
	Vector3f GetUpDir();
	
	void SetInnerConeAngle(Float32 Degree);
	void SetOuterConeAngle(Float32 Degree);
	
	Float32 GetOutConeAngle();

	virtual void GetShaderData(LightData &lightBuffer) final;

private:
	Float32 CosOuterCone;
	Float32 CosInnerCone;
	Float32 InvCosConeDifference;
	
	Float32 InnerConeAngle;
	Float32 OuterConeAngle;
};

