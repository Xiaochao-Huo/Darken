#include "Util.h"
#include "ReflectionCapture.h"
#include "DeferRenderPipeline.h"

SphereReflectionCapture::SphereReflectionCapture(const glm::vec3 &position, const float &radius, const float& brightness) :
	InfluenceRadius(radius),
	Brightness(brightness),
	CaptureTexSize(512)
{
	ObjectTransform.SetPosition(position);
	CreateCaptureResources();

	int Count = 0;
	std::vector<std::shared_ptr<Object>> AbstractActors = _Scene->GetObjects(ObjectType::AbstractActor);
	for(int Index = 0; Index < AbstractActors.size(); Index++)
	{
		if(dynamic_cast<SphereReflectionCapture*>(AbstractActors[Index].get()) != nullptr)
		{
			Count++;
		}
	}
	Create6FacesCameraList();
	_Scene->AddCamera(CameraIndex::ReflectionCaptureCamera + Count, CaptureCamera);
}

SphereReflectionCapture::~SphereReflectionCapture()
{
}

void SphereReflectionCapture::CreateCaptureResources()
{
	glGenTextures(1, &CaptureTexCube);
	glBindTexture(GL_TEXTURE_CUBE_MAP, CaptureTexCube);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA16F, CaptureTexSize, CaptureTexSize, 0, GL_RGBA, GL_HALF_FLOAT, 0);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA16F, CaptureTexSize, CaptureTexSize, 0, GL_RGBA, GL_HALF_FLOAT, 0);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA16F, CaptureTexSize, CaptureTexSize, 0, GL_RGBA, GL_HALF_FLOAT, 0);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA16F, CaptureTexSize, CaptureTexSize, 0, GL_RGBA, GL_HALF_FLOAT, 0);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA16F, CaptureTexSize, CaptureTexSize, 0, GL_RGBA, GL_HALF_FLOAT, 0);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA16F, CaptureTexSize, CaptureTexSize, 0, GL_RGBA, GL_HALF_FLOAT, 0);
	
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_LOD, 0);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LOD, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	glGenTextures(1, &CaptureTex2D);
	glBindTexture(GL_TEXTURE_2D, CaptureTex2D);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, CaptureTexSize, CaptureTexSize, 0, GL_RGBA, GL_HALF_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void SphereReflectionCapture::Create6FacesCameraList()
{
	std::shared_ptr<Camera> LCamera0 = std::shared_ptr<Camera>(new Camera(ObjectTransform.GetPosition(), glm::vec3(-90.0,  0.0,    0.0), glm::radians(90.0f), 1.0f, 0.1f, InfluenceRadius, glm::ivec2(CaptureTexSize, CaptureTexSize)));
	std::shared_ptr<Camera> LCamera1 = std::shared_ptr<Camera>(new Camera(ObjectTransform.GetPosition(), glm::vec3(90.0,  0.0,  180.0), glm::radians(90.0f), 1.0f, 0.1f, InfluenceRadius, glm::ivec2(CaptureTexSize, CaptureTexSize)));
	std::shared_ptr<Camera> LCamera2 = std::shared_ptr<Camera>(new Camera(ObjectTransform.GetPosition(), glm::vec3(180.0,  0.0,   90.0), glm::radians(90.0f), 1.0f, 0.1f, InfluenceRadius, glm::ivec2(CaptureTexSize, CaptureTexSize)));
	std::shared_ptr<Camera> LCamera3 = std::shared_ptr<Camera>(new Camera(ObjectTransform.GetPosition(), glm::vec3(0.0,  0.0,  -90.0), glm::radians(90.0f), 1.0f, 0.1f, InfluenceRadius, glm::ivec2(CaptureTexSize, CaptureTexSize)));
	std::shared_ptr<Camera> LCamera4 = std::shared_ptr<Camera>(new Camera(ObjectTransform.GetPosition(), glm::vec3(0.0,  90.0,  -90.0), glm::radians(90.0f), 1.0f, 0.1f, InfluenceRadius, glm::ivec2(CaptureTexSize, CaptureTexSize)));
	std::shared_ptr<Camera> LCamera5 = std::shared_ptr<Camera>(new Camera(ObjectTransform.GetPosition(), glm::vec3(0.0,  -90.0,  90.0), glm::radians(90.0f), 1.0f, 0.1f, InfluenceRadius, glm::ivec2(CaptureTexSize, CaptureTexSize)));

	LCamera0->SetNextCamera(LCamera1);
	LCamera1->SetNextCamera(LCamera2);
	LCamera2->SetNextCamera(LCamera3);
	LCamera3->SetNextCamera(LCamera4);
	LCamera4->SetNextCamera(LCamera5);

	CaptureCamera = LCamera0;
}

