#include "Light.h"

Light::Light()
{
	LightTransform = Transform();
	Type = Unknow;
	ShadowBias = 0.0;
}

Light::~Light()
{
}

void Light::SetColor(glm::vec3 newColor)
{
	Color = newColor;
}

void Light::SetShadowBias(float newBias)
{
	ShadowBias = newBias;
}

float Light::GetShadowBias()
{
	return ShadowBias;
}

void Light::SetTransform(glm::vec3 position, glm::vec3 eulerAngle)
{
	LightTransform.SetTransform(position, eulerAngle, glm::vec3(1.0));
}

Transform* Light::GetTransform()
{
	return &LightTransform;
}


DirectLight::DirectLight()
{
	LightTransform.SetPosition(glm::vec3(FLT_MAX));
	Type = LightType::Direct;
}

DirectLight::DirectLight(glm::vec3 eulerAngles) 
{
	LightTransform.SetEulerAngle(eulerAngles);
	Type = LightType::Direct;
}

glm::vec3 DirectLight::GetDirection()
{
	return -LightTransform.GetForward();
}

glm::vec3 DirectLight::GetUpDir()
{
	return LightTransform.GetUpward();
}

DirectLight::~DirectLight()
{
}

PointLight::PointLight()
{
	Type = LightType::Point;
	FallofExponent = 8.0f;
	SpecularScale = 1.0f;
}

PointLight::~PointLight()
{
}

void PointLight::SetIntensity(float newIntensity)
{
	Intensity = newIntensity;
	Brightness = Intensity * 16.0f;
}

void PointLight::SetAttenuationRadius(float newRadius)
{
	AttenuationRadius = newRadius;
	InvRadius = 1.0f / AttenuationRadius;
}

void PointLight::SetSourceRadius(float Radius)
{
	SourceRadius = Radius;
}

void PointLight::SetSoftSourceRadius(float newRadius)
{
	SoftSourceRadius = newRadius;
}

void PointLight::SetSourceLength(float newLength)
{
	SourceLength = newLength;
}

float PointLight::GetAttenuationRadius()
{
	return AttenuationRadius;
}

void PointLight::GetShaderData(LightData &lightBuffer)
{
	lightBuffer.LightPosition = LightTransform.GetPosition();
	lightBuffer.LightInvRadius = InvRadius;
	lightBuffer.LightColor = Color * Brightness;
	lightBuffer.LightFallofExponent = FallofExponent;
	lightBuffer.NormalizedLightDirection = glm::vec3(-1.0, 0.0, 0.0);
	lightBuffer.NormalizedLightTangent = glm::vec3(0.0, 0.0, 1.0);
	lightBuffer.LightSpotAngles = glm::vec2(-2.0f, 1.0f);
	lightBuffer.LightSourceRadius = SourceRadius;
	lightBuffer.LightSourceLength = SourceLength;
	lightBuffer.LightSoftSourceRadius = SoftSourceRadius;
	lightBuffer.LightSpecularScale = SpecularScale;
	lightBuffer.LightContactShadowLength = 0.0;
	lightBuffer.LightDistanceFadeMAD = glm::vec2(0.0f);
	lightBuffer.LightShadowMapChannelMask = glm::vec4(0.0f);
	lightBuffer.LightShadowedBits = 3;
	lightBuffer.LightType = 2;
}

SpotLight::SpotLight()
{
	Type = LightType::Spot;
}

SpotLight::~SpotLight()
{
}

glm::vec3 SpotLight::GetDirection()
{
	return -LightTransform.GetForward();
}

glm::vec3 SpotLight::GetUpDir()
{
	return LightTransform.GetUpward();
}

void SpotLight::GetShaderData(LightData &lightBuffer)
{
	lightBuffer.LightPosition = LightTransform.GetPosition();
	lightBuffer.LightInvRadius = InvRadius;
	lightBuffer.LightColor = Color * Brightness;
	lightBuffer.LightFallofExponent = FallofExponent;
	lightBuffer.NormalizedLightDirection = GetDirection();
	lightBuffer.NormalizedLightTangent = LightTransform.GetUpward();
	lightBuffer.LightSpotAngles = glm::vec2(CosOuterCone, InvCosConeDifference);
	lightBuffer.LightSourceRadius = SourceRadius;
	lightBuffer.LightSourceLength = SourceLength;
	lightBuffer.LightSoftSourceRadius = SoftSourceRadius;
	lightBuffer.LightSpecularScale = SpecularScale;
	lightBuffer.LightContactShadowLength = 0.0;
	lightBuffer.LightDistanceFadeMAD = glm::vec2(0.0f);
	lightBuffer.LightShadowMapChannelMask = glm::vec4(0.0f);
	lightBuffer.LightShadowedBits = 3;
	lightBuffer.LightType = 1;
}

void SpotLight::SetInnerConeAngle(float Degree)
{
	InnerConeAngle = Degree;
	CosInnerCone = glm::cos(InnerConeAngle / 180.0f * 3.14159265358979323846264338327950288f);
	InvCosConeDifference = 1.0f / (CosInnerCone - CosOuterCone);
}

void SpotLight::SetOuterConeAngle(float Degree)
{
	OuterConeAngle = Degree;
	CosOuterCone = glm::cos(OuterConeAngle / 180.0f * 3.14159265358979323846264338327950288f);
	InvCosConeDifference = 1.0f / (CosInnerCone - CosOuterCone);
}

float SpotLight::GetOutConeAngle()
{
	return OuterConeAngle;
}
