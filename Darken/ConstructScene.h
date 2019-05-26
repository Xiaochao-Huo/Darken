#pragma once

#include "Model.h"
#include "Texture.h"

class SimpleObject : public Model
{
public:
	SimpleObject(std::string ModelFileName, std::shared_ptr<MaterialInstance> ShadowDepthMaterialInst, std::shared_ptr<MaterialInstance> LightingMaterialInst)
	{
		LoadModelFromAsset(ModelFileName);
		BindMaterial(ShadowDepthMaterialInst, LightingMaterialInst);
		ObjectTransform.SetScale(Vector3f(1.0, 1.0, 1.0));
		ObjectTransform.SetEulerAngle(Vector3f(0.0, 0.0, 0.0));
		ObjectTransform.SetPosition(Vector3f(0.0, 0.0, 0.0));
	};
	~SimpleObject() {};

	virtual void Start() {};
	virtual void Update() {};

private:

};


class DigitalHuman : public Model
{
public:
	DigitalHuman(std::string ModelFileName, std::shared_ptr<MaterialInstance> ShadowDepthMaterialInst, std::shared_ptr<MaterialInstance> LightingMaterialInst)
	{
		LoadModelFromAsset(ModelFileName);
		BindMaterial(ShadowDepthMaterialInst, LightingMaterialInst);
		ObjectTransform.SetScale(Vector3f(1.0, 1.0, 1.0));
		ObjectTransform.SetEulerAngle(Vector3f(0.0, 0.0, 0.0));
		ObjectTransform.SetPosition(Vector3f(0.0, 0.0, 0.0));
	};
	~DigitalHuman() {};

	Float32 step = 0.005f;
	virtual void Start(){};
	virtual void Update()
	{
		Vector3f eulerAngle = ObjectTransform.GetEulerAngle();
		//ObjectTransform.SetEulerAngle(eulerAngle + Vector3f(0.0, 0.0, -0.5));
		Vector3f position = ObjectTransform.GetPosition();
		//ObjectTransform.SetPosition(Vector3f(0.0, step, 0.0) + position);
		if(position.y < - 5.0)
		{
			step = 0.005f;
		}
		else if (position.y > 5.0)
		{
			step = -0.005f;
		}
	};

private:

};