void SphereReflectionCapture::CaptureWithPipeLine(std::shared_ptr<DeferRenderPipeline> Pipeline)
{
	Camera * C = CaptureCamera.get();
	for(int FaceIndex = 0; FaceIndex < 6; FaceIndex++)
	{
		_GPUBuffers->UpdateViewBuffer(C);
		_GPUBuffers->UpdateCustomBufferData();

		_Scene->PrepareShadowDepthMaterial();
		Pipeline->RenderShadowDepthPass(StaticMesh);

		_Scene->PrepareLightingMaterial();
		Pipeline->RenderLightingPass(StaticMesh);
		Pipeline->RenderSSSPass();

		Pipeline->ExecuteTemporalAA();
		
		Util::CopyTex2D_Once(Pipeline->TAAPass->TAAToScreenFrame_Tex, CaptureTex2D, CaptureTexSize, CaptureTexSize);

		Util::FlipTex2DY_Once(CaptureTex2D, CaptureTexSize, CaptureTexSize, GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT);

		Util::CopyTex2DToCubeFace_Once(CaptureTex2D, CaptureTexCube, GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, CaptureTexSize, CaptureTexSize, FaceIndex);
				
		

		C = C->GetNextCamera().get();
	}

	glBindTexture(GL_TEXTURE_CUBE_MAP, CaptureTexCube);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);	
}

void SphereReflectionCapture::CalReflectionCubeTexAvgBrightness()
{
	//Create GPU Resources
	unsigned int CalRenderTex;
	glGenTextures(1, &CalRenderTex);
	glBindTexture(GL_TEXTURE_2D, CalRenderTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	unsigned int CalFrameBuffer;
	glGenFramebuffers(1, &CalFrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, CalFrameBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, CalRenderTex, 0);

	unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, attachments);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "CalReflectionCubeTexAvgBrightness FrameBufferError" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//Create Material
	std::shared_ptr<Material> CalculateMaterial = std::shared_ptr<Material>(new Material(std::vector<std::string> { "DrawRectVertShader.vsh", "ComputeAverageBrightnessFragShader.fsh" }));
	std::shared_ptr<MaterialInstance> ComputeMaterialInst = std::shared_ptr<MaterialInstance>(new MaterialInstance(CalculateMaterial));

	//Create Quad Geometry
	std::shared_ptr<RectBufferObject> QuadBufferObject = std::shared_ptr<RectBufferObject>(new RectBufferObject());

	//Opengl pc do not support texturelod in frag shader. So we remember and set min/max lod hear.
	int MinLod, MaxLod;
	glBindTexture(GL_TEXTURE_CUBE_MAP, CaptureTexCube);
	glGetTexParameteriv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_LOD, &MinLod);
	glGetTexParameteriv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LOD, &MaxLod);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	int NumMip = (int)glm::log2((float)CaptureTexSize);
	glBindTexture(GL_TEXTURE_CUBE_MAP, CaptureTexCube);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_LOD, NumMip);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_LOD, NumMip);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	//Calculate
	glBindFramebuffer(GL_FRAMEBUFFER, CalFrameBuffer);
	ComputeMaterialInst->SetTextureID("ReflectionTex", CaptureTexCube);
	ComputeMaterialInst->GetParent()->Draw(QuadBufferObject->VAO, QuadBufferObject->NumFaces, QuadBufferObject->IndexType);
	
	//Get Result
	unsigned char BrightnessRGBA[4];
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &BrightnessRGBA);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	//Decode RGBA8 to float
	AverageBrightness = ((float)BrightnessRGBA[0] / 10.0 + (float)BrightnessRGBA[1] / 100.0 + (float)BrightnessRGBA[2] / 1000.0 + (float)BrightnessRGBA[3] / 10000.0) / 255 * 10.0;

	//Release Buffer
	glDeleteFramebuffers(1, &CalFrameBuffer);
	glDeleteTextures(1, &CalRenderTex);
	
	//Don't forget reset min/max lod
	glBindTexture(GL_TEXTURE_CUBE_MAP, CaptureTexCube);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_LOD, MinLod);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LOD, MaxLod);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

unsigned int SphereReflectionCapture::GetReflectionTextureID()
{
	return CaptureTexCube;
}

float SphereReflectionCapture::GetInfluenceRaidus()
{
	return InfluenceRadius;
}

float SphereReflectionCapture::GetBrightness()
{
	return Brightness;
}

float SphereReflectionCapture::GetAverageBrightness()
{
	return AverageBrightness;
}