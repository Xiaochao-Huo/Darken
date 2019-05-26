#include "DeferRenderPipeline.h"
#include "Model.h"
#include "GlobalPram.h"
#include <glew.h>
#include <glfw3.h>

#define TAASingleTex

DeferRenderPipeline::DeferRenderPipeline()
{
}

DeferRenderPipeline::~DeferRenderPipeline()
{
}

void DeferRenderPipeline::Init()
{
	PPObj = std::shared_ptr<RectBufferObject>(new RectBufferObject());
	PPModelMatrix = Mat4f(1.0);
	PPViewMatrix = CameraUtil::LookAt(Vector3f(0.0, 0.0, 1.0), Vector3f(0.0, 0.0, 0.0), Vector3f(0.0, 1.0, 0.0));
	PPProjectMatrix = CameraUtil::Perspective(_Scene->GetCamera(CameraIndex::MainCamera)->GetFOVinRadians(), _Scene->GetCamera(CameraIndex::MainCamera)->GetAspect(), _Scene->GetCamera(CameraIndex::MainCamera)->GetNearClipPlaneDis(), _Scene->GetCamera(CameraIndex::MainCamera)->GetFarClipPlaneDis());

	SortSceneLights();

	//ShadowMapping
	ShadowMappingPass = std::shared_ptr<ShadowDepth>(new ShadowDepth(Lights));

	//Lighting
	LightingPass = std::shared_ptr<Lighting>(new Lighting(Lights));
	LightingPass->ShadowMappingPass = ShadowMappingPass;

	//ReflectionEnvironment
	ReflectionPass = std::shared_ptr<ReflectionEnvironment>(new ReflectionEnvironment());

	//SubSurfaceShading
	SSSPass = std::shared_ptr<SubSurfaceShading>(new SubSurfaceShading());
	SSSPass->SSSSetupMaterialInst->SetTextureID("PostprocessInput0", LightingPass->Lighting_Tex);
	SSSPass->SSSSetupMaterialInst->SetTextureID("PostprocessInput1", LightingPass->CustomData_Tex);
	SSSPass->SSSSetupMaterialInst->SetTextureID("PostprocessInput2", LightingPass->ScreenDepthZ_Tex);

	SSSPass->SSSScateringMaterialInst->SetTextureID("PostprocessInput1", LightingPass->CustomData_Tex);

	SSSPass->SSSRecombineMaterialInst->SetTextureID("PostprocessInput0", LightingPass->Lighting_Tex);
	SSSPass->SSSRecombineMaterialInst->SetTextureID("PostprocessInput1", SSSPass->SSSScatering_TexOut2);
	SSSPass->SSSRecombineMaterialInst->SetTextureID("PostprocessInput2", LightingPass->BaseColor_Tex);
	SSSPass->SSSRecombineMaterialInst->SetTextureID("PostprocessInput3", LightingPass->CustomData_Tex);

	//TAAPass
	TAAPass = std::shared_ptr<UE4TemporalAA>(new UE4TemporalAA());
	TAAPass->TAAPassMaterialInst->SetTextureID("DepthTex", LightingPass->ScreenDepthZ_Tex);
	TAAPass->TAAPassMaterialInst->SetTextureID("CurrentFrame", SSSPass->SSSRecombine_TexOut);
	//TAAPass->TAAPassMaterialInst->SetTextureID("CurrentFrame", LightingPass->Lighting_Tex);
	TAAPass->TAAPassMaterialInst->SetTextureID("VelocityTex", LightingPass->Velocity_Tex);
	TAAPass->TAAPassMaterialInst->SetTextureID("ShadowDepth_Tex", ShadowMappingPass->ShadowDepth_Texs2D[0]);

	//ToneMappingPass
	ToneMappingPass = std::shared_ptr<ToneMapping>(new ToneMapping());
	ToneMappingPass->GenerateLUTTexture(PPObj);
	ToneMappingPass->ToneMappingMaterialInst->SetUniform<Vector2f>("ScreenSize", Vector2f(_ScreenWidth, _ScreenHeight));
	ToneMappingPass->ToneMappingMaterialInst->SetTextureID("LUTTexture", ToneMappingPass->LUT_TEX);
	ToneMappingPass->ToneMappingMaterialInst->SetTextureID("TextureBefore", TAAPass->TAAToScreenFrame_Tex);
}

void DeferRenderPipeline::SortSceneLights()
{
	std::vector<std::shared_ptr<Light>> LightsGroup = _Scene->GetAllLights();
	for (UInt32 LightIndex = 0; LightIndex < LightsGroup.size(); LightIndex++)
	{
		Lights.push_back(LightsGroup[LightIndex].get());
	}
}

void DeferRenderPipeline::RenderShadowDepthPass(UInt32 typeFlags)
{
	ShadowMappingPass->Render(typeFlags);
}

void DeferRenderPipeline::RenderLightingPass(UInt32 typeFlags)
{
	LightingPass->Render(typeFlags);
}

void DeferRenderPipeline::RenderSSSPass()
{
	SSSPass->Render(PPObj->VAO, PPObj->NumFaces, PPObj->IndexType);
}

void DeferRenderPipeline::ExecuteTemporalAA()
{
	TAAPass->Execute(PPObj->VAO, PPObj->NumFaces, PPObj->IndexType);
}

void DeferRenderPipeline::ExecuteToneMapping()
{
	ToneMappingPass->Execute(PPObj->VAO, PPObj->NumFaces, PPObj->IndexType);
}

ShadowDepth::ShadowDepth(std::vector<Light*> lights) :
	ShadowDepthTexWidth(2048),
	ShadowDepthTexHeight(2048)
{
	Lights = lights;
	CreateShadowDepthResources();
	CalculateLightsVPMatrix();
}

ShadowDepth::~ShadowDepth()
{
	
}

void ShadowDepth::CreateShadowDepthResources()
{
	ShadowDepth_Texs2D.resize(Lights.size());
	ShadowDepth_TexsCube.resize(Lights.size());
	ShadowDepthFrameBuffers.resize(Lights.size());
	ShadowDepthRenderBuffers.resize(Lights.size());

	for (UInt32 LightIndex = 0; LightIndex < ShadowDepth_Texs2D.size(); LightIndex++)
	{
		if (Lights[LightIndex]->Type == LightType::Point)
		{
			glGenTextures(1, &ShadowDepth_TexsCube[LightIndex]);
			glBindTexture(GL_TEXTURE_CUBE_MAP, ShadowDepth_TexsCube[LightIndex]);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_R32F, ShadowDepthTexWidth, ShadowDepthTexHeight, 0, GL_RED, GL_FLOAT, 0);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_R32F, ShadowDepthTexWidth, ShadowDepthTexHeight, 0, GL_RED, GL_FLOAT, 0);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_R32F, ShadowDepthTexWidth, ShadowDepthTexHeight, 0, GL_RED, GL_FLOAT, 0);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_R32F, ShadowDepthTexWidth, ShadowDepthTexHeight, 0, GL_RED, GL_FLOAT, 0);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_R32F, ShadowDepthTexWidth, ShadowDepthTexHeight, 0, GL_RED, GL_FLOAT, 0);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_R32F, ShadowDepthTexWidth, ShadowDepthTexHeight, 0, GL_RED, GL_FLOAT, 0);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
			Float32 BlackBorder[] = { 0.0f, 0.0f, 0.0f, 0.0f };
			glTexParameterfv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BORDER_COLOR, BlackBorder);
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		}
		else
		{
			glGenTextures(1, &ShadowDepth_Texs2D[LightIndex]);
			glBindTexture(GL_TEXTURE_2D, ShadowDepth_Texs2D[LightIndex]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, ShadowDepthTexWidth, ShadowDepthTexHeight, 0, GL_RED, GL_FLOAT, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			Float32 BlackBorder[] = { 0.0f, 0.0f, 0.0f, 0.0f };
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, BlackBorder);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}
	for (UInt32 LightIndex = 0; LightIndex < ShadowDepth_Texs2D.size(); LightIndex++)
	{
		glGenFramebuffers(1, &ShadowDepthFrameBuffers[LightIndex]);
		glBindFramebuffer(GL_FRAMEBUFFER, ShadowDepthFrameBuffers[LightIndex]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ShadowDepth_Texs2D[LightIndex], 0);
		UInt32 attachments[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, attachments);
		glGenRenderbuffers(1, &ShadowDepthRenderBuffers[LightIndex]);
		glBindRenderbuffer(GL_RENDERBUFFER, ShadowDepthRenderBuffers[LightIndex]);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, ShadowDepthTexWidth, ShadowDepthTexHeight);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, ShadowDepthRenderBuffers[LightIndex]);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "ShadowMapping FrameBufferError" << std::endl;
		}

		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		/*else
		{
			glGenFramebuffers(1, &ShadowDepthFrameBuffers[LightIndex]);
			glBindFramebuffer(GL_FRAMEBUFFER, ShadowDepthFrameBuffers[LightIndex]);

			glGenRenderbuffers(1, &ShadowDepthRenderBuffers[LightIndex]);
			glBindRenderbuffer(GL_RENDERBUFFER, ShadowDepthRenderBuffers[LightIndex]);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, ShadowDepthTexWidth, ShadowDepthTexHeight);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, ShadowDepthRenderBuffers[LightIndex]);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			{
				std::cout << "ShadowMapping FrameBufferError" << std::endl;
			}

			glBindRenderbuffer(GL_RENDERBUFFER, 0);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}*/
	}
}

