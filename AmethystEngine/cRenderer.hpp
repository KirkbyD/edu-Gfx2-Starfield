#pragma once
#include "GLCommon.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "cModelObject.hpp"
#include "cVAOManager.hpp"
#include "cParticleEmitter.hpp"
#include "cFBO.hpp"
#include "Globals.hpp"
#include "cAnimationComponent.hpp"

class cRenderer {




public:
	cRenderer() { };
	~cRenderer() { };
	void RenderScene(std::vector<cModelObject*> scene, cFBO* pFBO_Out, cCamera* camera,	GLint shaderProgID, cVAOManager* pVAOManager, bool clearScreen = true);
	void RenderSceneWithStencilPortals(std::vector<cModelObject*> mainScene, std::map<std::string, std::vector<cModelObject*>> scenesToPortals,
								cFBO* pFBO_Out, cCamera* camera, GLint shaderProgID, cVAOManager* pVAOManager, bool clearScreen = true);
	void RenderStencilPortalToScene(cModelObject* portal, std::vector<cModelObject*> scene,
								cFBO* pFBO_Out, cCamera* camera, GLint shaderProgID, cVAOManager* pVAOManager, bool clearScreen = true);

	void RenderStencilPortalObject(cModelObject* pCurrentObject, GLint shaderProgID, cVAOManager* pVAOManager);
	void Render3DObject(cModelObject* pCurrentObject, GLint shaderProgID, cVAOManager* pVAOManager);
	void RenderSecondPassObject(cModelObject* pCurrentObject, GLint shaderProgID, cVAOManager* pVAOManager);
	void RenderReflectiveObject(cModelObject* pCurrentObject, GLint shaderProgID, cVAOManager* pVAOManager);
	void RenderAnimaObject(cAnimationComponent* pCurrentObject, sModelDrawInfo* drawInfo, GLint shaderProgID, cVAOManager* pVAOManager, GLint numBonesUsed, glm::f32* pVecFinalTransformtion);
	void Render3DObjectMovingTexture(cModelObject* pCurrentObject, GLint shaderProgID, cVAOManager* pVAOManager, glm::vec2 offset);
	void Render3DObjectHeightMap(cModelObject* pCurrentObject, GLint shaderProgID, cVAOManager* pVAOManager, glm::vec2 offset);
	void RenderDebugObject();
	void RenderTextObject();
	void RenderSkyBoxObject(cModelObject* pCurrentObject, GLint shaderProgID, cVAOManager* pVAOManager);
	void RenderImposterObject();
	void RenderInstanced3DObject();
	void RenderInstancedParticleObject(cModelObject* pCurrentObject, GLint shaderProgID, cVAOManager* pVAOManager, cParticleEmitter* pParticleEmitter);
	void RenderInstancedImposterObject();

	void SetUpTextureBindingsForObject(cModelObject* pCurrentObject, GLint shaderProgID);
	void SetUpSkyBoxTextureBindingsForObject(cModelObject* pCurrentObject, GLint shaderProgID);
};