#include <glew.h>
#include <glfw3.h>
#include <iostream>
#include "SceneManager.h"
#include "Quaternion.h"
#include "GlobalPram.h"
#include "ConstructScene.h"
#include "DeferRenderPipeline.h"
#include "BufferManager.h"
#include "ReflectionCapture.h"
#include "SystemTextures.h"

void framebuffer_size_callback(GLFWwindow* window, Int32 width, Int32 height);
void processInput(GLFWwindow *window);

// settings
std::string AssetFolderPath = "..\\Assets\\";

std::shared_ptr<SceneManager> _Scene;
std::shared_ptr<BufferManager> _GPUBuffers;
std::shared_ptr<SystemTextureFactory> GlobalTextures;

const UInt32 _ScreenWidth = 1200;
const UInt32 _ScreenHeight = 900;

UInt32 APP_DeltaTime = 0;
unsigned long APP_FrameCount = 0;

Bool bPause = false;

void CameraRotateLeftPlus()
{
	std::shared_ptr<Camera> camera = _Scene->GetCamera(CameraIndex::MainCamera);
	Vector3f angles = camera->GetEulerAngle();
	angles.z -= 1.0f;
	camera->SetEulerAngle(angles);
}

void CameraRotateLeftMinus()
{
	std::shared_ptr<Camera> camera = _Scene->GetCamera(CameraIndex::MainCamera);
	Vector3f angles = camera->GetEulerAngle();
	angles.z += 1.0f;
	camera->SetEulerAngle(angles);
}
void CameraRotateUpPlus()
{
	std::shared_ptr<Camera> camera = _Scene->GetCamera(CameraIndex::MainCamera);
	Vector3f angles = camera->GetEulerAngle();
	angles.y += 1.0f;
	camera->SetEulerAngle(angles);
}

void CameraRotateUpMinus()
{
	std::shared_ptr<Camera> camera = _Scene->GetCamera(CameraIndex::MainCamera);
	Vector3f angles = camera->GetEulerAngle();
	angles.y -= 1.0f;
	camera->SetEulerAngle(angles);
}

void CameraTranslateLeftPlus()
{
	std::shared_ptr<Camera> camera = _Scene->GetCamera(CameraIndex::MainCamera);
	Vector3f left = camera->GetLeftward();
	Vector3f position = camera->GetPosition();
	camera->SetPosition(position + left * 0.2f);
}
void CameraTranslateLeftMinus()
{
	std::shared_ptr<Camera> camera = _Scene->GetCamera(CameraIndex::MainCamera);
	Vector3f left = camera->GetLeftward();
	Vector3f position = camera->GetPosition();
	camera->SetPosition(position - left * 0.2f);
	bPause = true;
}
void CameraTranslateForwardPlus()
{
	std::shared_ptr<Camera> camera = _Scene->GetCamera(CameraIndex::MainCamera);
	Vector3f forward = camera->GetForward();
	//forward.z = 0.0f;
	Vector3f position = camera->GetPosition();
	camera->SetPosition(position + forward * 0.2f);
}

void CameraTranslateForwardMinus()
{
	std::shared_ptr<Camera> camera = _Scene->GetCamera(CameraIndex::MainCamera);
	Vector3f forward = camera->GetForward();
	//forward.z = 0.0f;
	Vector3f position = camera->GetPosition();
	camera->SetPosition(position - forward * 0.2f);
}

GLFWwindow* InitWindow()
{
	// glfw: initialize and configure
// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(_ScreenWidth, _ScreenHeight, "DigitalHumanDemo", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return 0;
	}
	glfwSetWindowPos(window, 10, 30);
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// At init, on windows
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize glew" << std::endl;
		return 0;
	}

	return window;
}

std::shared_ptr<DeferRenderPipeline> DeferPipeline;
std::shared_ptr<MaterialInstance> SimpleObjectMaterialInst;
std::shared_ptr<SphereReflectionCapture> ReflectionActor;