void ShadowDepth::CalculateLightsVPMatrix()
{
	for (UInt32 LightIndex = 0; LightIndex < Lights.size(); LightIndex++)
	{
		if (Lights[LightIndex]->Type == LightType::Direct)
		{
			DirectLight* DL = dynamic_cast<DirectLight*>(Lights[LightIndex]);
			std::shared_ptr<Camera> LCamera = std::shared_ptr<Camera>(new Camera(DL->GetTransform()->GetPosition(), DL->GetTransform()->GetEulerAngle(), 180.0f, 1.0f, 0.1f, 100.0f, Vector2i(ShadowDepthTexWidth, ShadowDepthTexHeight)));
			_Scene->AddCamera(CameraIndex::ShadowDepthCamera + LightIndex, LCamera);
		}
		else if (Lights[LightIndex]->Type == LightType::Point)
		{
			PointLight* PL = dynamic_cast<PointLight*>(Lights[LightIndex]);

			std::shared_ptr<Camera> LCamera0 = std::shared_ptr<Camera>(new Camera(PL->GetTransform()->GetPosition(), Vector3f(0.0,   0.0, 180.0), Math::Radians(90.0f), 1.0f, 2.0f, PL->GetAttenuationRadius(), Vector2i(ShadowDepthTexWidth, ShadowDepthTexHeight)));
			std::shared_ptr<Camera> LCamera1 = std::shared_ptr<Camera>(new Camera(PL->GetTransform()->GetPosition(), Vector3f(0.0,   0.0,   0.0), Math::Radians(90.0f), 1.0f, 2.0f, PL->GetAttenuationRadius(), Vector2i(ShadowDepthTexWidth, ShadowDepthTexHeight)));
			std::shared_ptr<Camera> LCamera2 = std::shared_ptr<Camera>(new Camera(PL->GetTransform()->GetPosition(), Vector3f(0.0, -90.0,   0.0), Math::Radians(90.0f), 1.0f, 2.0f, PL->GetAttenuationRadius(), Vector2i(ShadowDepthTexWidth, ShadowDepthTexHeight)));
			std::shared_ptr<Camera> LCamera3 = std::shared_ptr<Camera>(new Camera(PL->GetTransform()->GetPosition(), Vector3f(0.0,  90.0,   0.0), Math::Radians(90.0f), 1.0f, 2.0f, PL->GetAttenuationRadius(), Vector2i(ShadowDepthTexWidth, ShadowDepthTexHeight)));
			std::shared_ptr<Camera> LCamera4 = std::shared_ptr<Camera>(new Camera(PL->GetTransform()->GetPosition(), Vector3f(0.0, -90.0,  90.0), Math::Radians(90.0f), 1.0f, 2.0f, PL->GetAttenuationRadius(), Vector2i(ShadowDepthTexWidth, ShadowDepthTexHeight)));
			std::shared_ptr<Camera> LCamera5 = std::shared_ptr<Camera>(new Camera(PL->GetTransform()->GetPosition(), Vector3f(0.0,  90.0, -90.0), Math::Radians(90.0f), 1.0f, 2.0f, PL->GetAttenuationRadius(), Vector2i(ShadowDepthTexWidth, ShadowDepthTexHeight)));
			
			LCamera0->SetNextCamera(LCamera1);
			LCamera1->SetNextCamera(LCamera2);
			LCamera2->SetNextCamera(LCamera3);
			LCamera3->SetNextCamera(LCamera4);
			LCamera4->SetNextCamera(LCamera5);
			_Scene->AddCamera(CameraIndex::ShadowDepthCamera + LightIndex, LCamera0);			
		}
		else if (Lights[LightIndex]->Type == LightType::Spot)
		{
			SpotLight* SL = dynamic_cast<SpotLight*>(Lights[LightIndex]);
			std::shared_ptr<Camera> LCamera = std::shared_ptr<Camera>(new Camera(SL->GetTransform()->GetPosition(), SL->GetTransform()->GetEulerAngle(), Math::Radians(SL->GetOutConeAngle() / 2.0f), 1.0f, 2.0f, SL->GetAttenuationRadius(), Vector2i(ShadowDepthTexWidth, ShadowDepthTexHeight)));
			_Scene->AddCamera(CameraIndex::ShadowDepthCamera + LightIndex, LCamera);
		}
	}
}

void ShadowDepth::Render(unsigned typeFlags)
{
	glViewport(0, 0, ShadowDepthTexWidth, ShadowDepthTexHeight);
	const std::vector<std::shared_ptr<Object>> &Objects = _Scene->GetObjects(typeFlags);
	for (UInt32 LightIndex = 0; LightIndex < Lights.size(); LightIndex++)
	{
		switch (Lights[LightIndex]->Type)
		{
		case LightType::Direct: RenderDirectLightDepth(LightIndex, Objects); break;
		case LightType::Point: RenderPointLightDepth(LightIndex, Objects); break;
		case LightType::Spot: RenderSpotLightDepth(LightIndex, Objects); break;
		default:
			break;
		}
	}
	_Scene->GetCamera(MainCamera)->ActiveViewPort();
}