class LightGroup
{
public:
	LightGroup(){
		Intensity = 120000.0f;
		Color = Vector3f(1.0, 0.98, 0.975);
		FocalAngleOuter = 8.0f;
		FocalAngleInner = 0.0f;
		AttenuationDistance = 700.0f;
		LightWidth = 90.0f;
		LightLength = 120.0f;
		CastShadows = true;
		LightSamplesSquared = 2;
		SourceRadiusMult = 0.809524f;
		CenterOfInterestLength = 350.0f;
		SetEnabled = true;
		SoftRadius = 13.194313f;
		ShadowBias = 0.025f;

		Vector3f TotalTranslate = Vector3f(0.0, 0.0, 10.0);

		std::shared_ptr<SpotLight> SpotLightKey0 = std::shared_ptr<SpotLight>(new SpotLight());
		SpotLightKey0->GetTransform()->SetPosition(Vector3f(-148.65f, 309.37256f, 137.21f) + TotalTranslate);
		SpotLightKey0->GetTransform()->SetForward(Vector3f(0.0) - SpotLightKey0->GetTransform()->GetPosition() + TotalTranslate);
		
		SpotLightKey0->SetIntensity(GetIntensity());
		SpotLightKey0->SetOuterConeAngle(FocalAngleOuter);
		SpotLightKey0->SetInnerConeAngle(FocalAngleInner);
		SpotLightKey0->SetColor(Color);
		SpotLightKey0->SetAttenuationRadius(AttenuationDistance);
		SpotLightKey0->SetSourceRadius(GetSourceRadius());
		SpotLightKey0->SetSourceLength(GetSourceLength());
		SpotLightKey0->SetSoftSourceRadius(SoftRadius);
		SpotLightKey0->SetShadowBias(ShadowBias);		

		std::shared_ptr<SpotLight> SpotLightKey1 = std::shared_ptr<SpotLight>(new SpotLight());
		SpotLightKey1->GetTransform()->SetPosition(Vector3f(-212.18f, 284.45f, 106.085f) + TotalTranslate);
		SpotLightKey1->GetTransform()->SetForward(Vector3f(0.0) - SpotLightKey1->GetTransform()->GetPosition() + TotalTranslate);
		SpotLightKey1->SetIntensity(GetIntensity());
		SpotLightKey1->SetOuterConeAngle(FocalAngleOuter);
		SpotLightKey1->SetInnerConeAngle(FocalAngleInner);
		SpotLightKey1->SetColor(Color);
		SpotLightKey1->SetAttenuationRadius(AttenuationDistance);
		SpotLightKey1->SetSourceRadius(GetSourceRadius());
		SpotLightKey1->SetSourceLength(GetSourceLength());
		SpotLightKey1->SetSoftSourceRadius(SoftRadius);
		SpotLightKey1->SetShadowBias(ShadowBias);

		std::shared_ptr<SpotLight> SpotLightKey2 = std::shared_ptr<SpotLight>(new SpotLight());
		SpotLightKey2->GetTransform()->SetPosition(Vector3f(-225.654f, 290.605f, 63.5933f) + TotalTranslate);
		SpotLightKey2->GetTransform()->SetForward(Vector3f(0.0) - SpotLightKey2->GetTransform()->GetPosition() + TotalTranslate);
		SpotLightKey2->SetIntensity(GetIntensity());
		SpotLightKey2->SetOuterConeAngle(FocalAngleOuter);
		SpotLightKey2->SetInnerConeAngle(FocalAngleInner);
		SpotLightKey2->SetColor(Color);
		SpotLightKey2->SetAttenuationRadius(AttenuationDistance);
		SpotLightKey2->SetSourceRadius(GetSourceRadius());
		SpotLightKey2->SetSourceLength(GetSourceLength());
		SpotLightKey2->SetSoftSourceRadius(SoftRadius);
		SpotLightKey2->SetShadowBias(ShadowBias);

		std::shared_ptr<SpotLight> SpotLightKey3 = std::shared_ptr<SpotLight>(new SpotLight());
		SpotLightKey3->GetTransform()->SetPosition(Vector3f(-175.6046f, 321.6834f, 52.225f) + TotalTranslate);
		SpotLightKey3->GetTransform()->SetForward(Vector3f(0.0) - SpotLightKey3->GetTransform()->GetPosition() + TotalTranslate);
		SpotLightKey3->SetIntensity(GetIntensity());
		SpotLightKey3->SetOuterConeAngle(FocalAngleOuter);
		SpotLightKey3->SetInnerConeAngle(FocalAngleInner);
		SpotLightKey3->SetColor(Color);
		SpotLightKey3->SetAttenuationRadius(AttenuationDistance);
		SpotLightKey3->SetSourceRadius(GetSourceRadius());
		SpotLightKey3->SetSourceLength(GetSourceLength());
		SpotLightKey3->SetSoftSourceRadius(SoftRadius);
		SpotLightKey3->SetShadowBias(ShadowBias);

		std::shared_ptr<SpotLight> SpotLightKey4 = std::shared_ptr<SpotLight>(new SpotLight());
		SpotLightKey4->GetTransform()->SetPosition(Vector3f(-198.706f, 278.2945f, 148.5773f) + TotalTranslate);
		SpotLightKey4->GetTransform()->SetForward(Vector3f(0.0) - SpotLightKey4->GetTransform()->GetPosition() + TotalTranslate);
		SpotLightKey4->SetIntensity(GetIntensity());
		SpotLightKey4->SetOuterConeAngle(FocalAngleOuter);
		SpotLightKey4->SetInnerConeAngle(FocalAngleInner);
		SpotLightKey4->SetColor(Color);
		SpotLightKey4->SetAttenuationRadius(AttenuationDistance);
		SpotLightKey4->SetSourceRadius(GetSourceRadius());
		SpotLightKey4->SetSourceLength(GetSourceLength());
		SpotLightKey4->SetSoftSourceRadius(SoftRadius);
		SpotLightKey4->SetShadowBias(ShadowBias);

		std::shared_ptr<SpotLight> SpotLightKey5 = std::shared_ptr<SpotLight>(new SpotLight());
		SpotLightKey5->GetTransform()->SetPosition(Vector3f(-112.0817f, 346.606f, 83.349f) + TotalTranslate);
		SpotLightKey5->GetTransform()->SetForward(Vector3f(0.0) - SpotLightKey5->GetTransform()->GetPosition() + TotalTranslate);
		SpotLightKey5->SetIntensity(GetIntensity());
		SpotLightKey5->SetOuterConeAngle(FocalAngleOuter);
		SpotLightKey5->SetInnerConeAngle(FocalAngleInner);
		SpotLightKey5->SetColor(Color);
		SpotLightKey5->SetAttenuationRadius(AttenuationDistance);
		SpotLightKey5->SetSourceRadius(GetSourceRadius());
		SpotLightKey5->SetSourceLength(GetSourceLength());
		SpotLightKey5->SetSoftSourceRadius(SoftRadius);
		SpotLightKey5->SetShadowBias(ShadowBias);

		std::shared_ptr<SpotLight> SpotLightKey6 = std::shared_ptr<SpotLight>(new SpotLight());
		SpotLightKey6->GetTransform()->SetPosition(Vector3f(-98.608f, 340.451f, 125.841f) + TotalTranslate);
		SpotLightKey6->GetTransform()->SetForward(Vector3f(0.0) - SpotLightKey6->GetTransform()->GetPosition() + TotalTranslate);
		SpotLightKey6->SetIntensity(GetIntensity());
		SpotLightKey6->SetOuterConeAngle(FocalAngleOuter);
		SpotLightKey6->SetInnerConeAngle(FocalAngleInner);
		SpotLightKey6->SetColor(Color);
		SpotLightKey6->SetAttenuationRadius(AttenuationDistance);
		SpotLightKey6->SetSourceRadius(GetSourceRadius());
		SpotLightKey6->SetSourceLength(GetSourceLength());
		SpotLightKey6->SetSoftSourceRadius(SoftRadius);
		SpotLightKey6->SetShadowBias(ShadowBias);

		std::shared_ptr<SpotLight> SpotLightKey7 = std::shared_ptr<SpotLight>(new SpotLight());
		SpotLightKey7->GetTransform()->SetPosition(Vector3f(-125.5555f, 352.7615f, 40.857f) + TotalTranslate);
		SpotLightKey7->GetTransform()->SetForward(Vector3f(0.0) - SpotLightKey7->GetTransform()->GetPosition() + TotalTranslate);
		SpotLightKey7->SetIntensity(GetIntensity());
		SpotLightKey7->SetOuterConeAngle(FocalAngleOuter);
		SpotLightKey7->SetInnerConeAngle(FocalAngleInner);
		SpotLightKey7->SetColor(Color);
		SpotLightKey7->SetAttenuationRadius(AttenuationDistance);
		SpotLightKey7->SetSourceRadius(GetSourceRadius());
		SpotLightKey7->SetSourceLength(GetSourceLength());
		SpotLightKey7->SetSoftSourceRadius(SoftRadius);
		SpotLightKey7->SetShadowBias(ShadowBias);

		std::shared_ptr<SpotLight> SpotLightKey8 = std::shared_ptr<SpotLight>(new SpotLight());
		SpotLightKey8->GetTransform()->SetPosition(Vector3f(-162.131f, 315.528f, 94.717f) + TotalTranslate);
		SpotLightKey8->GetTransform()->SetForward(Vector3f(0.0) - SpotLightKey8->GetTransform()->GetPosition() + TotalTranslate);
		SpotLightKey8->SetIntensity(GetIntensity());
		SpotLightKey8->SetOuterConeAngle(FocalAngleOuter);
		SpotLightKey8->SetInnerConeAngle(FocalAngleInner);
		SpotLightKey8->SetColor(Color);
		SpotLightKey8->SetAttenuationRadius(AttenuationDistance);
		SpotLightKey8->SetSourceRadius(GetSourceRadius());
		SpotLightKey8->SetSourceLength(GetSourceLength());
		SpotLightKey8->SetSoftSourceRadius(SoftRadius);
		SpotLightKey8->SetShadowBias(ShadowBias);

		Intensity = 20000.0;
		Color = Vector3f(1.0, 1.0, 1.0);
		FocalAngleOuter = 7.028856f;
		FocalAngleInner = 0.0f;
		AttenuationDistance = 700.0f;
		LightWidth = 120.0f;
		LightLength = 120.0f;
		CastShadows = true;
		LightSamplesSquared = 0;
		SourceRadiusMult = 0.809524f;
		CenterOfInterestLength = 350.0f;
		SetEnabled = true;
		SoftRadius = 13.194313f;
		ShadowBias = 0.025f;

		std::shared_ptr<SpotLight> SpotLightKey9 = std::shared_ptr<SpotLight>(new SpotLight());
		SpotLightKey9->GetTransform()->SetPosition(Vector3f(1.17633f, -341.986f, 111.556f));
		SpotLightKey9->GetTransform()->SetForward(Vector3f(0.0) - SpotLightKey9->GetTransform()->GetPosition() );
		SpotLightKey9->SetIntensity(GetIntensity());
		SpotLightKey9->SetOuterConeAngle(FocalAngleOuter);
		SpotLightKey9->SetInnerConeAngle(FocalAngleInner);
		SpotLightKey9->SetColor(Color);
		SpotLightKey9->SetAttenuationRadius(AttenuationDistance);
		SpotLightKey9->SetSourceRadius(GetSourceRadius());
		SpotLightKey9->SetSourceLength(GetSourceLength());
		SpotLightKey9->SetSoftSourceRadius(SoftRadius);
		SpotLightKey9->SetShadowBias(ShadowBias);

		Intensity = 500000.0f;
		Color = Vector3f(1.0, 1.0, 1.0);
		FocalAngleOuter = 7.0f;
		FocalAngleInner = 0.0f;
		AttenuationDistance = 700.0f;
		LightWidth = 120.0f;
		LightLength = 120.0f;
		CastShadows = true;
		LightSamplesSquared = 0;
		SourceRadiusMult = 0.809524f;
		CenterOfInterestLength = 350.0f;
		SetEnabled = true;
		SoftRadius = 13.194313f;
		ShadowBias = 0.025f;

		std::shared_ptr<SpotLight> SpotLightKey10 = std::shared_ptr<SpotLight>(new SpotLight());
		SpotLightKey10->GetTransform()->SetPosition(Vector3f(341.456f, 6.170f, 67.12f));
		SpotLightKey10->GetTransform()->SetForward(Vector3f(0.0) - SpotLightKey10->GetTransform()->GetPosition());
		SpotLightKey10->SetIntensity(GetIntensity());
		SpotLightKey10->SetOuterConeAngle(FocalAngleOuter);
		SpotLightKey10->SetInnerConeAngle(FocalAngleInner);
		SpotLightKey10->SetColor(Color);
		SpotLightKey10->SetAttenuationRadius(AttenuationDistance);
		SpotLightKey10->SetSourceRadius(GetSourceRadius());
		SpotLightKey10->SetSourceLength(GetSourceLength());
		SpotLightKey10->SetSoftSourceRadius(SoftRadius);
		SpotLightKey10->SetShadowBias(ShadowBias);

		_Scene->AddLight(SpotLightKey0);
		/*_Scene->AddLight(SpotLightKey1);
		_Scene->AddLight(SpotLightKey2);
		_Scene->AddLight(SpotLightKey3);
		_Scene->AddLight(SpotLightKey4);
		_Scene->AddLight(SpotLightKey5);
		_Scene->AddLight(SpotLightKey6);
		_Scene->AddLight(SpotLightKey7);
		_Scene->AddLight(SpotLightKey8);*/
		//_Scene->AddLight(SpotLightKey9);
		//_Scene->AddLight(SpotLightKey10);

		std::shared_ptr<PointLight> PointLight0 = std::shared_ptr<PointLight>(new PointLight());
		PointLight0->GetTransform()->SetPosition(Vector3f(40.0, 0.0, 0.0));
		PointLight0->SetColor(Vector3f(1.0, 1.0, 1.0));
		PointLight0->SetIntensity(100.0);
		PointLight0->SetAttenuationRadius(1000.0f);
		PointLight0->SetSourceRadius(0.0f);
		PointLight0->SetSoftSourceRadius(0.0f);
		PointLight0->SetSourceLength(0.0f);
		PointLight0->SetShadowBias(0.025f);
		//_Scene->AddLight(PointLight0);
	};