void InitSimpleObjects()
{
	std::shared_ptr<Material> SimpleObjectShadowDepthMaterial = std::shared_ptr<Material>(new Material(std::vector<std::string>{ "ShadowDepthVertShader.vsh", "ShadowDepthFragShader.fsh" }));
	std::shared_ptr<MaterialInstance> SimpleObjectShadowDepthMaterialInst = std::shared_ptr<MaterialInstance>(new MaterialInstance(SimpleObjectShadowDepthMaterial));
	
	Float32 Scale = 1.5f;
	{
		std::shared_ptr<Material> SimpleObjectMaterial00 = std::shared_ptr<Material>(new Material(std::vector<std::string> { "SimpleVertShader.vsh", "SimpleFragShader.fsh" }));
		std::shared_ptr<MaterialInstance> SimpleObjectMaterialInst00 = std::shared_ptr<MaterialInstance>(new MaterialInstance(SimpleObjectMaterial00));
		SimpleObjectMaterialInst00->SetUniform<Vector3f>("ColorTest", Vector3f(0.25, 0.0, 0.0));
		std::shared_ptr<SimpleObject> Cube00 = std::shared_ptr<SimpleObject>(new SimpleObject("BasicModel\\Cube.FBX", SimpleObjectShadowDepthMaterialInst, SimpleObjectMaterialInst00));
		Cube00->ObjectTransform.SetPosition(Vector3f(-7.5, -7.5, 0.0) * Scale);
		_Scene->AddObj(ObjectType::StaticMesh, Cube00);

		std::shared_ptr<Material> SimpleObjectMaterial01 = std::shared_ptr<Material>(new Material(std::vector<std::string> { "SimpleVertShader.vsh", "SimpleFragShader.fsh" }));
		std::shared_ptr<MaterialInstance> SimpleObjectMaterialInst01 = std::shared_ptr<MaterialInstance>(new MaterialInstance(SimpleObjectMaterial01));
		SimpleObjectMaterialInst01->SetUniform<Vector3f>("ColorTest", Vector3f(0.5, 0.0, 0.0));
		std::shared_ptr<SimpleObject> Cube01 = std::shared_ptr<SimpleObject>(new SimpleObject("BasicModel\\Cube.FBX", SimpleObjectShadowDepthMaterialInst, SimpleObjectMaterialInst01));
		Cube01->ObjectTransform.SetPosition(Vector3f(-2.5, -7.5, 0.0) * Scale);
		_Scene->AddObj(ObjectType::StaticMesh, Cube01);

		std::shared_ptr<Material> SimpleObjectMaterial02 = std::shared_ptr<Material>(new Material(std::vector<std::string> { "SimpleVertShader.vsh", "SimpleFragShader.fsh" }));
		std::shared_ptr<MaterialInstance> SimpleObjectMaterialInst02 = std::shared_ptr<MaterialInstance>(new MaterialInstance(SimpleObjectMaterial02));
		SimpleObjectMaterialInst02->SetUniform<Vector3f>("ColorTest", Vector3f(0.75, 0.0, 0.0));
		std::shared_ptr<SimpleObject> Cube02 = std::shared_ptr<SimpleObject>(new SimpleObject("BasicModel\\Cube.FBX", SimpleObjectShadowDepthMaterialInst, SimpleObjectMaterialInst02));
		Cube02->ObjectTransform.SetPosition(Vector3f(2.5, -7.5, 0.0) * Scale);
		_Scene->AddObj(ObjectType::StaticMesh, Cube02);

		std::shared_ptr<Material> SimpleObjectMaterial03 = std::shared_ptr<Material>(new Material(std::vector<std::string> { "SimpleVertShader.vsh", "SimpleFragShader.fsh" }));
		std::shared_ptr<MaterialInstance> SimpleObjectMaterialInst03 = std::shared_ptr<MaterialInstance>(new MaterialInstance(SimpleObjectMaterial03));
		SimpleObjectMaterialInst03->SetUniform<Vector3f>("ColorTest", Vector3f(1.0, 0.0, 0.0));
		std::shared_ptr<SimpleObject> Cube03 = std::shared_ptr<SimpleObject>(new SimpleObject("BasicModel\\Cube.FBX", SimpleObjectShadowDepthMaterialInst, SimpleObjectMaterialInst03));
		Cube03->ObjectTransform.SetPosition(Vector3f(7.5, -7.5, 0.0) * Scale);
		_Scene->AddObj(ObjectType::StaticMesh, Cube03);
	}

	{
		std::shared_ptr<Material> SimpleObjectMaterial10 = std::shared_ptr<Material>(new Material(std::vector<std::string> { "SimpleVertShader.vsh", "SimpleFragShader.fsh" }));
		std::shared_ptr<MaterialInstance> SimpleObjectMaterialInst10 = std::shared_ptr<MaterialInstance>(new MaterialInstance(SimpleObjectMaterial10));
		SimpleObjectMaterialInst10->SetUniform<Vector3f>("ColorTest", Vector3f(0.0, 0.25, 0.0));
		std::shared_ptr<SimpleObject> Cube10 = std::shared_ptr<SimpleObject>(new SimpleObject("BasicModel\\Cube.FBX", SimpleObjectShadowDepthMaterialInst, SimpleObjectMaterialInst10));
		Cube10->ObjectTransform.SetPosition(Vector3f(-7.5, -2.5, 0.0) * Scale);
		_Scene->AddObj(ObjectType::StaticMesh, Cube10);

		std::shared_ptr<Material> SimpleObjectMaterial11 = std::shared_ptr<Material>(new Material(std::vector<std::string> { "SimpleVertShader.vsh", "SimpleFragShader.fsh" }));
		std::shared_ptr<MaterialInstance> SimpleObjectMaterialInst11 = std::shared_ptr<MaterialInstance>(new MaterialInstance(SimpleObjectMaterial11));
		SimpleObjectMaterialInst11->SetUniform<Vector3f>("ColorTest", Vector3f(0.0, 0.50, 0.0));
		std::shared_ptr<SimpleObject> Cube11 = std::shared_ptr<SimpleObject>(new SimpleObject("BasicModel\\Cube.FBX", SimpleObjectShadowDepthMaterialInst, SimpleObjectMaterialInst11));
		Cube11->ObjectTransform.SetPosition(Vector3f(-2.5, -2.5, 0.0) * Scale);
		_Scene->AddObj(ObjectType::StaticMesh, Cube11);

		std::shared_ptr<Material> SimpleObjectMaterial12 = std::shared_ptr<Material>(new Material(std::vector<std::string> { "SimpleVertShader.vsh", "SimpleFragShader.fsh" }));
		std::shared_ptr<MaterialInstance> SimpleObjectMaterialInst12 = std::shared_ptr<MaterialInstance>(new MaterialInstance(SimpleObjectMaterial12));
		SimpleObjectMaterialInst12->SetUniform<Vector3f>("ColorTest", Vector3f(0.0, 0.75, 0.0));
		std::shared_ptr<SimpleObject> Cube12 = std::shared_ptr<SimpleObject>(new SimpleObject("BasicModel\\Cube.FBX", SimpleObjectShadowDepthMaterialInst, SimpleObjectMaterialInst12));
		Cube12->ObjectTransform.SetPosition(Vector3f(2.5, -2.5, 0.0) * Scale);
		_Scene->AddObj(ObjectType::StaticMesh, Cube12);

		std::shared_ptr<Material> SimpleObjectMaterial13 = std::shared_ptr<Material>(new Material(std::vector<std::string> { "SimpleVertShader.vsh", "SimpleFragShader.fsh" }));
		std::shared_ptr<MaterialInstance> SimpleObjectMaterialInst13 = std::shared_ptr<MaterialInstance>(new MaterialInstance(SimpleObjectMaterial13));
		SimpleObjectMaterialInst13->SetUniform<Vector3f>("ColorTest", Vector3f(0.0, 1.0, 0.0));
		std::shared_ptr<SimpleObject> Cube13 = std::shared_ptr<SimpleObject>(new SimpleObject("BasicModel\\Cube.FBX", SimpleObjectShadowDepthMaterialInst, SimpleObjectMaterialInst13));
		Cube13->ObjectTransform.SetPosition(Vector3f(7.5, -2.5, 0.0) * Scale);
		_Scene->AddObj(ObjectType::StaticMesh, Cube13);
	}

	{
		std::shared_ptr<Material> SimpleObjectMaterial20 = std::shared_ptr<Material>(new Material(std::vector<std::string> { "SimpleVertShader.vsh", "SimpleFragShader.fsh" }));
		std::shared_ptr<MaterialInstance> SimpleObjectMaterialInst20 = std::shared_ptr<MaterialInstance>(new MaterialInstance(SimpleObjectMaterial20));
		SimpleObjectMaterialInst20->SetUniform<Vector3f>("ColorTest", Vector3f(0.0, 0.0, 0.25));
		std::shared_ptr<SimpleObject> Cube20 = std::shared_ptr<SimpleObject>(new SimpleObject("BasicModel\\Cube.FBX", SimpleObjectShadowDepthMaterialInst, SimpleObjectMaterialInst20));
		Cube20->ObjectTransform.SetPosition(Vector3f(-7.5, 2.5, 0.0) * Scale);
		_Scene->AddObj(ObjectType::StaticMesh, Cube20);

		std::shared_ptr<Material> SimpleObjectMaterial21 = std::shared_ptr<Material>(new Material(std::vector<std::string> { "SimpleVertShader.vsh", "SimpleFragShader.fsh" }));
		std::shared_ptr<MaterialInstance> SimpleObjectMaterialInst21 = std::shared_ptr<MaterialInstance>(new MaterialInstance(SimpleObjectMaterial21));
		SimpleObjectMaterialInst21->SetUniform<Vector3f>("ColorTest", Vector3f(0.0, 0.0, 0.50));
		std::shared_ptr<SimpleObject> Cube21 = std::shared_ptr<SimpleObject>(new SimpleObject("BasicModel\\Cube.FBX", SimpleObjectShadowDepthMaterialInst, SimpleObjectMaterialInst21));
		Cube21->ObjectTransform.SetPosition(Vector3f(-2.5, 2.5, 0.0) * Scale);
		_Scene->AddObj(ObjectType::StaticMesh, Cube21);

		std::shared_ptr<Material> SimpleObjectMaterial22 = std::shared_ptr<Material>(new Material(std::vector<std::string> { "SimpleVertShader.vsh", "SimpleFragShader.fsh" }));
		std::shared_ptr<MaterialInstance> SimpleObjectMaterialInst22 = std::shared_ptr<MaterialInstance>(new MaterialInstance(SimpleObjectMaterial22));
		SimpleObjectMaterialInst22->SetUniform<Vector3f>("ColorTest", Vector3f(0.0, 0.0, 0.75));
		std::shared_ptr<SimpleObject> Cube22 = std::shared_ptr<SimpleObject>(new SimpleObject("BasicModel\\Cube.FBX", SimpleObjectShadowDepthMaterialInst, SimpleObjectMaterialInst22));
		Cube22->ObjectTransform.SetPosition(Vector3f(2.5, 2.5, 0.0) * Scale);
		_Scene->AddObj(ObjectType::StaticMesh, Cube22);

		std::shared_ptr<Material> SimpleObjectMaterial23 = std::shared_ptr<Material>(new Material(std::vector<std::string> { "SimpleVertShader.vsh", "SimpleFragShader.fsh" }));
		std::shared_ptr<MaterialInstance> SimpleObjectMaterialInst23 = std::shared_ptr<MaterialInstance>(new MaterialInstance(SimpleObjectMaterial23));
		SimpleObjectMaterialInst23->SetUniform<Vector3f>("ColorTest", Vector3f(0.0, 0.0, 1.0));
		std::shared_ptr<SimpleObject> Cube23 = std::shared_ptr<SimpleObject>(new SimpleObject("BasicModel\\Cube.FBX", SimpleObjectShadowDepthMaterialInst, SimpleObjectMaterialInst23));
		Cube23->ObjectTransform.SetPosition(Vector3f(7.5, 2.5, 0.0) * Scale);
		_Scene->AddObj(ObjectType::StaticMesh, Cube23);
	}

	{
		std::shared_ptr<Material> SimpleObjectMaterial30 = std::shared_ptr<Material>(new Material(std::vector<std::string> { "SimpleVertShader.vsh", "SimpleFragShader.fsh" }));
		std::shared_ptr<MaterialInstance> SimpleObjectMaterialInst30 = std::shared_ptr<MaterialInstance>(new MaterialInstance(SimpleObjectMaterial30));
		SimpleObjectMaterialInst30->SetUniform<Vector3f>("ColorTest", Vector3f(0.25, 0.25, 0.0));
		std::shared_ptr<SimpleObject> Cube30 = std::shared_ptr<SimpleObject>(new SimpleObject("BasicModel\\Cube.FBX", SimpleObjectShadowDepthMaterialInst, SimpleObjectMaterialInst30));
		Cube30->ObjectTransform.SetPosition(Vector3f(-7.5, 7.5, 0.0) * Scale);
		_Scene->AddObj(ObjectType::StaticMesh, Cube30);

		std::shared_ptr<Material> SimpleObjectMaterial31 = std::shared_ptr<Material>(new Material(std::vector<std::string> { "SimpleVertShader.vsh", "SimpleFragShader.fsh" }));
		std::shared_ptr<MaterialInstance> SimpleObjectMaterialInst31 = std::shared_ptr<MaterialInstance>(new MaterialInstance(SimpleObjectMaterial31));
		SimpleObjectMaterialInst31->SetUniform<Vector3f>("ColorTest", Vector3f(0.50, 0.50, 0.0));
		std::shared_ptr<SimpleObject> Cube31 = std::shared_ptr<SimpleObject>(new SimpleObject("BasicModel\\Cube.FBX", SimpleObjectShadowDepthMaterialInst, SimpleObjectMaterialInst31));
		Cube31->ObjectTransform.SetPosition(Vector3f(-2.5, 7.5, 0.0) * Scale);
		_Scene->AddObj(ObjectType::StaticMesh, Cube31);

		std::shared_ptr<Material> SimpleObjectMaterial32 = std::shared_ptr<Material>(new Material(std::vector<std::string> { "SimpleVertShader.vsh", "SimpleFragShader.fsh" }));
		std::shared_ptr<MaterialInstance> SimpleObjectMaterialInst32 = std::shared_ptr<MaterialInstance>(new MaterialInstance(SimpleObjectMaterial32));
		SimpleObjectMaterialInst32->SetUniform<Vector3f>("ColorTest", Vector3f(0.75, 0.75, 0.0));
		std::shared_ptr<SimpleObject> Cube32 = std::shared_ptr<SimpleObject>(new SimpleObject("BasicModel\\Cube.FBX", SimpleObjectShadowDepthMaterialInst, SimpleObjectMaterialInst32));
		Cube32->ObjectTransform.SetPosition(Vector3f(2.5, 7.5, 0.0) * Scale);
		_Scene->AddObj(ObjectType::StaticMesh, Cube32);

		std::shared_ptr<Material> SimpleObjectMaterial33 = std::shared_ptr<Material>(new Material(std::vector<std::string> { "SimpleVertShader.vsh", "SimpleFragShader.fsh" }));
		std::shared_ptr<MaterialInstance> SimpleObjectMaterialInst33 = std::shared_ptr<MaterialInstance>(new MaterialInstance(SimpleObjectMaterial33));
		SimpleObjectMaterialInst33->SetUniform<Vector3f>("ColorTest", Vector3f(1.0, 1.0, 0.0));
		std::shared_ptr<SimpleObject> Cube33 = std::shared_ptr<SimpleObject>(new SimpleObject("BasicModel\\Cube.FBX", SimpleObjectShadowDepthMaterialInst, SimpleObjectMaterialInst33));
		Cube33->ObjectTransform.SetPosition(Vector3f(7.5, 7.5, 0.0) * Scale);
		_Scene->AddObj(ObjectType::StaticMesh, Cube33);
	}
	{
		std::shared_ptr<Material> SimpleObjectMaterial0 = std::shared_ptr<Material>(new Material(std::vector<std::string> { "SimpleVertShader.vsh", "SimpleFragShader.fsh" }));
		std::shared_ptr<MaterialInstance> SimpleObjectMaterialInst7 = std::shared_ptr<MaterialInstance>(new MaterialInstance(SimpleObjectMaterial0));
		SimpleObjectMaterialInst7->SetUniform<Vector3f>("ColorTest", Vector3f(1.0, 0.0, 1.0));
		std::shared_ptr<SimpleObject> Plane = std::shared_ptr<SimpleObject>(new SimpleObject("BasicModel\\Plane.FBX", SimpleObjectShadowDepthMaterialInst, SimpleObjectMaterialInst7));
		Plane->ObjectTransform.SetPosition(Vector3f(0.0, 0.0, -1.0));
		Plane->ObjectTransform.SetScale(Vector3f(50.0, 50.0, 50.0));
		_Scene->AddObj(ObjectType::StaticMesh, Plane);
	}
	
	std::shared_ptr<Material> SimpleObjectMaterial = std::shared_ptr<Material>(new Material(std::vector<std::string> { "SimpleVertShader.vsh", "SimpleFragShader.fsh" }));
	 SimpleObjectMaterialInst = std::shared_ptr<MaterialInstance>(new MaterialInstance(SimpleObjectMaterial));
	SimpleObjectMaterialInst->SetUniform<Vector3f>("ColorTest", Vector3f(1.0, 1.0, 1.0));
	std::shared_ptr<SimpleObject> Sphere = std::shared_ptr<SimpleObject>(new SimpleObject("BasicModel\\Sphere.FBX", SimpleObjectShadowDepthMaterialInst, SimpleObjectMaterialInst));
	Sphere->ObjectTransform.SetPosition(Vector3f(0.0, 0.0, 10.0));
	Sphere->ObjectTransform.SetScale(Vector3f(1.0, 1.0, 1.0));
	_Scene->AddObj(ObjectType::DynamicMesh, Sphere);

	ReflectionActor = std::shared_ptr<SphereReflectionCapture>(new SphereReflectionCapture(Vector3f(0.0, 0.0, 5.0), 1000.0, 1.0));
	_Scene->AddObj(ObjectType::AbstractActor, ReflectionActor);
}