void ShadowDepth::RenderDirectLightDepth(Int32 LightIndex, const std::vector<std::shared_ptr<Object>> &Objects)
{
	std::shared_ptr<Camera> LCamera = _Scene->GetCamera(CameraIndex::ShadowDepthCamera + LightIndex);
	for (UInt32 ObjectIndex = 0; ObjectIndex < Objects.size(); ObjectIndex++)
	{
		Model* Modelptr = dynamic_cast<Model*>(Objects[ObjectIndex].get());
		ShadowDepthMaterialInst = Modelptr->GetRenderMaterial();

		ShadowDepthMaterialInst->SetUniform<Int32>(MaterialDataIDs.bDirectLightID, 1);
		ShadowDepthMaterialInst->SetUniform<Mat4f>(MaterialDataIDs.LightSpaceVPMatrixID, LCamera->GetVPMatrix());
	}
	glBindFramebuffer(GL_FRAMEBUFFER, ShadowDepthFrameBuffers[LightIndex]);
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	_Scene->Render(LCamera);
	glDisable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowDepth::RenderPointLightDepth(Int32 LightIndex, const std::vector<std::shared_ptr<Object>> &Objects)
{
	glBindFramebuffer(GL_FRAMEBUFFER, ShadowDepthFrameBuffers[LightIndex]);

	glEnable(GL_DEPTH_TEST);
	std::shared_ptr<Camera> LCamera = _Scene->GetCamera(CameraIndex::ShadowDepthCamera + LightIndex);
	for (Int32 FaceIndex = 0; FaceIndex < 6; FaceIndex++)
	{
		for (UInt32 ObjectIndex = 0; ObjectIndex < Objects.size(); ObjectIndex++)
		{
			Model* Modelptr = dynamic_cast<Model*>(Objects[ObjectIndex].get());
			ShadowDepthMaterialInst = Modelptr->GetRenderMaterial();

			ShadowDepthMaterialInst->SetUniform<Int32>(MaterialDataIDs.bDirectLightID, 0);
			ShadowDepthMaterialInst->SetUniform<Mat4f>(MaterialDataIDs.LightSpaceVPMatrixID, LCamera->GetVPMatrix());
			ShadowDepthMaterialInst->SetUniform<Vector4f>(MaterialDataIDs.LightCamera_ZBufferParamsID, Vector4f(LCamera->GetNearClipPlaneDis(), LCamera->GetFarClipPlaneDis(), LCamera->GetFOVinRadians(), LCamera->GetAspect()));
		}

		glClearColor(1.0, 1.0, 1.0, 1.0);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		_Scene->Render(LCamera);

		glBindTexture(GL_TEXTURE_CUBE_MAP, ShadowDepth_TexsCube[LightIndex]);
		glCopyTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + FaceIndex, 0, GL_R32F, 0, 0, ShadowDepthTexWidth, ShadowDepthTexHeight, 0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		LCamera = LCamera->GetNextCamera();
	}
	glDisable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowDepth::RenderSpotLightDepth(Int32 LightIndex, const std::vector<std::shared_ptr<Object>> &Objects)
{
	std::shared_ptr<Camera> LCamera = _Scene->GetCamera(CameraIndex::ShadowDepthCamera + LightIndex);
	for (UInt32 ObjectIndex = 0; ObjectIndex < Objects.size(); ObjectIndex++)
	{
		Model* Modelptr = dynamic_cast<Model*>(Objects[ObjectIndex].get());
		ShadowDepthMaterialInst = Modelptr->GetRenderMaterial();

		ShadowDepthMaterialInst->SetUniform<Int32>(MaterialDataIDs.bDirectLightID, 0);
		ShadowDepthMaterialInst->SetUniform<Mat4f>(MaterialDataIDs.LightSpaceVPMatrixID, LCamera->GetVPMatrix());
		ShadowDepthMaterialInst->SetUniform<Vector4f>(MaterialDataIDs.LightCamera_ZBufferParamsID, Vector4f(LCamera->GetNearClipPlaneDis(), LCamera->GetFarClipPlaneDis(), LCamera->GetFOVinRadians(), LCamera->GetAspect()));
	}
	glBindFramebuffer(GL_FRAMEBUFFER, ShadowDepthFrameBuffers[LightIndex]);
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	_Scene->Render(LCamera);
	glDisable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Lighting::Lighting(std::vector<Light*> lights)
{
	Lights = lights;
	MaterialDataIDs = std::shared_ptr<LightingMaterialDataIDs>(new LightingMaterialDataIDs());
	CreateLightingPassResources();
}

Lighting::~Lighting()
{
}

void Lighting::CreateLightingPassResources()
{
	glGenTextures(1, &Lighting_Tex);
	glBindTexture(GL_TEXTURE_2D, Lighting_Tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _ScreenWidth, _ScreenHeight, 0, GL_RGBA, GL_HALF_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &ScreenDepthZ_Tex);
	glBindTexture(GL_TEXTURE_2D, ScreenDepthZ_Tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, _ScreenWidth, _ScreenHeight, 0, GL_RED, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &Velocity_Tex);
	glBindTexture(GL_TEXTURE_2D, Velocity_Tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, _ScreenWidth, _ScreenHeight, 0, GL_RG, GL_HALF_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &BaseColor_Tex);
	glBindTexture(GL_TEXTURE_2D, BaseColor_Tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _ScreenWidth, _ScreenHeight, 0, GL_RGBA, GL_HALF_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &CustomData_Tex);
	glBindTexture(GL_TEXTURE_2D, CustomData_Tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _ScreenWidth, _ScreenHeight, 0, GL_RGBA, GL_HALF_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &LightingPassFrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, LightingPassFrameBuffer);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Lighting_Tex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, ScreenDepthZ_Tex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, Velocity_Tex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, BaseColor_Tex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, CustomData_Tex, 0);


	UInt32 attachments[5] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
	glDrawBuffers(5, attachments);

	glGenRenderbuffers(1, &LightingPass_RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, LightingPass_RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, _ScreenWidth, _ScreenHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, LightingPass_RBO);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "FrameBufferError" << std::endl;
	}
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//void Lighting::CreateLightingPassMaterial()
//{
//	std::shared_ptr<Material> LightingPassMaterial = std::shared_ptr<Material>(new Material(*App, std::vector<std::string> {"StanderdVertShader.vsh", "StanderdFragShader.fsh"}));
//	LightingPassMaterialInst = std::shared_ptr<MaterialInstance>(new MaterialInstance(LightingPassMaterial));
//}

void Lighting::Render(UInt32 typeFlags)
{
	static const Float32 DefalutBackGroundColor[] = { 0.0f, 0.67f, 0.7f, 1.0f };
	static const Float32 WhiteColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	static const Float32 BlackColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

	glBindFramebuffer(GL_FRAMEBUFFER, LightingPassFrameBuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearBufferfv(GL_COLOR, 0, BlackColor);
	glClearBufferfv(GL_COLOR, 1, WhiteColor);
	glClearBufferfv(GL_COLOR, 2, BlackColor);
	glClearBufferfv(GL_COLOR, 3, BlackColor);
	glClearBufferfv(GL_COLOR, 4, BlackColor);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	std::vector<std::shared_ptr<Object>> Objects = _Scene->GetObjects(typeFlags);
	if (Objects.empty()) return;
	for (UInt32 LightIndex = 0; LightIndex < Lights.size(); LightIndex++)
	{
		if (LightIndex == 0)
		{
			glDisable(GL_BLEND);
		}
		else if (LightIndex == 1)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE);
		}

		ShadowData ShadowBuffer;

		ShadowBuffer.LightSpaceVPMatrix = _Scene->GetCamera(CameraIndex::ShadowDepthCamera + LightIndex)->GetVPMatrix();// ShadowMappingPass->LightCameras[LightIndex]->VPMatrix[0];
		ShadowBuffer.ShadowBufferSize = Vector4f(ShadowMappingPass->ShadowDepthTexWidth, ShadowMappingPass->ShadowDepthTexHeight, 1.0f / ShadowMappingPass->ShadowDepthTexWidth, 1.0f / ShadowMappingPass->ShadowDepthTexHeight);

		std::shared_ptr<Camera> LCamera = _Scene->GetCamera(CameraIndex::ShadowDepthCamera + LightIndex);
		Float32 Near = LCamera->GetNearClipPlaneDis();
		Float32 Far = LCamera->GetFarClipPlaneDis();
		Float32 FOV = LCamera->GetFOVinRadians();
		Float32 Aspect = LCamera->GetAspect();
		if (ShadowMappingPass->Lights[LightIndex]->Type == LightType::Direct)
		{
			Float32 TransitionScale = 0.5f / Far * (Far / ShadowMappingPass->ShadowDepthTexWidth) * Lights[LightIndex]->GetShadowBias();
			ShadowBuffer.SoftTransitionScale = 1.0f / TransitionScale;
			Float32 LightShadowBias = 1.0f / (Far - Near) * (Far / max(ShadowMappingPass->ShadowDepthTexWidth, ShadowMappingPass->ShadowDepthTexHeight));
			ShadowBuffer.ProjectionDepthBiasParameters = Vector2f(0.5f * LightShadowBias, Far - Near);
			ShadowBuffer.LightCamera_ZBufferParams = Vector4f(Near, Far, FOV, Aspect);
			Int32 bDirectLight = 1;
		}
		else if (ShadowMappingPass->Lights[LightIndex]->Type == LightType::Point)
		{
			ShadowBuffer.SoftTransitionScale = 0.0;
			ShadowBuffer.ProjectionDepthBiasParameters = Vector2f(Lights[LightIndex]->GetShadowBias(), Far - Near);
			ShadowBuffer.LightCamera_ZBufferParams = Vector4f(Near, Far, FOV, Aspect);
			ShadowBuffer.bDirectLight = 0;
		}
		else if (ShadowMappingPass->Lights[LightIndex]->Type == LightType::Spot)
		{
			Float32 TransitionScale = 1.0f / 60.0f * 2.0f * Lights[LightIndex]->GetShadowBias();
			ShadowBuffer.SoftTransitionScale = 1.0f / TransitionScale;
			Float32 LightShadowBias = 5.0f * 512.0f / ((Far - Near) * max(ShadowMappingPass->ShadowDepthTexWidth, ShadowMappingPass->ShadowDepthTexHeight));
			ShadowBuffer.ProjectionDepthBiasParameters = Vector2f(Math::Clamp(LightShadowBias * 2.0f *Lights[LightIndex]->GetShadowBias(), 0.0f, 1.0f), Far - Near);
			ShadowBuffer.LightCamera_ZBufferParams = Vector4f(Near, Far, FOV, Aspect);
			ShadowBuffer.bDirectLight = 0;
		}
	
		_GPUBuffers->UpdateShadowBuffer(ShadowBuffer);

		LightData LightBuffer;

		ShadowMappingPass->Lights[LightIndex]->GetShaderData(LightBuffer);
		LightBuffer.bLastLight = (LightIndex == Lights.size() - 1) ? 1 : 0;
		_GPUBuffers->UpdateLightBuffer(LightBuffer);
		for (UInt32 ObjectIndex = 0; ObjectIndex < Objects.size(); ObjectIndex++)
		{
			Model* Modelptr = dynamic_cast<Model*>(Objects[ObjectIndex].get());
			LightingPassMaterialInst = Modelptr->GetRenderMaterial();
			if (Lights[LightIndex]->Type == LightType::Point)
			{
				LightingPassMaterialInst->SetTextureID(MaterialDataIDs->ShadowDepth_TexCubeID, ShadowMappingPass->ShadowDepth_TexsCube[LightIndex]);
				Camera * LCamera = _Scene->GetCamera(CameraIndex::ShadowDepthCamera + LightIndex).get();
				Mat4f PL_VPMarixes[6] = { LCamera[0].GetVPMatrix(), 
					LCamera[1].GetVPMatrix(), 
					LCamera[2].GetVPMatrix(), 
					LCamera[3].GetVPMatrix(), 
					LCamera[4].GetVPMatrix(), 
					LCamera[5].GetVPMatrix() };
				LightingPassMaterialInst->SetUniformArray<Mat4f>(MaterialDataIDs->PointLightSpaceVPMatricesID, PL_VPMarixes, 6);
			}
			else
			{
				LightingPassMaterialInst->SetTextureID(MaterialDataIDs->ShadowDepth_Tex2DID, ShadowMappingPass->ShadowDepth_Texs2D[LightIndex]);
			}
		}
		
		_Scene->Render(_Scene->GetCamera(CameraIndex::MainCamera), typeFlags);
		Int32 a = glGetError(); // if we don't use point light, and we dont use cube map shadow depth textur. But shader need compile Cube map, there is a glerror but no influence.
	}

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

ReflectionEnvironment::ReflectionEnvironment()
{
	CreateReflectionEnvResources();
}

ReflectionEnvironment::~ReflectionEnvironment()
{
}

void ReflectionEnvironment::CreateReflectionEnvResources()
{
	
}

SubSurfaceShading::SubSurfaceShading()
{
	CreateResources();
	InitSubsurfaceProfileEntries();

	std::shared_ptr<Material> SSSSetupMaterial = std::shared_ptr<Material>(new Material(std::vector<std::string> {"DrawRectVertShader.vsh", "SubsurfaceSetup.fsh"}));
	SSSSetupMaterialInst = std::shared_ptr<MaterialInstance>(new MaterialInstance(SSSSetupMaterial));

	std::shared_ptr<Material> SSSScateringMaterial = std::shared_ptr<Material>(new Material(std::vector<std::string> {"DrawRectVertShader.vsh", "SubsurfaceScatering.fsh"}));
	SSSScateringMaterialInst = std::shared_ptr<MaterialInstance>(new MaterialInstance(SSSScateringMaterial));

	std::shared_ptr<Material> SSSRecombineMaterial = std::shared_ptr<Material>(new Material(std::vector<std::string> {"DrawRectVertShader.vsh", "SubsurfaceRecombine.fsh"}));
	SSSRecombineMaterialInst = std::shared_ptr<MaterialInstance>(new MaterialInstance(SSSRecombineMaterial));

	std::shared_ptr<Camera> ViewCamera = _Scene->GetCamera(CameraIndex::MainCamera);
	SSSSetupMaterialInst->SetUniform<Vector2f>("CameraNearFar", Vector2f(ViewCamera->GetNearClipPlaneDis(), ViewCamera->GetFarClipPlaneDis()));

	Float32 distanceToProjectionWindow = 1.0f / tan(0.5f * ViewCamera->GetFOVinRadians() /*have been radiance*/);
	Float32 SSSScaleZ = distanceToProjectionWindow;
	// * 0.5f: hacked in 0.5 - -1..1 to 0..1 but why this isn't in demo code?
	Float32 SSSScaleX = SSSScaleZ / (Float32)(SUBSURFACE_KERNEL_SIZE) * 0.5f;
	Vector2f SSSParams = Vector2f(SSSScaleX, SSSScaleZ);
	Vector4f ViewSizeAndInvSize = Vector4f(_ScreenWidth, _ScreenHeight, 1.0 / _ScreenWidth, 1.0 / _ScreenHeight);
	//FSubsurfaceProfileStruct Data = SubsurfaceProfileEntries[QualityIndex];
	//InitSSSSProfilekernel(QualityIndex);

	SSSScateringMaterialInst->SetUniform<Vector2f>("SSSParams", SSSParams);
	InitSSSSProfilekernelParams("_SKIN", skin_profile_index);
	InitSSSSProfilekernelParams("_EYEBLEND", eye_blend_index);
	InitSSSSProfilekernelParams("_EYEREFRACTIVE", eye_refractive_index);
	//SSSScateringMaterialInst->SetUniformArray<Vector4f>("SSSSMirroredProfilekernel", &SSSSMirroredProfilekernel[0], SSSSMirroredProfilekernel.size());
	//SSSRecombineMaterialInst->SetUniform<Vector4f>("SubsurfaceColor", Data.SubsurfaceColor);
}
void SubSurfaceShading::InitSSSSProfilekernelParams(std::string const & _prefix, Int32 index)
{
	InitSSSSProfilekernel(index);
	FSubsurfaceProfileStruct Data = SubsurfaceProfileEntries[index];
	std::string kenelname = "SSSSMirroredProfilekernel" + _prefix;
	std::string colorname = "SubsurfaceColor" + _prefix;
	SSSScateringMaterialInst->SetUniformArray(kenelname, &SSSSMirroredProfilekernelMap[index][0], (Int32)SSSSMirroredProfilekernelMap[index].size());
	SSSRecombineMaterialInst->SetUniform(colorname, Data.SubsurfaceColor);
}
SubSurfaceShading::~SubSurfaceShading()
{

}

void SubSurfaceShading::CreateResources()
{
	glGenTextures(1, &SSSRender_Tex);
	glBindTexture(GL_TEXTURE_2D, SSSRender_Tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _ScreenWidth, _ScreenHeight, 0, GL_RGBA, GL_HALF_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &SSSSetup_TexOut);
	glBindTexture(GL_TEXTURE_2D, SSSSetup_TexOut);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _ScreenWidth, _ScreenHeight, 0, GL_RGBA, GL_HALF_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &SSSScatering_TexOut1);
	glBindTexture(GL_TEXTURE_2D, SSSScatering_TexOut1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _ScreenWidth, _ScreenHeight, 0, GL_RGBA, GL_HALF_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &SSSScatering_TexOut2);
	glBindTexture(GL_TEXTURE_2D, SSSScatering_TexOut2);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _ScreenWidth, _ScreenHeight, 0, GL_RGBA, GL_HALF_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &SSSRecombine_TexOut);
	glBindTexture(GL_TEXTURE_2D, SSSRecombine_TexOut);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _ScreenWidth, _ScreenHeight, 0, GL_RGBA, GL_HALF_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
	

	glGenFramebuffers(1, &SSSFrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, SSSFrameBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, SSSRender_Tex, 0);
	UInt32 attachments[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, attachments);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "SSS FrameBufferError" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Vector4f SubSurfaceShading::GetSSSS_DUAL_SPECULAR_Params(Int32 index)
{
	Vector4f res;
	FSubsurfaceProfileStruct Data = SubsurfaceProfileEntries[index];;
	Float32 MaterialRoughnessToAverage = Data.Roughness0 * (1.0f - Data.LobeMix) + Data.Roughness1 * Data.LobeMix;
	Float32 AverageToRoughness0 = Data.Roughness0 / MaterialRoughnessToAverage;
	Float32 AverageToRoughness1 = Data.Roughness1 / MaterialRoughnessToAverage;
	res.r = Math::Clamp(AverageToRoughness0 / SSSS_MAX_DUAL_SPECULAR_ROUGHNESS, 0.0f, 1.0f);
	res.g = Math::Clamp(AverageToRoughness1 / SSSS_MAX_DUAL_SPECULAR_ROUGHNESS, 0.0f, 1.0f);
	res.b = Data.LobeMix;
	res.a = Math::Clamp(MaterialRoughnessToAverage / SSSS_MAX_DUAL_SPECULAR_ROUGHNESS, 0.0f, 1.0f);
	return res;
}
Vector4f SubSurfaceShading::GetSSSS_TRANSMISSION_Params(Int32 index)
{
	Vector4f res;
	FSubsurfaceProfileStruct Data = SubsurfaceProfileEntries[index];
	res.r = Data.ExtinctionScale;
	res.g = Data.NormalScale;
	res.b = Data.ScatteringDistribution;
	res.a = 1.0f / Data.IOR;
	return res;
}

void SubSurfaceShading::InitSubsurfaceProfileEntries()
{
	Int32 index = 0;
	SubsurfaceProfileEntries[index].ScatterRadius = 1.2f;
	SubsurfaceProfileEntries[index].SubsurfaceColor = Vector4f(0.48f, 0.41f, 0.28f, 1.0);
	SubsurfaceProfileEntries[index].FalloffColor = Vector4f(1.0f, 0.37f, 0.3f, 1.0);
	SubsurfaceProfileEntries[index].BoundaryColorBleed = Vector4f(1.0f);;
	SubsurfaceProfileEntries[index].ExtinctionScale = 1.0f;
	SubsurfaceProfileEntries[index].NormalScale = 0.08f;
	SubsurfaceProfileEntries[index].ScatteringDistribution = 0.93f;
	SubsurfaceProfileEntries[index].IOR = 1.55f;
	SubsurfaceProfileEntries[index].Roughness0 = 0.75f;
	SubsurfaceProfileEntries[index].Roughness1 = 1.30f;
	SubsurfaceProfileEntries[index].LobeMix = 0.85f;

	++index;
	SubsurfaceProfileEntries[index].ScatterRadius = 0.8f;
	SubsurfaceProfileEntries[index].SubsurfaceColor = Vector4f(0.8f, 0.781781f, 0.748775f, 1.0);
	SubsurfaceProfileEntries[index].FalloffColor = Vector4f(0.735f, 0.308307f, 0.301288f, 1.0);
	SubsurfaceProfileEntries[index].BoundaryColorBleed = Vector4f(0.735f, 0.527861f, 0.524452984f, 1.0);;
	SubsurfaceProfileEntries[index].ExtinctionScale = 1.0f;
	SubsurfaceProfileEntries[index].NormalScale = 0.08f;
	SubsurfaceProfileEntries[index].ScatteringDistribution = 0.93f;
	SubsurfaceProfileEntries[index].IOR = 1.55f;
	SubsurfaceProfileEntries[index].Roughness0 = 0.75f;
	SubsurfaceProfileEntries[index].Roughness1 = 1.30f;
	SubsurfaceProfileEntries[index].LobeMix = 0.85f;
	++index;
	SubsurfaceProfileEntries[index].ScatterRadius = 1.5f;
	SubsurfaceProfileEntries[index].SubsurfaceColor = Vector4f(1.0);
	SubsurfaceProfileEntries[index].FalloffColor = Vector4f(1.0);
	SubsurfaceProfileEntries[index].BoundaryColorBleed = Vector4f(0.416667f, 0.107603f, 0.0f, 1.0);;
	SubsurfaceProfileEntries[index].ExtinctionScale = 1.0f;
	SubsurfaceProfileEntries[index].NormalScale = 0.364666998f;
	SubsurfaceProfileEntries[index].ScatteringDistribution = 0.93f;
	SubsurfaceProfileEntries[index].IOR = 1.55f;
	SubsurfaceProfileEntries[index].Roughness0 = 0.75f;
	SubsurfaceProfileEntries[index].Roughness1 = 1.30f;
	SubsurfaceProfileEntries[index].LobeMix = 0.85f;
	++index;
	SubsurfaceProfileEntries[index].ScatterRadius = 0.95f;
	SubsurfaceProfileEntries[index].SubsurfaceColor = Vector4f(0.8f, 0.781781f, 0.748775f, 1.0);Vector4f(1.0);
	SubsurfaceProfileEntries[index].FalloffColor = Vector4f(0.716666958f, 0.277326989, 0.286445993, 1.0);
	SubsurfaceProfileEntries[index].BoundaryColorBleed = Vector4f(0.802083015, 0.681032002, 0.584851980, 1.0);;
	SubsurfaceProfileEntries[index].ExtinctionScale = 0.15f;
	SubsurfaceProfileEntries[index].NormalScale = 0.25;
	SubsurfaceProfileEntries[index].ScatteringDistribution = 0.93f;
	SubsurfaceProfileEntries[index].IOR = 1.47f;
	SubsurfaceProfileEntries[index].Roughness0 = 0.75f;
	SubsurfaceProfileEntries[index].Roughness1 = 1.30f;
	SubsurfaceProfileEntries[index].LobeMix = 0.85f;
	++index;
	SubsurfaceProfileEntries[index].ScatterRadius = 0.5f;
	SubsurfaceProfileEntries[index].SubsurfaceColor = Vector4f(1.0);
	SubsurfaceProfileEntries[index].FalloffColor = Vector4f(1.0f, 0.446346998, 0.486173004, 1.0);
	SubsurfaceProfileEntries[index].BoundaryColorBleed = Vector4f(1.0);;
	SubsurfaceProfileEntries[index].ExtinctionScale = 1.0f;
	SubsurfaceProfileEntries[index].NormalScale = 0.99f;
	SubsurfaceProfileEntries[index].ScatteringDistribution = 0.93f;
	SubsurfaceProfileEntries[index].IOR = 1.47f;
	SubsurfaceProfileEntries[index].Roughness0 = 0.75f;
	SubsurfaceProfileEntries[index].Roughness1 = 1.30f;
	SubsurfaceProfileEntries[index].LobeMix = 0.85f;
}

void SubSurfaceShading::InitSSSSProfilekernel(Int32 index)
{
	auto&  SSSSMirroredProfilekernel = SSSSMirroredProfilekernelMap[index];
	SSSSMirroredProfilekernel.clear();

	SSSSMirroredProfilekernel.resize(SSSS_KERNEL_TOTAL_SIZE_WITH_PADDING);// = new Vector4f[SSSS_KERNEL_TOTAL_SIZE_WITH_PADDING];
	FSubsurfaceProfileStruct Data = SubsurfaceProfileEntries[index];
	Int32 nTotalSamples = SSSS_KERNEL0_OFFSET * 2 - 1;
	//SSSSProfilekernel.clear();
	//SSSSProfilekernel.resize(SSSS_KERNEL_TOTAL_SIZE_WITH_PADDING);
	//ComputeSSSKernel(nTotalSamples,Data.SubsurfaceColor, Data.FalloffColor);
	//ComputeMirroredSSSKernel(SSSSMirroredProfilekernel, 13, Data.SubsurfaceColor, Data.FalloffColor);
	ComputeMirroredSSSKernel(index, SSSS_KERNEL0_OFFSET, SSSS_KERNEL0_SIZE, Data.SubsurfaceColor, Data.FalloffColor);
	ComputeMirroredSSSKernel(index, SSSS_KERNEL1_OFFSET, SSSS_KERNEL1_SIZE, Data.SubsurfaceColor, Data.FalloffColor);
	ComputeMirroredSSSKernel(index, SSSS_KERNEL2_OFFSET, SSSS_KERNEL2_SIZE, Data.SubsurfaceColor, Data.FalloffColor);

	const Float32 TableMaxRGB = 1.0f;
	const Float32 TableMaxA = 3.0f;
	const Vector4f TableColorScale = Vector4f(
		1.0f / TableMaxRGB,
		1.0f / TableMaxRGB,
		1.0f / TableMaxRGB,
		1.0f / TableMaxA);
	for (Int32 pos = 0; pos < SSSS_KERNEL_TOTAL_SIZE; ++pos)
	{
		//SSSSMirroredProfilekernel[pos] *= TableColorScale;
		// requires 16bit (could be made with 8 bit e.g. using sample0.w as 8bit scale applied to all samples (more multiplications in the shader))
		SSSSMirroredProfilekernel[pos].w *= (Data.ScatterRadius /*/ Float32(SUBSURFACE_RADIUS_SCALE)*/);
	}
	SSSSMirroredProfilekernel[SSSS_KERNEL_TOTAL_SIZE] = Vector4f(0.0, 1.0, 0.0, 1.0);
}

void SubSurfaceShading::ComputeMirroredSSSKernel(Int32 index, UInt32 startPos, UInt32 TargetBufferSize, Vector4f SubsurfaceColor, Vector4f FalloffColor)
{
	UInt32 nNonMirroredSamples = TargetBufferSize;
	Int32 nTotalSamples = nNonMirroredSamples * 2 - 1;
	Vector3f sssStrength = Vector3f(0.78, 0.43, 0.43);
	// we could generate Out directly but the original code form SeparableSSS wasn't done like that so we convert it later
	// .a is in mm
	//check(nTotalSamples < 64);
	Vector4f kernel[64];
	{
		const Float32 Range = nTotalSamples > 20 ? 3.0f : 2.0f;
		// tweak constant
		const Float32 Exponent = 2.0f;

		// Calculate the offsets:
		Float32 step = 2.0f * Range / (nTotalSamples - 1);
		for (Int32 i = 0; i < nTotalSamples; i++)
		{
			Float32 o = -Range + Float32(i) * step;
			Float32 sign = o < 0.0f ? -1.0f : 1.0f;
			kernel[i].a = Range * sign * abs(pow(o, Exponent)) / pow(Range, Exponent);
		}

		// Calculate the weights:
		for (Int32 i = 0; i < nTotalSamples; i++)
		{
			Float32 w0 = i > 0 ? abs(kernel[i].a - kernel[i - 1].a) : 0.0f;
			Float32 w1 = i < nTotalSamples - 1 ? abs(kernel[i].a - kernel[i + 1].a) : 0.0f;
			Float32 area = (w0 + w1) / 2.0f;
			Vector3f t = area * SeparableSSS_Profile(kernel[i].a, FalloffColor);
			kernel[i].r = t.x;
			kernel[i].g = t.y;
			kernel[i].b = t.z;
		}

		// We want the offset 0.0 to come first:
		Vector4f t = kernel[nTotalSamples / 2];

		for (Int32 i = nTotalSamples / 2; i > 0; i--)
		{
			kernel[i] = kernel[i - 1];
		}
		kernel[0] = t;

		// Normalize the weights in RGB
		{
			Vector3f sum = Vector3f(0, 0, 0);

			for (Int32 i = 0; i < nTotalSamples; i++)
			{
				sum.x += kernel[i].r;
				sum.y += kernel[i].g;
				sum.z += kernel[i].b;
			}

			for (Int32 i = 0; i < nTotalSamples; i++)
			{
				kernel[i].r /= sum.x;
				kernel[i].g /= sum.y;
				kernel[i].b /= sum.z;
			}
		}

		/* we do that in the shader for better quality with half res*/

		// Tweak them using the desired strength. The first one is:
		//     lerp(1.0, kernel[0].rgb, strength)
		/*kernel[0].r = (1.0f - sssStrength.x) * 1.0f + sssStrength.x * kernel[0].x;
		kernel[0].g = (1.0f - sssStrength.y) * 1.0f + sssStrength.y * kernel[0].y;
		kernel[0].b = (1.0f - sssStrength.z) * 1.0f + sssStrength.z * kernel[0].z;

		for (Int32 i = 1; i < nTotalSamples; i++)
		{
			kernel[i].r *= sssStrength.r;
			kernel[i].g *= sssStrength.g;
			kernel[i].b *= sssStrength.b;
		}*/
	}

	// generate output (remove negative samples)
	{
		//check(kernel[0].a == 0.0f);

		// center sample
		SSSSMirroredProfilekernelMap[index][startPos + 0] = kernel[0];

		// all positive samples
		for (UInt32 i = 0; i < nNonMirroredSamples - 1; i++)
		{
			SSSSMirroredProfilekernelMap[index][startPos + i + 1] = kernel[nNonMirroredSamples + i];
		}
	}
}

Vector3f SubSurfaceShading::SeparableSSS_Gaussian(Float32 variance, Float32 r, Vector4f FalloffColor)
{
	Vector3f Ret;

	/**
	* We use a falloff to modulate the shape of the profile. Big falloffs
	* spreads the shape making it wider, while small falloffs make it
	* narrower.
	*/
	for (Int32 i = 0; i < 3; i++)
	{
		Float32 rr = r / (0.001f + FalloffColor[i]);
		Ret[i] = exp((-(rr * rr)) / (2.0f * variance)) / (2.0f * 3.14f * variance);
	}

	return Ret;
}
Vector3f SubSurfaceShading::SeparableSSS_Profile(Float32 r, Vector4f FalloffColor)
{
	/**
	* We used the red channel of the original skin profile defined in
	* [d'Eon07] for all three channels. We noticed it can be used for green
	* and blue channels (scaled using the falloff parameter) without
	* introducing noticeable differences and allowing for total control over
	* the profile. For example, it allows to create blue SSS gradients, which
	* could be useful in case of rendering blue creatures.
	*/
	// first parameter is variance in mm^2
	return  // 0.233f * SeparableSSS_Gaussian(0.0064f, r, FalloffColor) + /* We consider this one to be directly bounced light, accounted by the strength parameter (see @STRENGTH) */
		0.100f * SeparableSSS_Gaussian(0.0484f, r, FalloffColor) +
		0.118f * SeparableSSS_Gaussian(0.187f, r, FalloffColor) +
		0.113f * SeparableSSS_Gaussian(0.567f, r, FalloffColor) +
		0.358f * SeparableSSS_Gaussian(1.99f, r, FalloffColor) +
		0.078f * SeparableSSS_Gaussian(7.41f, r, FalloffColor);
}

void SubSurfaceShading::ComputeTransmissionProfile(Vector4f* TargetBuffer, UInt32 TargetBufferSize, Vector4f SubsurfaceColor, Vector4f FalloffColor, Float32 ExtinctionScale)
{

	static Float32 MaxTransmissionProfileDistance = 5.0f; // See MAX_TRANSMISSION_PROFILE_DISTANCE in TransmissionCommon.ush

	for (UInt32 i = 0; i < TargetBufferSize; ++i)
	{
		//10 mm
		const Float32 InvSize = 1.0f / TargetBufferSize;
		Float32 Distance = i * InvSize * MaxTransmissionProfileDistance;
		Vector3f TransmissionProfile = SeparableSSS_Profile(Distance, FalloffColor);
		TargetBuffer[i] = Vector4f(TransmissionProfile, 1.0);
		//Use Luminance of scattering as SSSS shadow.
		TargetBuffer[i].a = exp(-Distance * ExtinctionScale);
	}

	// Do this is because 5mm is not enough cool down the scattering to zero, although which is small number but after tone mapping still noticeable
	// so just Let last pixel be 0 which make sure thickness great than MaxRadius have no scattering
	static Bool bMakeLastPixelBlack = true;
	if (bMakeLastPixelBlack)
	{
		TargetBuffer[TargetBufferSize - 1] = Vector4f(0.0);
	}
}
void SubSurfaceShading::ComputeTransmissionProfile(Int32 index, Vector4f* TargetBuffer, UInt32 TargetBufferSize)
{
	FSubsurfaceProfileStruct Data = SubsurfaceProfileEntries[index];

	ComputeTransmissionProfile(TargetBuffer, TargetBufferSize, Data.SubsurfaceColor, Data.FalloffColor, Data.ExtinctionScale);
}

void SubSurfaceShading::Render(UInt32 VAO, Int32 NumFaces, IndexSizeType indexType)
{
	glBindFramebuffer(GL_FRAMEBUFFER, SSSFrameBuffer);
	glClear(GL_COLOR_BUFFER_BIT);
	SSSSetupMaterialInst->GetParent()->Draw(VAO, NumFaces, indexType, OGL_ELEMENT);
	glBindTexture(GL_TEXTURE_2D, SSSSetup_TexOut);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 0, 0, _ScreenWidth, _ScreenHeight, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glClear(GL_COLOR_BUFFER_BIT);
	SSSScateringMaterialInst->SetUniform<Int32>("SSS_DIRECTION", 0);
	SSSScateringMaterialInst->SetTextureID("PostprocessInput0", SSSSetup_TexOut);
	SSSScateringMaterialInst->GetParent()->Draw(VAO, NumFaces, indexType, OGL_ELEMENT);
	glBindTexture(GL_TEXTURE_2D, SSSScatering_TexOut1);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 0, 0, _ScreenWidth, _ScreenHeight, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glClear(GL_COLOR_BUFFER_BIT);
	SSSScateringMaterialInst->SetUniform<Int32>("SSS_DIRECTION", 1);
	SSSScateringMaterialInst->SetTextureID("PostprocessInput0", SSSScatering_TexOut1);
	SSSScateringMaterialInst->GetParent()->Draw(VAO, NumFaces, indexType, OGL_ELEMENT);
	glBindTexture(GL_TEXTURE_2D, SSSScatering_TexOut2);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 0, 0, _ScreenWidth, _ScreenHeight, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glClear(GL_COLOR_BUFFER_BIT);
	SSSRecombineMaterialInst->GetParent()->Draw(VAO, NumFaces, indexType, OGL_ELEMENT);
	glBindTexture(GL_TEXTURE_2D, SSSRecombine_TexOut);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 0, 0, _ScreenWidth, _ScreenHeight, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

UE4TemporalAA::UE4TemporalAA()
{
	CreateTAAPassMaterial();
	CreateTAAPassResources();
	TAAPassMaterialInst->SetTextureID("PreFrame", TAAHistoryFrame_Tex);
	TAAPassMaterialInst->SetUniform<Vector2f>("ScreenSize", Vector2f(_ScreenWidth, _ScreenHeight));
	TAAPassMaterialInst->SetUniform<Vector4f>("ViewCamera_ZBufferParams", Vector4f(_Scene->GetCamera(CameraIndex::MainCamera)->GetNearClipPlaneDis(), _Scene->GetCamera(CameraIndex::MainCamera)->GetFarClipPlaneDis(), _Scene->GetCamera(CameraIndex::MainCamera)->GetFOVinRadians(), _Scene->GetCamera(CameraIndex::MainCamera)->GetAspect()));
	TAAPixelUniformData = std::shared_ptr<TemporalAAPixelUniformData>(new TemporalAAPixelUniformData());

	InitUE4SampleType4();
	//InitUE4SampleType2();
}

UE4TemporalAA::~UE4TemporalAA()
{
}

void UE4TemporalAA::Execute(UInt32 VAO, Int32 NumFaces, IndexSizeType indexType)
{
	Vector4f JitterUV = ActiveJitterSample;
	TAAPassMaterialInst->SetUniform<Vector4f>(TAAPixelUniformData->JitterID, Vector4f(JitterUV.x / _ScreenWidth, JitterUV.y / _ScreenHeight, JitterUV.z / _ScreenWidth, JitterUV.w / _ScreenHeight));

	//Update Weight Plus
	{
		static const Float32 SampleOffsets[9][2] =
		{
			{ -1.0f, -1.0f },
			{ 0.0f, -1.0f },
			{ 1.0f, -1.0f },
			{ -1.0f, 0.0f },
			{ 0.0f, 0.0f },
			{ 1.0f, 0.0f },
			{ -1.0f, 1.0f },
			{ 0.0f, 1.0f },
			{ 1.0f, 1.0f },
		};

		Float32 Weights[9];
		Float32 WeightsPlus[5];
		Float32 TotalWeightPlus;

		for (Int32 i = 0; i < 9; i++)
		{
			Float32 PixelOffsetX = SampleOffsets[i][0] - JitterUV.x;
			Float32 PixelOffsetY = SampleOffsets[i][1] - JitterUV.y;

			// Normal distribution, Sigma = 0.47
			Weights[i] = Math::Exp(-2.29f * (PixelOffsetX * PixelOffsetX + PixelOffsetY * PixelOffsetY));
		}

		WeightsPlus[0] = Weights[1];
		WeightsPlus[1] = Weights[3];
		WeightsPlus[2] = Weights[4];
		WeightsPlus[3] = Weights[5];
		WeightsPlus[4] = Weights[7];
		TotalWeightPlus = Weights[1] + Weights[3] + Weights[4] + Weights[5] + Weights[7];

		TAAPassMaterialInst->SetUniform<Vector4f>(TAAPixelUniformData->PlusWeights_0ID, Vector4f(WeightsPlus[0], WeightsPlus[1], WeightsPlus[2], WeightsPlus[3]) / TotalWeightPlus);
		TAAPassMaterialInst->SetUniform<Vector4f>(TAAPixelUniformData->PlusWeights_1ID, Vector4f(WeightsPlus[5], 0.0, 0.0, 0.0) / TotalWeightPlus);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, TAAFrameBuffer);
	glClear(GL_COLOR_BUFFER_BIT);

	TAAPassMaterialInst->GetParent()->Draw(VAO, NumFaces, indexType);

#ifdef TAASingleTex
	glCopyImageSubData(TAAToScreenFrame_Tex, GL_TEXTURE_2D, 0, 0, 0, 0, TAAHistoryFrame_Tex, GL_TEXTURE_2D, 0, 0, 0, 0, _ScreenWidth, _ScreenHeight, 1);
#endif // TAASingleTex



	//glBindTexture(GL_TEXTURE_2D, TAAToScreenFrame_Tex);
	//glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 0, 0, _ScreenWidth, _ScreenHeight, 0);
	//glBindTexture(GL_TEXTURE_2D, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void UE4TemporalAA::CreateTAAPassMaterial()
{
	std::shared_ptr<Material> TAAMaterial = std::shared_ptr<Material>(new Material(std::vector<std::string> {"DrawRectVertShader.vsh", "UE4TemporalAAFragShader.fsh"}));
	TAAPassMaterialInst = std::shared_ptr<MaterialInstance>(new MaterialInstance(TAAMaterial));
	TAAPassMaterialInst->SetBlockUniform<Vector2f>("TestBlock", "Color2", Vector4f(1.0, 1.0, 0.0, 1.0));
}

void UE4TemporalAA::InitUE4SampleType4()
{
	FrustumJitter.resize(4);
	FrustumJitter[0] = Vector2f(-0.125f, -0.375f);	//ll  3 - 2
	FrustumJitter[1] = Vector2f(0.375f, -0.125f);	//ur      |
	FrustumJitter[2] = Vector2f(0.125f, 0.375f);	//lr	  |
	FrustumJitter[3] = Vector2f(-0.375f, 0.125f);	//ul  0 - 1
}

void UE4TemporalAA::InitUE4SampleType2()
{
	FrustumJitter.resize(2);					//    1
	FrustumJitter[0] = Vector2f(-0.5f, 0.0f);	//	 /
	FrustumJitter[1] = Vector2f(0.0f, 0.5f);   //  0
}

void UE4TemporalAA::UpdateJitter()
{
	ActiveJitterSample.z = ActiveJitterSample.x;
	ActiveJitterSample.w = ActiveJitterSample.y;

	ActiveJitterIndex += 1;
	ActiveJitterIndex %= FrustumJitter.size();

	ActiveJitterSample.x = FrustumJitter[ActiveJitterIndex].x;
	ActiveJitterSample.y = FrustumJitter[ActiveJitterIndex].y;

	ActiveJitterSample *= 1.0f;

	HackUpdateCameraProjectMatrix(ActiveJitterSample.x, ActiveJitterSample.y);
}

void UE4TemporalAA::HackUpdateCameraProjectMatrix(Float32 sampleOffsetX, Float32 sampleOffsetY)
{
	JitterProjectMatrix = _Scene->GetCamera(CameraIndex::MainCamera)->GetProjectMatrix();
	JitterProjectMatrix[2][0] += sampleOffsetX * (2.0f) / _ScreenWidth;
	JitterProjectMatrix[2][1] += sampleOffsetY * (-2.0f) / _ScreenHeight;
	_Scene->GetCamera(CameraIndex::MainCamera)->SetProjectMatrix(JitterProjectMatrix);
}

void UE4TemporalAA::RemoveJitter()
{
	HackRemoveCameraProjectMatrix(ActiveJitterSample.x, ActiveJitterSample.y);
}

void UE4TemporalAA::HackRemoveCameraProjectMatrix(Float32 sampleOffsetX, Float32 sampleOffsetY)
{
	JitterProjectMatrix = _Scene->GetCamera(CameraIndex::MainCamera)->GetProjectMatrix();
	JitterProjectMatrix[2][0] -= sampleOffsetX * (2.0f) / _ScreenWidth;
	JitterProjectMatrix[2][1] -= sampleOffsetY * (-2.0f) / _ScreenHeight;
	_Scene->GetCamera(CameraIndex::MainCamera)->SetProjectMatrix(JitterProjectMatrix);
}

void UE4TemporalAA::UpdateCameraProjectMatrix(Float32 sampleOffsetX, Float32 sampleOffsetY)
{
	Float32 oneExtentY = Math::Tan(0.5f * _Scene->GetCamera(CameraIndex::MainCamera)->GetFOVinRadians());
	Float32 oneExtentX = oneExtentY * _Scene->GetCamera(CameraIndex::MainCamera)->GetAspect();
	Float32 texelSizeX = oneExtentX / (0.5f * _ScreenWidth);
	Float32 texelSizeY = oneExtentY / (0.5f * _ScreenHeight);
	Float32 oneJitterX = texelSizeX * sampleOffsetX;
	Float32 oneJitterY = texelSizeY * sampleOffsetY;

	Vector4f extents = Vector4f(oneExtentX, oneExtentY, oneJitterX, oneJitterY);// xy = frustum extents at distance 1, zw = jitter at distance 1

	Float32 cf = _Scene->GetCamera(CameraIndex::MainCamera)->GetFarClipPlaneDis();
	Float32 cn = _Scene->GetCamera(CameraIndex::MainCamera)->GetNearClipPlaneDis();
	Float32 xm = extents.z - extents.x;
	Float32 xp = extents.z + extents.x;
	Float32 ym = extents.w - extents.y;
	Float32 yp = extents.w + extents.y;

	JitterProjectMatrix = CameraUtil::Frustum(xm * cn, xp * cn, ym * cn, yp * cn, cn, cf);

	JitterProjectMatrix = _Scene->GetCamera(CameraIndex::MainCamera)->GetProjectMatrix();
	JitterProjectMatrix[2][0] += sampleOffsetX * (2.0f) / _ScreenWidth;
	JitterProjectMatrix[2][1] += sampleOffsetX * (-2.0f) / _ScreenHeight;
	_Scene->GetCamera(CameraIndex::MainCamera)->SetProjectMatrix(JitterProjectMatrix);
}

void UE4TemporalAA::CreateTAAPassResources()
{
	glGenTextures(1, &TAAHistoryFrame_Tex);
	glBindTexture(GL_TEXTURE_2D, TAAHistoryFrame_Tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _ScreenWidth, _ScreenHeight, 0, GL_RGBA, GL_HALF_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &TAAToScreenFrame_Tex);
	glBindTexture(GL_TEXTURE_2D, TAAToScreenFrame_Tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _ScreenWidth, _ScreenHeight, 0, GL_RGBA, GL_HALF_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

#ifdef TAASingleTex
	glGenFramebuffers(1, &TAAFrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, TAAFrameBuffer);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, TAAHistoryFrame_Tex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, TAAToScreenFrame_Tex, 0);
	UInt32 attachments[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, attachments);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "TAA FrameBufferError" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

#else
	glGenFramebuffers(1, &TAAFrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, TAAFrameBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, TAAHistoryFrame_Tex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, TAAToScreenFrame_Tex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, DebugTex, 0);
	UInt32 attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "TAA FrameBufferError" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


#endif // TAASingleTex
}


ToneMapping::ToneMapping()
{
	ToneMappingUniformDatas = std::shared_ptr<ToneMappingPixelShaderUniformData>(new ToneMappingPixelShaderUniformData());
	CreateToneMappingPassMaterial();
}

ToneMapping::~ToneMapping()
{
}

void ToneMapping::CreateToneMappingPassMaterial()
{
	std::shared_ptr<Material> ToneMappingMaterial = std::shared_ptr<Material>(new Material(std::vector<std::string> {"ToneMappingVertShader.vsh", "ToneMappingFragShader.fsh"}));
	ToneMappingMaterialInst = std::shared_ptr<MaterialInstance>(new MaterialInstance(ToneMappingMaterial));
}

void ToneMapping::GenerateLUTTexture(std::shared_ptr<RectBufferObject> pPObj)
{
	//Prepare LUT render Material
	std::shared_ptr<Material> LUTMaterial = std::shared_ptr<Material>(new Material(std::vector<std::string> {"LUTVertShader.vsh", "LUTFragShader.fsh"}));
	std::shared_ptr<MaterialInstance> LUTMaterialInst = std::shared_ptr<MaterialInstance>(new MaterialInstance(LUTMaterial));

	//Prepare LUT Texture
	static const Int32 LUT_TEX_Size = 32;
	glGenTextures(1, &LUT_TEX);
	glBindTexture(GL_TEXTURE_3D, LUT_TEX);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB10_A2, LUT_TEX_Size, LUT_TEX_Size, LUT_TEX_Size, 0, GL_RGBA, GL_UNSIGNED_INT_2_10_10_10_REV, 0);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_3D, 0);

	//No Geometry Shader(GL_ES 3.0 EXT or GL_ES 3.1) support, render 32 times to create Texture
	UInt32 LUT_FrameBuffer[LUT_TEX_Size];
	glGenFramebuffers(LUT_TEX_Size, LUT_FrameBuffer);

	for (Int32 FrameBufferIndex = 0; FrameBufferIndex < LUT_TEX_Size; FrameBufferIndex++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, LUT_FrameBuffer[FrameBufferIndex]);
		glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, LUT_TEX, 0, FrameBufferIndex);
		UInt32 attachments[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, attachments);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "FrameBufferError" << std::endl;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}


	glViewport(0, 0, LUT_TEX_Size, LUT_TEX_Size);
	for (Int32 FrameBufferIndex = 0; FrameBufferIndex < LUT_TEX_Size; FrameBufferIndex++)
	{
		LUTMaterialInst->SetUniform<Int32>("LayerIndex", FrameBufferIndex);

		glBindFramebuffer(GL_FRAMEBUFFER, LUT_FrameBuffer[FrameBufferIndex]);
		glClear(GL_COLOR_BUFFER_BIT);
		LUTMaterialInst->GetParent()->Draw(pPObj->VAO, pPObj->NumFaces, pPObj->IndexType);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	_Scene->GetCamera(CameraIndex::MainCamera)->ActiveViewPort();
}