	~LightGroup(){};

private:
	Float32 GetSourceRadius()
	{
		Float32 SampleSquared = (Float32) max(LightSamplesSquared, 1);
		Float32 Length = min(LightLength, LightWidth);
		Length /= SampleSquared;
		if(LightSamplesSquared != 0)
		{
			Length *= 1.0f;
		} else
		{
			Length *= 0.0f;
		}
		Length /= 2.0f;
		Float32 SourceRadius1 = Length * SourceRadiusMult;
		Int32 SampleSquaredInt = Math::Clamp((Int32)LightSamplesSquared, 0, 1);
		SourceRadius1 *= SampleSquaredInt;
		SampleSquaredInt = 1 - SampleSquaredInt;
		Float32 SourceRadius2 = min(LightLength, LightWidth) * SourceRadiusMult;
		SourceRadius2 *= SampleSquaredInt;
		Float32 SourceRadius = SourceRadius1 + SourceRadius2;
		return SourceRadius;
	}

	Float32 GetIntensity()
	{
		Float32 SampleSquared = (Float32)(LightSamplesSquared + 1);
		return Intensity / Math::Pow(SampleSquared, 2.0f);
	}

	Float32 GetSourceLength()
	{
		return (max(LightLength, LightWidth) / min(LightLength, LightWidth) - 1.0f) * GetSourceRadius() * 2.0f;
	}

	Float32 Intensity;
	Vector3f Color;
	Float32 FocalAngleOuter;
	Float32 FocalAngleInner;
	Float32 AttenuationDistance;
	Float32 LightWidth;
	Float32 LightLength ;
	Bool CastShadows;
	Int32 LightSamplesSquared;
	Float32 SourceRadiusMult;
	Float32 CenterOfInterestLength;
	Bool SetEnabled;
	Float32 SoftRadius;
	Float32 ShadowBias;
};