void CaptureReflection()
{
	std::vector<std::shared_ptr<Object>> AbstractActors = _Scene->GetObjects(ObjectType::AbstractActor);
	for (Int32 Index = 0; Index < AbstractActors.size(); Index++)
	{
		SphereReflectionCapture * Capture = dynamic_cast<SphereReflectionCapture*>(AbstractActors[Index].get());
		if (Capture != nullptr)
		{
			Capture->CaptureWithPipeLine(DeferPipeline);
			Capture->CalReflectionCubeTexAvgBrightness();
		}
	}
}

void InitApplication()
{
	GlobalTextures = std::shared_ptr<SystemTextureFactory>(new SystemTextureFactory());
	GlobalTextures->GeneratePreIntegratedGFTexture();
}

void InitView()
{
	_Scene = std::shared_ptr<SceneManager>(new SceneManager());
	_GPUBuffers = std::shared_ptr<BufferManager>(new BufferManager());
	_Scene->SetCurrentEditMode(ModeState::EditMode::CameraTranslation);
	std::shared_ptr<Camera> ViewCamera = std::shared_ptr<Camera>(new Camera(Vector3f(0.0, 30.0, 10.0), Vector3f(0.0, 0.0, -90.0), Math::Radians(60.0), (Float32)_ScreenWidth / (Float32)_ScreenHeight, 0.1f, 100.0f, Vector2i(_ScreenWidth, _ScreenHeight)));
	//std::shared_ptr<Camera> ViewCamera = std::shared_ptr<Camera>(new Camera(Vector3f(0, 0.0, 50.0), Vector3f(0.0, -90.0, 0.0), Math::Radians(60.0), (Float32)_ScreenWidth / (Float32)_ScreenHeight, 0.10f, 100.0f, Vector2i(_ScreenWidth, _ScreenHeight)));

	_Scene->AddCamera(CameraIndex::MainCamera, ViewCamera);

	std::shared_ptr<LightGroup> Lights = std::shared_ptr<LightGroup>(new LightGroup());
	
	DeferPipeline = std::shared_ptr<DeferRenderPipeline>(new DeferRenderPipeline());
	DeferPipeline->Init();

	InitSimpleObjects();

	CaptureReflection();
	SimpleObjectMaterialInst->SetTextureID("ReflectionTex", ReflectionActor->GetReflectionTextureID());
	SimpleObjectMaterialInst->SetTextureID("PreIntegratedGF", GlobalTextures->GetPreIntegratedGF_Tex());
	SimpleObjectMaterialInst->SetUniform<Float32>("AverageBrightness", ReflectionActor->GetAverageBrightness());
	SimpleObjectMaterialInst->SetUniform<Float32>("Brightness", ReflectionActor->GetBrightness());
	SimpleObjectMaterialInst->SetUniform<Vector4f>("CapturePositionAndInfluenceRadius", Vector4f(ReflectionActor->ObjectTransform.GetPosition(), ReflectionActor->GetInfluenceRaidus()));
	SimpleObjectMaterialInst->SetUniform<Int32>("bReflect", 1);

	_Scene->GetCamera(CameraIndex::MainCamera)->ActiveViewPort();
}


