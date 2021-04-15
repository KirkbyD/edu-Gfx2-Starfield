#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>
#include <memory>

#ifdef _DEBUG
#define DEBUG_NEW new (_NORMAL_BLOCK , __FILE__ , __LINE__, __func__)
#else
#define DBG_NEW
#endif

#pragma region Includes
#include "GLCommon.hpp"
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <map>
#include <algorithm>
#include "cModelLoader.hpp"			
#include "cVAOManager.hpp"
#include "cModelObject.hpp"
#include "cShaderManager.hpp"
#include <sstream>
#include <limits.h>
#include <float.h>
#include "DebugRenderer/cDebugRenderer.hpp"
#include "cLightHelper.hpp"
#include "cLightManager.hpp"
#include "cFileManager.hpp"
#include "cError.hpp"
#include "cCallbacks.hpp"
#include "Globals.hpp"
#include "cModelManager.hpp"
#include "cCommandManager.hpp"
#include "cAudioManager.hpp"
#include "cMediator.hpp"
#include "cComplexObject.hpp"
#include "cPhysicsManager.hpp"
#include "cParticleEmitter.hpp"
#include "cBasicTextureManager.hpp"
#include "cTextRenderer.hpp"
#include "cLowPassFilter.hpp"
#include "cRenderer.hpp"
#include "cAnimationManager.hpp"
#include "cCamera.hpp""
#include "cCameraManager.hpp"
#include "cLuaManager.hpp"
#include "cFBO.hpp"
#pragma endregion

//FOR MIDTERM
bool LUDICROUS_SPEED = false;
bool WARPING = false;
bool STAR_SPAWNING = true;

// TEXT RENDERER GLOBALS
GLFWwindow* _window;
GLFWwindow* _text_window;
unsigned _program;
unsigned _text_program;
unsigned _uniform_tex;
unsigned _attribute_coord;
unsigned _uniform_color;
unsigned _dp_vbo;
int _window_width, _width;
int _window_height, _height;
const float MAX_HEIGHT = 1200.0f;
FT_Library _ft;
FT_Face _face;
GLfloat _current_y_position;
GLfloat _y_offset = 40.0f;
char _text_buffer[512];
int _xpos, _ypos;

// DEVCON GLOBALS
std::string _cmd_result;
bool isDataDisplay = false;
bool isDebugDisplay = false;
bool isDebugModel = false;
bool isDebugModelNormals = false;
bool isDebugModelTriangles = false;
bool isDebugAABB = true;
bool isDebugAABBTriangles = true;
bool isDebugAABBNormals = true;
bool isDebugCollision = true;
bool isDebugLights = true;

/**************Textures**************/
std::map<std::string, GLuint> mpTexture;
/**************Textures**************/

std::map<std::string, cMesh*> mpMesh;
std::map<std::string, cLightObject*> mpLight;
std::map<std::string, cModelObject*> mpModel;
std::map<std::string, cAudioObject*> mpAudio;
std::map<std::string, FMOD::ChannelGroup*> mpChannelGoups;
std::map<std::string, std::string> mpScripts;

std::map<std::string, cAnimationComponent*> mpAnima;

double deltaTime = 0.0f;
double lastFrame = 0.0f;

bool bLightDebugSheresOn = true;

int vpLightObjectsCurrentPos = 0;
int vpComplexObjectsCurrentPos = 0;
std::map<std::string, FMOD::ChannelGroup*>::iterator itChannelGroup;
std::map<std::string, cModelObject*>::iterator itModelObjects;
std::map<std::string, cAnimationComponent*>::iterator itAnimaComps;

// VARIABLES FOR SELECTING AND CONTROLING VARIOUS ASPECTS OF THE ENGINE AT RUNTIME
//		isDevCons		=		Is Developer Console Enabled?
//		isLightSelect	=		Is Lights Selectable Enabled?
//		isModelSelect	=		Is Models Selectable Enabled?
//		isObjCtrl		=		Is Objects Controllable Enabled?
bool isDevCon = false;
bool isModelSelect = false;
bool isLightSelect = false;
bool isComplexSelect = false;
bool isAudioSelect = false;
bool isObjCtrl = true;

bool isCutSceneActive = false;
bool isCutSceneActive2 = false;
bool isCutSceneActive3 = false;

std::string cmdstr;

// Load up my "scene"  (now global)
// OBJECT VECTORS
std::vector<cModelObject*> g_vec_pGameObjects;
std::vector<cLightObject*> g_vec_pLightObjects;
std::vector<cActorObject*> g_vec_pActorObjects;
std::vector<cComplexObject*> g_vec_pComplexObjects;
std::vector<cMesh*> g_vec_pMeshObjects;
std::vector<DSP> g_vec_DSP;


// MAPS
//std::map<std::string /*FriendlyName*/, cModelObject*> g_map_GameObjectsByFriendlyName;
std::map<std::string, cFBO*> FBO_map;

// returns NULL (0) if we didn't find it.
cModelObject* pFindObjectByFriendlyName(std::string name);
cModelObject* pFindObjectByFriendlyNameMap(std::string name);


// MOVED USER INPUT TO AN EXTERNAL FILE.

// command to target different object types...
// model = meshes
// lighting = lights
// ...
// command = target parm ||| target lighting as an example
// commands should handle no parms, 1 parm, or two parms.

std::map<std::string, eShapeTypes> g_mp_String_to_ShapeType;

void PopulateShapeTypeMap() {
	g_mp_String_to_ShapeType["CUBE"] = eShapeTypes::CUBE;
	g_mp_String_to_ShapeType["MESH"] = eShapeTypes::MESH;
	g_mp_String_to_ShapeType["PLANE"] = eShapeTypes::PLANE;
	g_mp_String_to_ShapeType["SPHERE"] = eShapeTypes::SPHERE;
	g_mp_String_to_ShapeType["UNKNOWN"] = eShapeTypes::UNKNOWN;
}

std::string baseTexture;

void ClearGlobals();