Vector2f ToneMapping::GrainRandomFromFrame(Int32 FrameCountMode8)
{
	Int32 Index = FrameCountMode8 & 1023;
	Int32 BaseX = 2;
	Int32 BaseY = 3;

	Vector2f Result = Vector2f(0.0);
	Vector2f InvBase = Vector2f(1.0 / BaseX, 1.0 / BaseY);
	Vector2f Fraction = InvBase;

	while (Index > 0)
	{
		Result.x += (Index % BaseX) * Fraction.x;
		Index /= BaseX;
		Fraction *= InvBase;
	}

	Index = FrameCountMode8 & 1023;
	Fraction = InvBase;
	while (Index > 0)
	{
		Result.y += (Index % BaseY) * Fraction.y;
		Index /= BaseY;
		Fraction *= InvBase;
	}

	return Result;
}

Int32 LayerIndex = 0;
void ToneMapping::Execute(UInt32 VAO, Int32 NumFaces, IndexSizeType indexType)
{
	glClear(GL_COLOR_BUFFER_BIT);
	if (APP_FrameCount % 5 == 0)
	{
		LayerIndex++;
		LayerIndex %= 32;
	}

	Vector2f GrainRandomFull = GrainRandomFromFrame(APP_FrameCount % 8);
	ToneMappingMaterialInst->SetUniform<Vector2f>(ToneMappingUniformDatas->GrainRandomFullID, GrainRandomFull);

	ToneMappingMaterialInst->GetParent()->Draw(VAO, NumFaces, indexType);
}