Float32 angle = 0.0f;
Float32 R = 15.0f;

void Render()
{
	Vector3f Position = Vector3f(R * cos(angle), R * sin(angle), 10.0);
	//_Scene->GetCamera(MainCamera)->SetPosition(Position);
	//_Scene->GetCamera(MainCamera)->SetDirection(Vector3f(0.0, 0.0, 10.0) - Position, Vector3f(0.0, 0.0, 1.0));
	//CaptureReflection();
	angle += 0.01f;
	if(angle >= 3.141592654f * 2.0f)
	{
		angle = 0.0f;
	}
	//CaptureReflection();
	_Scene->GetCamera(CameraIndex::MainCamera)->ActiveViewPort();

	_Scene->InternalUpdate();
	_Scene->Update();

	DeferPipeline->TAAPass->UpdateJitter();

	_GPUBuffers->UpdateViewBuffer(_Scene->GetCamera(CameraIndex::MainCamera).get());
	_GPUBuffers->UpdateCustomBufferData();


	_Scene->PrepareShadowDepthMaterial();
	DeferPipeline->RenderShadowDepthPass(StaticMesh | DynamicMesh);

	_Scene->PrepareLightingMaterial();
	DeferPipeline->RenderLightingPass(StaticMesh | DynamicMesh);

	DeferPipeline->RenderSSSPass();

	DeferPipeline->ExecuteTemporalAA();
	DeferPipeline->ExecuteToneMapping();
	
	_Scene->UpdatePreFrameData();
	DeferPipeline->TAAPass->RemoveJitter();
	
}

