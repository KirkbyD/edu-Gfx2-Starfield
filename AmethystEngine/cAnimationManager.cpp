#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>
#include <memory>

#ifdef _DEBUG
#define DEBUG_NEW new (_NORMAL_BLOCK , __FILE__ , __LINE__)
#else
#define DBG_NEW
#endif

#include <iostream>
#include "cAnimationManager.hpp"
#include "nConvert.hpp"
#include <glm\gtc\type_ptr.hpp>


#pragma region SINGLETON
cAnimationManager cAnimationManager::stonAnimaMngr;
cAnimationManager* cAnimationManager::GetAnimationManager() { return &(cAnimationManager::stonAnimaMngr); }
cAnimationManager::cAnimationManager() {
	std::cout << "Animation Manager Created" << std::endl;
	pMediator = nullptr;
}
#pragma endregion


void cAnimationManager::DeconstructAnimationComponents() {
	//TODO
}

cAnimationComponent* cAnimationManager::LoadMeshFromFile(const std::string& friendlyName, const std::string& filename) {
	unsigned int Flags = aiProcess_Triangulate |
		aiProcess_OptimizeMeshes |
		aiProcess_OptimizeGraph |
		aiProcess_JoinIdenticalVertices |
		aiProcess_GenNormals |
		aiProcess_CalcTangentSpace;

	cAnimationComponent* AnimaObj = dynamic_cast<cAnimationComponent*>(this->_fact_game_obj.CreateGameObject("[ANIMATION]"));

	AnimaObj->SetScene(mImporter.ReadFile(filename.c_str(), Flags));

	if (AnimaObj->GetScene())
	{
		AnimaObj->SetFileName(filename);
		// Assume the friendlyName is the same as the file, for now
		AnimaObj->friendlyName = friendlyName;

		AnimaObj->SetGlobalInverseTransformation(nConvert::AIMatrixToGLMMatrix(AnimaObj->GetScene()->mRootNode->mTransformation));
		AnimaObj->SetGlobalInverseTransformation(glm::inverse(AnimaObj->GetGlobalInverseTransformation()));

		// Calcualte all the bone things
		if (!AnimaObj->Initialize())
		{	// There was an issue doing this calculation
			return nullptr;
		}
	}//if ( this->pScene )

	mpAnima[friendlyName] = AnimaObj;

	return AnimaObj;
}

void cAnimationManager::Update(float dt, GLint shaderProgID, cRenderer* pRenderer, cVAOManager* pVAOManager) {
	for (auto it : mpAnima) {
		auto pCurrentObject = it.second;
		// Set to all identity
		const int NUMBEROFBONES = pCurrentObject->GetNumBones();

		// Taken from "Skinned Mesh 2 - todo.docx"
		std::vector< glm::mat4x4 > vecFinalTransformation;
		std::vector< glm::mat4x4 > vecOffsets;
		std::vector< glm::mat4x4 > vecObjectBoneTransformation;

		// This loads the bone transforms from the animation model
		pCurrentObject->BoneTransform(dt,	// 0.0f // Frame time
			vecFinalTransformation,
			vecObjectBoneTransformation,
			vecOffsets);

		// Wait until all threads are done updating.

		GLint numBonesUsed = (GLint)vecFinalTransformation.size();

		std::vector<sModelDrawInfo*> vecDrawInfo = it.second->GetMeshes();

		for (size_t i = 0; i < vecDrawInfo.size(); i++) {
			pRenderer->RenderAnimaObject(pCurrentObject, vecDrawInfo[i], shaderProgID, pVAOManager, numBonesUsed, glm::value_ptr(vecFinalTransformation[0]));
		}
	}

	return;
}

//void cAnimationManager::Render(cRenderer* pRenderer, GLint shaderProgID) {
//	for (auto it : mpAnima) {
//	}
//	return;
//}



#pragma region MEDIATOR_COMMUNICATION
void cAnimationManager::setMediatorPointer(iMediatorInterface* pMediator) {
	this->pMediator = pMediator;
	return;
}

sData cAnimationManager::RecieveMessage(sData& data) {
	data.setResult(OK);

	return data;
}
#pragma endregion