int main() {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	PopulateShapeTypeMap();

	cError errmngr;
	cFileManager* pFileManager = cFileManager::GetFileManager();

	//SET UP PHYSICS STUFF
	cPhysicsManager* pPhysicsManager = cPhysicsManager::GetPhysicsManager();
	pFileManager->LoadPhysicsLibraryStruct(pPhysicsManager->GetDLLStruct());
	pPhysicsManager->InitializePhysicsSystem();
	//SET UP PHYSICS STUFF

	cLightManager* pLightManager = cLightManager::GetLightManager();
	cModelManager* pModelManager = cModelManager::GetModelManager();
	cAudioManager* pAudioManager = cAudioManager::GetAudioManager();
	cAnimationManager* pAnimaManager = cAnimationManager::GetAnimationManager();
	cCommandManager* pCommandManager = cCommandManager::GetCommandManager();
	cCameraManager* pCameraManager = cCameraManager::GetCameraManager();
	pCameraManager->Initialize();
	cCamera* mainCamera = pCameraManager->GetActiveCamera();

	cLuaManager* pLuaManager = new cLuaManager();
	cTextRenderer* pTextRenderer = new cTextRenderer();

	cMediator* pMediator = new cMediator();
	pMediator->ConnectEndpointClients();

	cRenderer* pRenderer = new cRenderer();
	cBasicTextureManager* pTextureManager = new cBasicTextureManager();

	std::map<std::string, cShaderManager::cShader> mpVertexShader;
	std::map<std::string, cShaderManager::cShader> mpFragmentShader;
	std::map<std::string, cShaderManager::cShader> mpGeometryShader;

	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_SAMPLES, 4);

	_window = glfwCreateWindow(1280, 960, "Amethyst Engine", NULL, NULL);

	if (!_window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	// for window transparency & stops user manipulation of devcon window
	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
	glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
	_text_window = glfwCreateWindow(1280, 960, "Dev Console", NULL, _window);
	// change the windows height and width based off of the main windows


	if (!_text_window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwGetWindowPos(_window, &_xpos, &_ypos);
	glfwSetWindowPos(_text_window, _xpos, _ypos);
	glfwGetWindowSize(_window, &_width, &_height);
	glfwSetWindowSize(_text_window, _width, _height);
	glfwGetWindowSize(_text_window, &_window_width, &_window_height);
	
	glfwSetKeyCallback(_window, key_callback);
	glfwSetKeyCallback(_text_window, key_callback);
	glfwSetMouseButtonCallback(_window, mouse_button_callback);
	glfwSetCursorPosCallback(_window, cursor_position_callback);
	glfwSetScrollCallback(_window, scroll_callback);

	glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(_window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glfwSwapInterval(1);
	glfwHideWindow(_text_window);

	// FOR DYLANS LAPTOP
	glfwSetWindowOpacity(_text_window, 0.7f);

	pAudioManager->LoadDSPs();

	cDebugRenderer* pDebugRenderer = new cDebugRenderer();
	pDebugRenderer->initialize();

	cModelLoader* pTheModelLoader = new cModelLoader();	// Heap
	cShaderManager* pTheShaderManager = new cShaderManager();

	pTextRenderer->InitFreetype();

	pFileManager->LoadShaders(pFileManager->GetShadersFile(), mpVertexShader, mpFragmentShader, mpGeometryShader);

	//pFileManager->LoadLuaScripts();

	// change LoadShaders to bool and return true or false based off of success, then change below to return -1 if false.
	// error will display from the LoadShaders function.
	if (!pTheShaderManager->createProgramFromFile("SimpleShader", mpVertexShader["exampleVertexShader01"], mpFragmentShader["exampleFragmentShader01"])) {
		errmngr.DisplayError("Main", __func__, "shader_compile_failure", "Could not compile shader", pTheShaderManager->getLastError());
		return -1;
	}
	if (!pTheShaderManager->createProgramFromFile("SimpleTextShader", mpVertexShader["exampleTextVertexShader01"], mpFragmentShader["exampleTextFragmentShader01"])) {
		errmngr.DisplayError("Main", __func__, "shader_compile_failure", "Could not compile shader", pTheShaderManager->getLastError());
		return -1;
	}

	GLuint shaderProgID = pTheShaderManager->getIDFromFriendlyName("SimpleShader");
	GLuint textShaderProgID = pTheShaderManager->getIDFromFriendlyName("SimpleTextShader");


	///*******************Named Uniform Blocks**************************/

	// Get the shader
	cShaderManager::cShaderProgram* pSP = pTheShaderManager->pGetShaderProgramFromFriendlyName("SimpleShader");

	// Get the active uniforms (All of them, not just NUBs)
	pSP->LoadActiveUniforms();

	///*****glGetActiveUniformBlockName example*****/
	//const int NUB_NAME_BUFFERSIZE = 1000;
	//char NUB_name[NUB_NAME_BUFFERSIZE] = { 0 };
	//int charactersGLWrote = 0;

	//// Gets the name of the active NUB from index position into NUB_name buffer
	//glGetActiveUniformBlockName(pSP->ID,					// Shader program ID
	//							0,							// NUM number (index)
	//							NUB_NAME_BUFFERSIZE,		// Size of NUB_name buffer
	//							&charactersGLWrote,			// Length
	//							NUB_name);					// Buffer for the name of the NUB
	///*****glGetActiveUniformBlockName example*****/

	///*****Questionable example of sending data to NUB on GPU by Feeney*****/
	//// Create a buffer and bind the shader to it 
	//GLuint blockID = 0;
	//glGenBuffers(1, &blockID);
	//glBindBuffer(GL_UNIFORM_BUFFER, blockID);

	//// C++ version of GLSL NUB
	//struct cMyBlockType
	//{
	//	glm::vec2 SomeData;
	//private:
	//	float padding1[2];					// because std140 (always vec4)

	//public:
	//	glm::vec3 MoreData;
	//private:
	//	float padding2;
	//	//public:
	//	//	bool isSkybox;
	//	//private:
	//	//	float padding3[3];
	//};

	//cMyBlockType dataOnApplicationSide;

	//dataOnApplicationSide.MoreData.x = 1.0f;

	//// I think Feeney was supposed to use glNamedBufferData, but hell if I know
	//glBufferData(GL_UNIFORM_BUFFER,
	//	sizeof(dataOnApplicationSide),	// Size in bytes
	//	(void*)&dataOnApplicationSide,
	//	GL_DYNAMIC_DRAW);

	//glBindBuffer(GL_UNIFORM_BUFFER, 0);  // Should be set to 0 after we're done

	///*****Questionable example of sending data to NUB on GPU by Feeney*****/

	//// GL_MAX_UNIFORM_BLOCK_SIZE: 65536 or 16384 floats
	//// We could have a lot of lights with this (which counts as feature)

	///*******************Named Uniform Blocks**************************/





	// Create a VAO Manager...
	cVAOManager* pVAOManager = new cVAOManager();

	// move this configuration into a seperate file that loads from a configuration file.
	// Note, the "filename" here is really the "model name" that we can look up later (i.e. it doesn't have to be the file name)


	//move the physics stuff into a physics.ini so that it can be loaded in the physics engine instead of the graphics engine portion of the game engine file loading.
	// At this point, the model is loaded into the GPU

	// make a call to the filemanager to create the gameobjects from the loaded file.
	pFileManager->BuildObjects(pFileManager->GetModelFile());
	pFileManager->LoadModelsOntoGPU(pTheModelLoader, pVAOManager, shaderProgID, pFileManager->GetMeshFile(), mpMesh);

	/**************Textures**************/
	pFileManager->LoadTexturesOntoGPU(pTextureManager, pFileManager->GetTextureFile(), mpTexture);
	pFileManager->LoadSkyboxTexturesOntoGPU(pTextureManager, pFileManager->GetSkyboxTextureFile(), mpTexture);
	/**************Textures**************/

	// Enable depth test
	glEnable(GL_DEPTH);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glfwMakeContextCurrent(_text_window);
	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	//glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_TRUE);
	glfwMakeContextCurrent(_window);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	cLowPassFilter avgDeltaTimeThingy;

	cLightHelper* pLightHelper = new cLightHelper();

	// Get the initial time
	double lastTime = glfwGetTime();

	pFileManager->BuildObjects(pFileManager->GetLightsFile());
	pFileManager->BuildObjects(pFileManager->GetAudioFile());

	// Assigning DSP's 
	itChannelGroup = mpChannelGoups.begin();
	for (size_t i = 0; i < g_vec_DSP.size(); ++i) {
		if (i % 3 == 0 && i != 0)
			++itChannelGroup;

		g_vec_DSP[i].channelgroup = itChannelGroup->second;
		g_vec_DSP[i].bypass = true;
		//0 as the index still applies them in order, like (i%3) did.
		pAudioManager->_result = g_vec_DSP[i].channelgroup->addDSP(0, g_vec_DSP[i].dsp);
		pAudioManager->error_check();
		pAudioManager->_result = g_vec_DSP[i].dsp->setBypass(g_vec_DSP[i].bypass);
		pAudioManager->error_check();
	}
	itChannelGroup = mpChannelGoups.begin();
	itModelObjects = mpModel.begin();

	//BUILD COMPLEX OBJECTS
	//pFileManager->BuildObjects(pFileManager->GetComplexObjFile());

	//pFileManager->BuildEntities(pPhysicsManager, pAnimaManager);
	////Must be done after entity Loading
	//pFileManager->LoadAnimationsOntoGPU(pAnimaManager, pTheVAOManager, shaderProgID);

	if (g_vec_pComplexObjects.size() != 0) {
		vpComplexObjectsCurrentPos = g_vec_pComplexObjects.size() - 1;
		baseTexture = g_vec_pComplexObjects[vpComplexObjectsCurrentPos]->GetModels()[0]->v_textureNames[0];
		g_vec_pComplexObjects[vpComplexObjectsCurrentPos]->GetModels()[0]->v_textureNames[0] = "Yellow";
		mainCamera->SetFollowObject(g_vec_pComplexObjects[vpComplexObjectsCurrentPos]);
		mainCamera->SetFollowDistance(5.0f);
		mainCamera->SetType(CameraType::objectCam);
		mainCamera->SetFollowOffset(glm::vec3(50.f, 50.f, 50.f));
		isComplexSelect = true;
	}
	else if (g_vec_pGameObjects.size() != 0) {
		mainCamera->SetFollowObject((*itModelObjects).second);
		mainCamera->SetFollowDistance(5.0f);
		mainCamera->SetType(CameraType::objectCam);
		mainCamera->SetFollowOffset(glm::vec3(50.f, 50.f, 50.f));
		isModelSelect = true;
	}

	_uniform_tex = glGetUniformLocation(textShaderProgID, "tex");
	_attribute_coord = glGetAttribLocation(textShaderProgID, "coord");
	_uniform_color = glGetUniformLocation(textShaderProgID, "color");
	glGenBuffers(1, &_dp_vbo);

	//for (cModelObject* Model : g_vec_pGameObjects) {
		//if (Model->physicsShapeType == AABB)
			//pPhysicsManager->CalcAABBsForMeshModel(Model);
	//}
	//pPhysicsManager->DrawAABBs();

	cModelObject* EarthPlane = pFindObjectByFriendlyName("EarthQuad");
	cModelObject* StarEmitter = pFindObjectByFriendlyName("StarEmitter");
	StarEmitter->recalculateWorldMatrix();

	/*****************************************Instance Imposter Particles*****************************************/
	cParticleEmitter* pMyPartcles = new cParticleEmitter();
	pMyPartcles->location = StarEmitter->getPosition();
	pMyPartcles->acceleration = glm::vec3(0.0f, 0.0f, 0.0f);
	pMyPartcles->Initialize(
		0.5f,								// Min Size
		0.8f,								// Max Size
		glm::vec3(-10.0f, -10.0f, 50.0f),	// Min Vel
		glm::vec3(10.0f, 10.0f, 300.0f),	// Max Vel
		glm::vec3(0.0f, 0.0f, 0.0f),		// Min Delta Position
		glm::vec3(0.0f, 0.0f, 0.0f),		// Max delta Position
		0.0f,	// Min life
		25.0f,	// Max life
		00,		// Min # of particles per frame
		80000);		// Max # of particles per frame


	pMyPartcles->location = glm::vec3(StarEmitter->positionXYZ.x, StarEmitter->positionXYZ.y, StarEmitter->positionXYZ.z);

	// The VBO containing the mat models of the particles
	GLuint particles_mat_model_buffer;
	glGenBuffers(1, &particles_mat_model_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particles_mat_model_buffer);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, pMyPartcles->getMaxParticles() * sizeof(glm::mat4), NULL, GL_STREAM_DRAW);

	/*****************************************Instance Imposter Particles*****************************************/

	lastTime = glfwGetTime();

	glm::vec4 waterOffset = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

	pCommandManager->PopulateSubsystemMap();
	pCommandManager->PopulateCommandMap();
	pCommandManager->PopulateCommandStateMap();
	pCommandManager->PopulateCommandArgsMap();



	for (int index = 0; index < ::g_vec_pGameObjects.size(); index++)
	{

		std::string FBOError;

		if (::g_vec_pGameObjects[index]->objectType == "SecondPass")
		{
			FBO_map[::g_vec_pGameObjects[index]->friendlyName] = new cFBO();

			if (FBO_map[::g_vec_pGameObjects[index]->friendlyName]->init(1920, 1080, FBOError))
			{
				std::cout << "Frame buffer is OK" << std::endl;
			}
			else
			{
				std::cout << "FBO Error: " << FBOError << std::endl;
			}
		}

		if (::g_vec_pGameObjects[index]->objectType == "Reflective")
		{
			FBO_map[::g_vec_pGameObjects[index]->friendlyName] = new cFBO();

			if (FBO_map[::g_vec_pGameObjects[index]->friendlyName]->initCubeMap(512, FBOError))
			{
				std::cout << "Frame buffer is OK" << std::endl;
			}
			else
			{
				std::cout << "FBO Error: " << FBOError << std::endl;
			}
		}
	}


	pCameraManager->CreateCamera("QuadCamera");
	cCamera* quadCamera = pCameraManager->GetCameraByName("QuadCamera");
	quadCamera->SetPosition(glm::vec3(0.0f, 0.0f, -50.0f));
	//quadCamera->SetFront(glm::vec3(0.0f, 0.0f, 50.0f));
	quadCamera->SetLookAtObject(pFindObjectByFriendlyName("FullScreenQuad"));
	quadCamera->SetUpVector(glm::vec3(0.0f, 1.0f, 0.0f));

	pCameraManager->CreateCamera("starCam");
	cCamera* starCam = pCameraManager->GetCameraByName("starCam");
	glm::vec3 starCamPos = StarEmitter->getPosition();
	starCamPos.z += 2000.0f;
	starCam->SetPosition(starCamPos);
	starCam->SetLookAtObject(StarEmitter);
	starCam->MoveFront();

	EarthPlane->positionXYZ.z = starCam->GetPosition().z - 50.f;

	mainCamera->SetFollowDistance(1.0f);
	mainCamera->SetLookAtObject(pFindObjectByFriendlyName("StarQuad"));
	mainCamera->SetFollowObject(NULL);
	mainCamera->SetPosition(glm::vec3(2.f, 3.5f, 12.f));
	
	while (!glfwWindowShouldClose(_window)) {

		/***************************************/

		int width, height;
		glfwGetFramebufferSize(_window, &width, &height);

		/***************************************/

		// Clear both the colour buffer (what we see) and the depth (or z) buffer.
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/***************************************/

		// DEBUG TITLE CURRENT OBJECT SELECTED
		if (mainCamera->GetFollowObject() == nullptr)
			glfwSetWindowTitle(_window, "FreeCame Mode");
		else if (isComplexSelect)
			glfwSetWindowTitle(_window, g_vec_pComplexObjects[vpComplexObjectsCurrentPos]->friendlyName.c_str());
		else if (isModelSelect)
			glfwSetWindowTitle(_window, (*itModelObjects).second->friendlyName.c_str());
		else if (isLightSelect)
			glfwSetWindowTitle(_window, g_vec_pLightObjects[vpLightObjectsCurrentPos]->friendlyName.c_str());
		else if (isAudioSelect)
			glfwSetWindowTitle(_window, itChannelGroup->first.c_str());

		/***************************************/

		for (auto const& x : FBO_map)
		{
			std::string FBOError;
			if (!(x.second->isCubeMap))
			{
				if (x.second->reset(width, height, FBOError))
				{
					std::cout << "Frame buffer is OK" << std::endl;
				}
				else
				{
					std::cout << "FBO Error: " << FBOError << std::endl;
				}
			}
			// Not really needed
			//else
			//{
			//	if (x.second->resetCubeMap(512, FBOError))
			//	{
			//		std::cout << "Frame buffer is OK" << std::endl;
			//	}
			//	else
			//	{
			//		std::cout << "FBO Error: " << FBOError << std::endl;
			//	}
			//}
		}

		/***************************************/

		// Get the initial time
		double currentTime = glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		const double SOME_HUGE_TIME = 0.1;	// 100 ms;
		if (deltaTime > SOME_HUGE_TIME)
		{
			deltaTime = SOME_HUGE_TIME;
		}

		avgDeltaTimeThingy.addValue(deltaTime);

		/***************************************/

		//if (!isDevCon) {
		//	if (!pCommandManager->_map_script_to_cmd["P2Trigger"]->Update(deltaTime)) { }
		//	if (isCutSceneActive) {
		//		isCutSceneActive = !pCommandManager->_map_script_to_cmd["P2Script"]->Update(deltaTime);
		//	}
		//	if (isCutSceneActive2) {
		//		isCutSceneActive2 = !pCommandManager->_map_script_to_cmd["P2ScriptSerialParallel"]->Update(deltaTime);
		//	}
		//	if (isCutSceneActive3) {
		//		isCutSceneActive3 = !pCommandManager->_map_script_to_cmd["P2ScriptParallelSerial"]->Update(deltaTime);
		//	}
		//}

		/***************************************/

		glm::mat4 ProjectionMatrix, ViewMatrix;

		float ratio;
		ratio = (height > 0.0f) ? (width / (float)height) : 0.0f;

		// Projection matrix
		ProjectionMatrix = glm::perspective(0.6f,		// FOV  // Might need to change this during the exam
			ratio,			// Aspect ratio
			0.1f,			// Near clipping plane
			10000.0f);		// Far clipping plane

		//mainCamera->Move();
		pCameraManager->Update();
		ViewMatrix = mainCamera->GetViewMatrix();

		GLint matView_UL = glGetUniformLocation(shaderProgID, "matView");
		GLint matProj_UL = glGetUniformLocation(shaderProgID, "matProj");

		glUniformMatrix4fv(matView_UL, 1, GL_FALSE, glm::value_ptr(ViewMatrix));
		glUniformMatrix4fv(matProj_UL, 1, GL_FALSE, glm::value_ptr(ProjectionMatrix));

		/***************************************/

		// Set the position of my "position" (the camera)
		GLint eyeLocation_UL = glGetUniformLocation(shaderProgID, "eyeLocation");

		glm::vec3 CameraPosition = mainCamera->GetPosition();

		glUniform4f(eyeLocation_UL,
			CameraPosition.x,
			CameraPosition.y,
			CameraPosition.z,
			1.0f);

		/***************************************/

		pLightManager->LoadLightsToGPU(g_vec_pLightObjects, shaderProgID);

		/***************************************/

		//		std::sort( ::g_vec_pGameObjects.begin(), ::g_vec_pGameObjects.end(), 
		//				   isACloserThanB(::g_pFlyCamera->position ) );

		// Single pass of the bubble sort 
		//for (int index = 0; index != (::g_vec_pGameObjects.size() - 1); index++)
		//{
		//	glm::vec3 ObjA = ::g_vec_pGameObjects[index]->positionXYZ;
		//	glm::vec3 ObjB = ::g_vec_pGameObjects[index + 1]->positionXYZ;
		//	if (glm::distance(ObjA, CameraPosition) < glm::distance(ObjB, CameraPosition))
		//	{
		//		// Out of order, so swap the positions...
		//		cModelObject* pTemp = ::g_vec_pGameObjects[index];
		//		::g_vec_pGameObjects[index] = ::g_vec_pGameObjects[index + 1];
		//		::g_vec_pGameObjects[index + 1] = pTemp;
		//	}
		//}

		std::vector<cModelObject*> basicModels;
		std::vector<cModelObject*> transparentModels;
		std::vector<cModelObject*> skyboxModels;
		std::vector<cModelObject*> secondPassModels;
		std::vector<cModelObject*> reflectiveModels;
		std::vector<cModelObject*> stencilPortalModels;

		unsigned int highestSceneNumber = 0;

		for (int index = 0; index < ::g_vec_pGameObjects.size(); index++)
		{
			if (::g_vec_pGameObjects[index]->scene > highestSceneNumber)
			{
				highestSceneNumber = ::g_vec_pGameObjects[index]->scene;
			}

			if (::g_vec_pGameObjects[index]->objectType == "Basic")
			{
				basicModels.push_back(::g_vec_pGameObjects[index]);
			}
			else if (::g_vec_pGameObjects[index]->objectType == "Transparent")
			{
				transparentModels.push_back(::g_vec_pGameObjects[index]);
			}
			else if (::g_vec_pGameObjects[index]->objectType == "Skybox")
			{
				skyboxModels.push_back(::g_vec_pGameObjects[index]);
			}
			else if (::g_vec_pGameObjects[index]->objectType == "SecondPass")
			{
				secondPassModels.push_back(::g_vec_pGameObjects[index]);
			}
			else if (::g_vec_pGameObjects[index]->objectType == "Reflective")
			{
				reflectiveModels.push_back(::g_vec_pGameObjects[index]);
			}
			else if (::g_vec_pGameObjects[index]->objectType == "FlatStencilPortal" || ::g_vec_pGameObjects[index]->objectType == "VolumetricStencilPortal")
			{
				stencilPortalModels.push_back(::g_vec_pGameObjects[index]);
			}
		}

		// Single pass of the bubble sort for transparent models
		for (int index = 0; index < transparentModels.size(); index++)
		{
			glm::vec3 ObjA = transparentModels[index]->positionXYZ;
			glm::vec3 ObjB = transparentModels[(index + 1)]->positionXYZ;

			if (glm::distance(ObjA, CameraPosition) < glm::distance(ObjB, CameraPosition))
			{
				// Out of order, so swap the positions
				cModelObject* pTemp = transparentModels[index];
				transparentModels[index] = transparentModels[(index + 1)];
				transparentModels[(index + 1)] = pTemp;
			}
		}

		std::vector<std::vector<cModelObject*>> scenes;

		for (int scene_index = 0; scene_index < highestSceneNumber + 1; scene_index++)
		{
			std::vector<cModelObject*> tmpScene;

			for (int index = 0; index < skyboxModels.size(); index++)
			{
				if (skyboxModels[index]->scene == scene_index)
				{
					tmpScene.push_back(skyboxModels[index]);
				}
			}
			for (int index = 0; index < basicModels.size(); index++)
			{
				if (basicModels[index]->scene == scene_index)
				{
					tmpScene.push_back(basicModels[index]);
				}
			}
			for (int index = 0; index < secondPassModels.size(); index++)
			{
				if (secondPassModels[index]->scene == scene_index)
				{
					tmpScene.push_back(secondPassModels[index]);
				}
			}
			for (int index = 0; index < reflectiveModels.size(); index++)
			{
				if (reflectiveModels[index]->scene == scene_index)
				{
					tmpScene.push_back(reflectiveModels[index]);
				}
			}
			for (int index = 0; index < stencilPortalModels.size(); index++)
			{
				if (stencilPortalModels[index]->scene == scene_index)
				{
					tmpScene.push_back(stencilPortalModels[index]);
				}
			}
			for (int index = 0; index < transparentModels.size(); index++)
			{
				if (transparentModels[index]->scene == scene_index)
				{
					tmpScene.push_back(transparentModels[index]);
				}
			}

			scenes.push_back(tmpScene);
		}

		/***************************************/

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Set the passNumber to 0
		GLint passNumber_UniLoc = glGetUniformLocation(shaderProgID, "passNumber");
		glUniform1i(passNumber_UniLoc, 0);  //"passNumber"

	
		//LUDICROUS SPEED
		if (LUDICROUS_SPEED) {
			pMyPartcles->m_minVelocity *= 6;
			pMyPartcles->m_maxVelocity *= 6;
			LUDICROUS_SPEED = false;
			WARPING = true;
		}
		else if (WARPING) {
			//Resizing Logic

			if (EarthPlane->scale >= 0.1f) {
				WARPING = false;
				STAR_SPAWNING = false;
			}
			else {
				pMyPartcles->m_minVelocity.z *= 0.5;
				pMyPartcles->m_maxVelocity.z *= 0.5;
				pMyPartcles->setMaxParticles(pMyPartcles->getMaxParticles() * 0.90);

				EarthPlane->scale *= 1.101 - EarthPlane->scale;
				EarthPlane->recalculateWorldMatrix();

				StarEmitter->alphaTransparency = fabsf((EarthPlane->scale*10) - 1);
			}

			//MOVING LOGIC I coded before realising you wanted scaling
			/*starCamPos = starCam->GetPosition();
			float dist = glm::distance(starCamPos, EarthPlane->getPosition());

			if (dist <= 50.f) {
				WARPING = false;
				STAR_SPAWNING = false;
			}
			else {
				pMyPartcles->m_minVelocity.z *= 0.95;
				pMyPartcles->m_maxVelocity.z *= 0.95;
				pMyPartcles->setMaxParticles(pMyPartcles->getMaxParticles() * 0.95);

				starCamPos.z -= dist * 0.25 * avgDeltaTimeThingy.getAverage();
				starCam->SetPosition(starCamPos);
				EarthPlane->positionXYZ.z += dist * 0.75 * avgDeltaTimeThingy.getAverage();
			}*/
		}

		//midterm starfield deferred rendering
		pRenderer->RenderScene(scenes[2], FBO_map["StarQuad"], pCameraManager->GetCameraByName("starCam"), shaderProgID, pVAOManager);

		//Emitting particles
		if (STAR_SPAWNING) {
			glBindBuffer(GL_ARRAY_BUFFER, particles_mat_model_buffer);
			glBufferData(GL_ARRAY_BUFFER, pMyPartcles->getMaxParticles() * sizeof(glm::mat4), NULL, GL_STATIC_DRAW); // Buffer orphaning, a common way to improve streaming perf.
			glBufferSubData(GL_ARRAY_BUFFER, 0, pMyPartcles->getParticlesCount() * sizeof(glm::mat4), pMyPartcles->g_particule_mat_model_data);
		}

		pRenderer->RenderInstancedParticleObject(StarEmitter, shaderProgID, pVAOManager, pMyPartcles);

		pRenderer->RenderScene(scenes[1], FBO_map["FullScreenQuad"], mainCamera, shaderProgID, pVAOManager);
		
		pRenderer->RenderScene(scenes[0], NULL, quadCamera, shaderProgID, pVAOManager);


		//pAnimaManager->Update(avgDeltaTimeThingy.getAverage(), shaderProgID, pRenderer, pTheVAOManager);
		//pAnimaManager->Render(pRenderer, shaderProgID, pTheVAOManager);

		glDisable(GL_STENCIL_TEST);							// Disable stencil test

		/***************************************/

		//DEBUG Drawing - hook up to a mediator boolean
		if (isDebugDisplay) {
			// Bounds:		Cyan	glm::vec3(0.0f, 1.0f, 1.0f)
			// Collision:	Red		glm::vec3(1.0f, 0.0f, 0.0f)
			// Triangles:	Purple	glm::vec3(1.0f, 0.0f, 1.0f)
			// Normals:		Yellow	glm::vec3(1.0f, 1.0f, 0.0f)
			// ModelCenter:	Green	glm::vec3(0.0f, 1.0f, 0.0f)
			// Lights:		White	glm::vec3(1.0f, 1.0f, 1.0f)
			//std::vector<collisionPoint*> thePoints = g_vec_pComplexObjects[vpComplexObjectsCurrentPos]->GetCollisionPoints();

			//if (isDebugAABB) {
			//	std::set<cAABB*> setAABBs;
			//	for (size_t i = 0; i < thePoints.size(); i++) {
			//		cAABB* theAABB = pPhysicsManager->GetPointAABB(thePoints[i]->Position);
			//		if (theAABB != nullptr)
			//			setAABBs.emplace(theAABB);
			//	}
			//	//AABB
			//	for (cAABB* theAABB : setAABBs) {
			//		//Box Min / Max - Cyan
			//		float sideLength = theAABB->GetLength();
			//		//min Corner
			//		{
			//			glm::vec3 minCorner = theAABB->GetMin();
			//			glm::vec3 minX = minCorner;
			//			minX.x += sideLength;
			//			pDebugRenderer->addLine(minCorner, minX, glm::vec3(0.0f, 1.0f, 1.0f), 0.1f);
			//			glm::vec3 minY = minCorner;
			//			minY.y += sideLength;
			//			pDebugRenderer->addLine(minCorner, minY, glm::vec3(0.0f, 1.0f, 1.0f), 0.1f);
			//			glm::vec3 minZ = minCorner;
			//			minZ.z += sideLength;
			//			pDebugRenderer->addLine(minCorner, minZ, glm::vec3(0.0f, 1.0f, 1.0f), 0.1f);
			//		}
			//		//max Corner
			//		{
			//			glm::vec3 maxCorner = theAABB->GetMax();
			//			glm::vec3 maxX = maxCorner;
			//			maxX.x -= sideLength;
			//			pDebugRenderer->addLine(maxCorner, maxX, glm::vec3(0.0f, 1.0f, 1.0f), 0.1f);
			//			glm::vec3 maxY = maxCorner;
			//			maxY.y -= sideLength;
			//			pDebugRenderer->addLine(maxCorner, maxY, glm::vec3(0.0f, 1.0f, 1.0f), 0.1f);
			//			glm::vec3 maxZ = maxCorner;
			//			maxZ.z -= sideLength;
			//			pDebugRenderer->addLine(maxCorner, maxZ, glm::vec3(0.0f, 1.0f, 1.0f), 0.1f);
			//		}

			//		//AABB Contents
			//		if (isDebugAABBTriangles || isDebugAABBNormals) {
			//			for (size_t i = 0; i < theAABB->vec_pAABBTriangles.size(); i++) {
			//				//Triangle - Purple
			//				if (isDebugAABBTriangles) {
			//					pDebugRenderer->addTriangle(theAABB->vec_pAABBTriangles[i].vertsPos[0],
			//						theAABB->vec_pAABBTriangles[i].vertsPos[1],
			//						theAABB->vec_pAABBTriangles[i].vertsPos[2],
			//						glm::vec3(1.0f, 0.0f, 1.0f));
			//				}

			//				if (isDebugAABBNormals) {
			//					//Normals - Yellow
			//					glm::vec3 triNorm = glm::normalize(theAABB->vec_pAABBTriangles[i].vertsNorm[0] + theAABB->vec_pAABBTriangles[i].vertsNorm[1] + theAABB->vec_pAABBTriangles[i].vertsNorm[2]);
			//					glm::vec3 triCenter = (theAABB->vec_pAABBTriangles[i].vertsPos[0] +
			//						theAABB->vec_pAABBTriangles[i].vertsPos[1] +
			//						theAABB->vec_pAABBTriangles[i].vertsPos[2]) / 3.0f;
			//					pDebugRenderer->addLine(triCenter,
			//						(triCenter + triNorm * 5.0f),
			//						glm::vec3(1.0f, 1.0f, 0.0f),
			//						0.1f);
			//				}
			//			}
			//		}
			//	}
			//}
			
			//Draw collision Points - red
			/*if (isDebugCollision) {
				for (int i = 0; i < g_vec_pComplexObjects.size(); i++) {
					std::vector<collisionPoint*> Points = g_vec_pComplexObjects[i]->GetCollisionPoints();
					for (int j = 0; j < Points.size(); j++) {
						if (Points[j]->Collided >= 5.0f) {
							pDebugRenderer->addPointPointSize(Points[j]->Position, glm::vec3(1.0f, 0.0f, 0.0f), 5.0f);
							Points[j]->Collided = -1.0f;
						}
						else if (Points[j]->Collided >= 0.0f) {
							pDebugRenderer->addPointPointSize(Points[j]->Position, glm::vec3(1.0f, 0.0f, 1.0f), 10.0f);
							Points[j]->Collided += deltaTime;
						}
						else
							pDebugRenderer->addPointPointSize(Points[j]->Position, glm::vec3(1.0f, 0.0f, 0.0f), 5.0f);
					}
				}
			}*/

			if (isDebugModel) {
				//Model center point
				//(*itModelObjects).second
				pDebugRenderer->addPointPointSize((*itModelObjects).second->positionXYZ, glm::vec3(0.0f, 1.0f, 0.0f), 5.0f);

				//render bounding box
				//recalcs model world matrix:
				//BoundingBox modelBB = pPhysicsManager->CalculateBoundingBox((*itModelObjects).second);
				
				/*if (isDebugModelNormals || isDebugModelTriangles) {
					cMesh theMesh;
					pPhysicsManager->CalculateTransformedMesh(*mpMesh[(*itModelObjects).second->meshName], (*itModelObjects).second->matWorld, theMesh);
					for (size_t i = 0; i < theMesh.vecTriangles.size(); i++) {
						sPlyTriangle theTriangle = theMesh.vecTriangles[i];

						glm::vec3 v1 = glm::vec3(theMesh.vecVertices[theTriangle.vert_index_1].x, theMesh.vecVertices[theTriangle.vert_index_1].y, theMesh.vecVertices[theTriangle.vert_index_1].z);
						glm::vec3 v2 = glm::vec3(theMesh.vecVertices[theTriangle.vert_index_2].x, theMesh.vecVertices[theTriangle.vert_index_2].y, theMesh.vecVertices[theTriangle.vert_index_2].z);
						glm::vec3 v3 = glm::vec3(theMesh.vecVertices[theTriangle.vert_index_3].x, theMesh.vecVertices[theTriangle.vert_index_3].y, theMesh.vecVertices[theTriangle.vert_index_3].z);

						if (isDebugModelNormals) {
							glm::vec3 n1 = glm::vec3(theMesh.vecVertices[theTriangle.vert_index_1].nx, theMesh.vecVertices[theTriangle.vert_index_1].ny, theMesh.vecVertices[theTriangle.vert_index_1].nz);
							glm::vec3 n2 = glm::vec3(theMesh.vecVertices[theTriangle.vert_index_2].nx, theMesh.vecVertices[theTriangle.vert_index_2].ny, theMesh.vecVertices[theTriangle.vert_index_2].nz);
							glm::vec3 n3 = glm::vec3(theMesh.vecVertices[theTriangle.vert_index_2].nx, theMesh.vecVertices[theTriangle.vert_index_2].ny, theMesh.vecVertices[theTriangle.vert_index_2].nz);
							glm::vec3 triNorm = glm::normalize(n1 + n2 + n3);
							glm::vec3 triCenter = (v1 + v2 + v3) / 3.0f;
							pDebugRenderer->addLine(triCenter,
								(triCenter + triNorm * 5.0f),
								glm::vec3(1.0f, 1.0f, 0.0f),
								0.1f);
						}

						if (isDebugModelTriangles) {
							pDebugRenderer->addTriangle(v1,	v2, v3, glm::vec3(1.0f, 0.0f, 1.0f));
						}
					}
				}*/
			}

			if (isDebugLights) {
				for (size_t i = 0; i < g_vec_pLightObjects.size(); i++) {
					pDebugRenderer->addPointPointSize(g_vec_pLightObjects[i]->position, glm::vec3(1.0f, 1.0f, 1.0f), 10.0f);
				}

				float sphereSize = pLightHelper->calcApproxDistFromAtten(
								0.95f,		// 95% brightness
								0.001f,		// Within 0.1%
								100000.0f,	// Will quit when it's at this distance
								g_vec_pLightObjects[vpLightObjectsCurrentPos]->atten.x,
								g_vec_pLightObjects[vpLightObjectsCurrentPos]->atten.y,
								g_vec_pLightObjects[vpLightObjectsCurrentPos]->atten.z);
				pDebugRenderer->addDebugSphere(g_vec_pLightObjects[vpLightObjectsCurrentPos]->position, glm::vec3(0.95f, 0.95f, 0.95f), sphereSize);

				sphereSize = pLightHelper->calcApproxDistFromAtten(
								0.75f,		// 75% brightness
								0.001f,		// Within 0.1%
								100000.0f,	// Will quit when it's at this distance
								g_vec_pLightObjects[vpLightObjectsCurrentPos]->atten.x,
								g_vec_pLightObjects[vpLightObjectsCurrentPos]->atten.y,
								g_vec_pLightObjects[vpLightObjectsCurrentPos]->atten.z);
				pDebugRenderer->addDebugSphere(g_vec_pLightObjects[vpLightObjectsCurrentPos]->position, glm::vec3(0.75f, 0.75f, 0.75f), sphereSize);

				sphereSize = pLightHelper->calcApproxDistFromAtten(
								0.50f,		// 50% brightness
								0.001f,		// Within 0.1%
								100000.0f,	// Will quit when it's at this distance
								g_vec_pLightObjects[vpLightObjectsCurrentPos]->atten.x,
								g_vec_pLightObjects[vpLightObjectsCurrentPos]->atten.y,
								g_vec_pLightObjects[vpLightObjectsCurrentPos]->atten.z);
				pDebugRenderer->addDebugSphere(g_vec_pLightObjects[vpLightObjectsCurrentPos]->position, glm::vec3(0.5f, 0.5f, 0.5f), sphereSize);

				sphereSize = pLightHelper->calcApproxDistFromAtten(
								0.25f,		// 25% brightness
								0.001f,		// Within 0.1%
								100000.0f,	// Will quit when it's at this distance
								g_vec_pLightObjects[vpLightObjectsCurrentPos]->atten.x,
								g_vec_pLightObjects[vpLightObjectsCurrentPos]->atten.y,
								g_vec_pLightObjects[vpLightObjectsCurrentPos]->atten.z);
				pDebugRenderer->addDebugSphere(g_vec_pLightObjects[vpLightObjectsCurrentPos]->position, glm::vec3(0.25f, 0.25f, 0.25f), sphereSize);

				sphereSize = pLightHelper->calcApproxDistFromAtten(
								0.01f,		// 1% brightness (essentially black)
								0.001f,		// Within 0.1%
								100000.0f,	// Will quit when it's at this distance
								g_vec_pLightObjects[vpLightObjectsCurrentPos]->atten.x,
								g_vec_pLightObjects[vpLightObjectsCurrentPos]->atten.y,
								g_vec_pLightObjects[vpLightObjectsCurrentPos]->atten.z);
				pDebugRenderer->addDebugSphere(g_vec_pLightObjects[vpLightObjectsCurrentPos]->position, glm::vec3(0.1f, 0.1f, 0.1f), sphereSize);
			}

			pDebugRenderer->RenderDebugObjects(ViewMatrix, ProjectionMatrix, 0.01f);
		}

		/***************************************/
		//glm::vec3 currentNosePos = g_vec_pComplexObjects[0]->GetCollisionPoints()[0]->Position;
		//std::string currentAABB = pPhysicsManager->CurrentAABB(currentNosePos);
		//int currentTriangles = pPhysicsManager->TrianglesInCurrentAABB(currentNosePos);

		if (isDevCon) {
			// Reset text y position
			_current_y_position = 30.0f;
			glfwMakeContextCurrent(_text_window);

			glfwGetWindowPos(_window, &_xpos, &_ypos);
			glfwSetWindowPos(_text_window, _xpos, _ypos);
			glfwGetWindowSize(_window, &_width, &_height);
			glfwSetWindowSize(_text_window, _width, _height);
			glfwGetWindowSize(_text_window, &_window_width, &_window_height);

			glfwGetFramebufferSize(_text_window, &_window_width, &_window_height);
			glViewport(0, 0, width, height);
			glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// help commands:
			// /help, /help lights, /help complex, /help audio, /help model, /help physics, /help gamelogichere
			// for displayed data, have predefined data layouts that would be loaded for display...
			// TODO: CREATE PREDEFINED DATA DISPLAYS FOR DISPLAY
			// TODO: Help Displays, Models/Meshes, Lights, Physics, Commands, ComplexObjects, Actors, Audio, GameLogic
			sprintf_s(_text_buffer, "%s", _cmd_result.c_str());
			pTextRenderer->RenderText(_text_buffer, textShaderProgID);
			sprintf_s(_text_buffer, "CMD >> %s", cmdstr.c_str());
			pTextRenderer->RenderText(_text_buffer, textShaderProgID);
			pTextRenderer->RenderText("", textShaderProgID);
			//printf();

			//sprintf_s(_text_buffer, "Current AABB: %s\n", currentAABB.c_str());
			//pTextRenderer->RenderText(_text_buffer, textShaderProgID);
			//sprintf_s(_text_buffer, "Triangles: %d\n", currentTriangles);
			//pTextRenderer->RenderText(_text_buffer, textShaderProgID);

			if (isDataDisplay) {
				// data displayed will depend on the object selected.
				// complex objects will be the most complicated objects to display data for.
				// models should display position, rotation, scale, friendlyname, meshname, and idname
				// colour as well, thier object state (wireframe debug or solid, etc.)
				pTextRenderer->RenderText("Data Display", textShaderProgID);
				if (isModelSelect) {
					pTextRenderer->RenderText("Model/Mesh Objects", textShaderProgID);
					sprintf_s(_text_buffer, "Name: %s", (*itModelObjects).second->friendlyName.c_str());
					pTextRenderer->RenderText(_text_buffer, textShaderProgID);
					sprintf_s(_text_buffer, "Position (XYZ): %f, %f, %f", (*itModelObjects).second->positionXYZ.x, (*itModelObjects).second->positionXYZ.y, (*itModelObjects).second->positionXYZ.z);
					pTextRenderer->RenderText(_text_buffer, textShaderProgID);
					sprintf_s(_text_buffer, "Rotation (XYZ): %f, %f, %f", (*itModelObjects).second->getEulerAngle().x, (*itModelObjects).second->getEulerAngle().y, (*itModelObjects).second->getEulerAngle().z);
					pTextRenderer->RenderText(_text_buffer, textShaderProgID);
					sprintf_s(_text_buffer, "Scale: %f", (*itModelObjects).second->scale);
					pTextRenderer->RenderText(_text_buffer, textShaderProgID);
					sprintf_s(_text_buffer, "Color (RGBA): %f, %f, %f, %f", (*itModelObjects).second->objectColourRGBA.r, (*itModelObjects).second->objectColourRGBA.g, (*itModelObjects).second->objectColourRGBA.b, (*itModelObjects).second->objectColourRGBA.a);
					pTextRenderer->RenderText(_text_buffer, textShaderProgID);
					sprintf_s(_text_buffer, "Diffuse (RGBA): %f, %f, %f, %f", (*itModelObjects).second->diffuseColour.r, (*itModelObjects).second->diffuseColour.g, (*itModelObjects).second->diffuseColour.b, (*itModelObjects).second->diffuseColour.a);
					pTextRenderer->RenderText(_text_buffer, textShaderProgID);
					sprintf_s(_text_buffer, "Specular (RGBW): %f, %f, %f, %f", (*itModelObjects).second->specularColour.r, (*itModelObjects).second->specularColour.g, (*itModelObjects).second->specularColour.b, (*itModelObjects).second->specularColour.w);
					pTextRenderer->RenderText(_text_buffer, textShaderProgID);
					sprintf_s(_text_buffer, "Wireframe Mode: %s", ((*itModelObjects).second->isWireframe) ? "ENABLED" : "DISABLED");
					pTextRenderer->RenderText(_text_buffer, textShaderProgID);
					sprintf_s(_text_buffer, "Visible Mode: %s", ((*itModelObjects).second->isVisible) ? "ENABLED" : "DISABLED");
					pTextRenderer->RenderText(_text_buffer, textShaderProgID);
					sprintf_s(_text_buffer, "Physics Object Type (ENUM): %d", (*itModelObjects).second->physicsShapeType);
					pTextRenderer->RenderText(_text_buffer, textShaderProgID);
					sprintf_s(_text_buffer, "Inverse Mass: %f", (*itModelObjects).second->inverseMass);
					pTextRenderer->RenderText(_text_buffer, textShaderProgID);
					sprintf_s(_text_buffer, "Velocity (XYZ): %f, %f, %f", (*itModelObjects).second->velocity.x, (*itModelObjects).second->velocity.y, (*itModelObjects).second->velocity.z);
					pTextRenderer->RenderText(_text_buffer, textShaderProgID);
					sprintf_s(_text_buffer, "Acceleration (XYZ): %f, %f, %f", (*itModelObjects).second->accel.x, (*itModelObjects).second->accel.y, (*itModelObjects).second->accel.z);
					pTextRenderer->RenderText(_text_buffer, textShaderProgID);
				}
				if (isLightSelect) {
					pTextRenderer->RenderText("Light Objects", textShaderProgID);
					sprintf_s(_text_buffer, "Name: %s", g_vec_pLightObjects[vpLightObjectsCurrentPos]->friendlyName.c_str());
					pTextRenderer->RenderText(_text_buffer, textShaderProgID);
					sprintf_s(_text_buffer, "Position (XYZ): %f, %f, %f, %f", g_vec_pLightObjects[vpLightObjectsCurrentPos]->position.x, g_vec_pLightObjects[vpLightObjectsCurrentPos]->position.y, g_vec_pLightObjects[vpLightObjectsCurrentPos]->position.z, g_vec_pLightObjects[vpLightObjectsCurrentPos]->position.w);
					pTextRenderer->RenderText(_text_buffer, textShaderProgID);
					sprintf_s(_text_buffer, "Diffuse (RGBA): %f, %f, %f, %f", g_vec_pLightObjects[vpLightObjectsCurrentPos]->diffuse.r, g_vec_pLightObjects[vpLightObjectsCurrentPos]->diffuse.g, g_vec_pLightObjects[vpLightObjectsCurrentPos]->diffuse.b, g_vec_pLightObjects[vpLightObjectsCurrentPos]->diffuse.a);
					pTextRenderer->RenderText(_text_buffer, textShaderProgID);
					sprintf_s(_text_buffer, "Specular (RGBW): %f, %f, %f, %f", g_vec_pLightObjects[vpLightObjectsCurrentPos]->specular.r, g_vec_pLightObjects[vpLightObjectsCurrentPos]->specular.g, g_vec_pLightObjects[vpLightObjectsCurrentPos]->specular.b, g_vec_pLightObjects[vpLightObjectsCurrentPos]->specular.w);
					pTextRenderer->RenderText(_text_buffer, textShaderProgID);
					sprintf_s(_text_buffer, "Attenuation (XYZW): %f, %f, %f, %f", g_vec_pLightObjects[vpLightObjectsCurrentPos]->atten.x, g_vec_pLightObjects[vpLightObjectsCurrentPos]->atten.y, g_vec_pLightObjects[vpLightObjectsCurrentPos]->atten.z, g_vec_pLightObjects[vpLightObjectsCurrentPos]->atten.w);
					pTextRenderer->RenderText(_text_buffer, textShaderProgID);
					sprintf_s(_text_buffer, "Direction (XYZW): %f, %f, %f, %f", g_vec_pLightObjects[vpLightObjectsCurrentPos]->direction.x, g_vec_pLightObjects[vpLightObjectsCurrentPos]->direction.y, g_vec_pLightObjects[vpLightObjectsCurrentPos]->direction.z, g_vec_pLightObjects[vpLightObjectsCurrentPos]->direction.w);
					pTextRenderer->RenderText(_text_buffer, textShaderProgID);
					sprintf_s(_text_buffer, "Light Type / Cone (XYZW): %f, %f, %f, %f", g_vec_pLightObjects[vpLightObjectsCurrentPos]->param1.x, g_vec_pLightObjects[vpLightObjectsCurrentPos]->param1.y, g_vec_pLightObjects[vpLightObjectsCurrentPos]->param1.z, g_vec_pLightObjects[vpLightObjectsCurrentPos]->param1.w);
					pTextRenderer->RenderText(_text_buffer, textShaderProgID);
					sprintf_s(_text_buffer, "Light On / Off: %s", (g_vec_pLightObjects[vpLightObjectsCurrentPos]->param2.x) ? "ON" : "OFF");
					pTextRenderer->RenderText(_text_buffer, textShaderProgID);
				}
				if (isAudioSelect) {
					pTextRenderer->RenderText("Audio Objects To Be Implemented At A Later Date...", textShaderProgID);
					// TODO: Implement the audio items such that we can display information about the audio items.
				}
				if (isComplexSelect) {
					pTextRenderer->RenderText("Complex Objects To Be Implemented At A Later Date...", textShaderProgID);
					// TODO: Implement the complex items such that we can display information about the complex items.
				}
			}

			glfwMakeContextCurrent(_window);
		}

		//	Pause physics updates if the dev console is open.
		if (!isDevCon) {
			/*****************************************Instance Imposter Particles*****************************************/
			pMyPartcles->Step(deltaTime, CameraPosition);
			/*****************************************Instance Imposter Particles*****************************************/

			// Update the objects through physics
			pPhysicsManager->WorldUpdate(avgDeltaTimeThingy.getAverage());

			g_vec_pComplexObjects;
			g_vec_pGameObjects;

			/*****************************************  FMOD  *****************************************/
			/*pAudioManager->_result = pAudioManager->_system->set3DListenerAttributes(0,
				&mainCamera->AudLisGetPosition(),
				&mainCamera->AudLisGetVelocity(),
				&mainCamera->AudLisGetForward(),
				&mainCamera->AudLisGetUp());
			pAudioManager->error_check();
			pAudioManager->_system->update();
			pAudioManager->error_check();	*/		
			/*****************************************  FMOD  *****************************************/
		}

		glfwSwapBuffers(_text_window);
		glfwSwapBuffers(_window);
		glfwPollEvents();
	}
	glfwDestroyWindow(_text_window);
	glfwDestroyWindow(_window);
	glfwTerminate();

	// Delete everything
	pPhysicsManager->TerminatePhysicsSystem();
	pAudioManager->Release();
	errmngr.~cError();
	pModelManager->DeconstructGraphicsComponents();

	// REMOVE THIS ONCE WE HAVE AN ENTITY MANAGER, THE MANAGER SHOULD HANDLE THE DECONSTRUCTION OF THE POINTERS TO THE ENTITIES
	for (auto a : g_vec_pComplexObjects) {
		a->DeconstructEntities();
		delete a;
	}

	pLightManager->DeconstructLightComponents();
	delete pMyPartcles;
	delete pLuaManager;
	delete pRenderer;
	pTextureManager->DeconstructTextures();
	delete pTextureManager;
	delete pTextRenderer;
	delete pTheModelLoader;
	delete pDebugRenderer;
	delete pLightHelper;
	pVAOManager->DeconstructDrawInfoComponents();
	delete pVAOManager;
	delete pMediator;
	pTheShaderManager->DeconstructShaderPrograms();
	delete pTheShaderManager;
	mpVertexShader.clear();
	mpFragmentShader.clear();
	ClearGlobals();
	exit(EXIT_SUCCESS);
}

void ClearGlobals() {
	mpTexture.clear();
	
	for (auto a : mpMesh)
		delete a.second;

	mpMesh.clear();
	mpLight.clear();
	mpModel.clear();
	mpAudio.clear();
	mpChannelGoups.clear();
	mpScripts.clear();
	g_mp_String_to_ShapeType.clear();
	g_vec_pGameObjects.clear();
	g_vec_pLightObjects.clear();
	g_vec_pActorObjects.clear();
	g_vec_pComplexObjects.clear();
	g_vec_pMeshObjects.clear();
	g_vec_DSP.clear();
}

// returns NULL (0) if we didn't find it.
cModelObject* pFindObjectByFriendlyName(std::string name) {
	// Do a linear search 
	for (unsigned int index = 0; index != g_vec_pGameObjects.size(); index++) {
		if (::g_vec_pGameObjects[index]->friendlyName == name) {
			// Found it!!
			return ::g_vec_pGameObjects[index];
		}
	}
	// Didn't find it
	return NULL;
}

// returns NULL (0) if we didn't find it.
cModelObject* pFindObjectByFriendlyNameMap(std::string name) {
	return ::mpModel[name];
}