Int32 main()
{
	Bool b = glewInit();
	GLFWwindow* Window = InitWindow();
	InitApplication();
	InitView();
	Bool a = glewInit();
	// uncomment this call to draw in wireframe polygons.
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(Window))
	{
		// input
		// -----
		processInput(Window);

		// render
		// ------
		glClear(GL_COLOR_BUFFER_BIT);
		Render();

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
	 // -------------------------------------------------------------------------------
		if (bPause)
		{
			Int32 a = 0;
		}
		glfwSwapBuffers(Window);
		glfwPollEvents();
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		bPause = !bPause;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		CameraTranslateForwardPlus();
	else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		CameraTranslateForwardMinus();
	else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		CameraTranslateLeftPlus();
	else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		CameraTranslateLeftMinus();
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		CameraRotateLeftPlus();
	else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		CameraRotateLeftMinus();
	else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		CameraRotateUpPlus();
	else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		CameraRotateUpMinus();
	else if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
	{
		//MikeHead->m_bstop = !MikeHead->m_bstop;
		//MikeHair->m_bstop = !MikeHair->m_bstop;
	}
	else if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
	{
		std::shared_ptr<Material> MikeHeadMaterial = std::shared_ptr<Material>(new Material(std::vector<std::string> {"HeadResources\\HeadStanderdVertShader.vsh", "HeadResources\\HeadStanderdFragShader.fsh"}));
		//MikeHeadMaterialInst->ChangedParentDynamic(MikeHeadMaterial);

		std::shared_ptr<Material> MikeHairMaterial = std::shared_ptr<Material>(new Material(std::vector<std::string> {"HairResources\\HairStanderdVertShader.vsh", "HairResources\\HairStanderdFragShader.fsh"}));
		//MikeHairMaterialInst->ChangedParentDynamic(MikeHairMaterial);

		//std::string VertSourceCode3 = ReadSourceCode("..\\Assets\\BustResources\\BustHairStanderdVertShader.vsh");
		//std::string FragSourceCode3 = ReadSourceCode("..\\Assets\\BustResources\\BustHairStanderdFragShader.fsh");
		//std::shared_ptr<Material> newMaterial3 = std::shared_ptr<Material>(new Material(VertSourceCode3, FragSourceCode3));
		//BustHairMaterialInst->ChangedParentDynamic(newMaterial3);
	}
	else if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
	{
		std::shared_ptr<Material> newMaterial = std::shared_ptr<Material>(new Material(std::vector<std::string> {"DrawRectVertShader.vsh", "UE4TemporalAAFragShader.fsh"}));
		DeferPipeline->TAAPass->TAAPassMaterialInst->ChangedParentDynamic(newMaterial);
	}
	else if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
	{
		std::shared_ptr<Material> newMaterial = std::shared_ptr<Material>(new Material(std::vector<std::string> {"ToneMappingVertShader.vsh", "ToneMappingFragShader.fsh"}));
		DeferPipeline->ToneMappingPass->ToneMappingMaterialInst->ChangedParentDynamic(newMaterial);
	}
	else if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
	{
		std::string VertSourceCode = ReadSourceCode("..\\Assets\\DrawRectVertShader.vsh");
		std::string FragSourceCode = ReadSourceCode("..\\Assets\\SubsurfaceRecombine.fsh");
		std::shared_ptr<Material> newMaterial = std::shared_ptr<Material>(new Material({ VertSourceCode, FragSourceCode }));
		DeferPipeline->SSSPass->SSSRecombineMaterialInst->ChangedParentDynamic(newMaterial);
	}
	else if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
	{
		std::string VertSourceCode = ReadSourceCode("..\\Assets\\DrawRectVertShader.vsh");
		std::string FragSourceCode = ReadSourceCode("..\\Assets\\SubsurfaceSetup.fsh");
		std::shared_ptr<Material> newMaterial = std::shared_ptr<Material>(new Material({ VertSourceCode, FragSourceCode }));
		DeferPipeline->SSSPass->SSSSetupMaterialInst->ChangedParentDynamic(newMaterial);
	}
	else if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
	{
		std::string VertSourceCode = ReadSourceCode("..\\Assets\\DrawRectVertShader.vsh");
		std::string FragSourceCode = ReadSourceCode("..\\Assets\\SubsurfaceScatering.fsh");
		std::shared_ptr<Material> newMaterial = std::shared_ptr<Material>(new Material({ VertSourceCode, FragSourceCode }));
		DeferPipeline->SSSPass->SSSScateringMaterialInst->ChangedParentDynamic(newMaterial);
	}
	else if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
	{
		/*std::string VertSourceCode = ReadSourceCode("..\\Assets\\DrawRectVertShader.vsh");
		std::string FragSourceCode = ReadSourceCode("..\\Assets\\SubsurfaceSetup.fsh");
		std::shared_ptr<Material> newMaterial1 = std::shared_ptr<Material>(new Material(VertSourceCode, FragSourceCode));
		DeferPipeline->SSSPass->SSSSetupMaterialInst->ChangedParentDynamic(newMaterial1);

		VertSourceCode = ReadSourceCode("..\\Assets\\DrawRectVertShader.vsh");
		FragSourceCode = ReadSourceCode("..\\Assets\\SubsurfaceScatering.fsh");
		std::shared_ptr<Material> newMaterial2 = std::shared_ptr<Material>(new Material(VertSourceCode, FragSourceCode));
		DeferPipeline->SSSPass->SSSScateringMaterialInst->ChangedParentDynamic(newMaterial2);

		VertSourceCode = ReadSourceCode("..\\Assets\\DrawRectVertShader.vsh");
		FragSourceCode = ReadSourceCode("..\\Assets\\SubsurfaceRecombine.fsh");
		std::shared_ptr<Material> newMaterial3 = std::shared_ptr<Material>(new Material(VertSourceCode, FragSourceCode));
		DeferPipeline->SSSPass->SSSRecombineMaterialInst->ChangedParentDynamic(newMaterial3);*/
	}
	else if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, Int32 width, Int32 height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}