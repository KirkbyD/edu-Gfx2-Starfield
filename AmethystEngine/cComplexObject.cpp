#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>
#include <memory>
#include <glm/gtx/matrix_decompose.hpp>

#ifdef _DEBUG
#define DEBUG_NEW new (_NORMAL_BLOCK , __FILE__ , __LINE__)
#else
#define DBG_NEW
#endif

#include "cComplexObject.hpp"
#include "Globals.hpp"

cComplexObject::cComplexObject(std::vector<cComplexObject*> &vObjects)
	: _actor(nullptr), scale(1.f)
{
	// TODO: check to see if this can be default initialized within the interface class ctor...
	this->friendlyIDNumber = 0;
	this->friendlyName = "";
	this->parentObject = nullptr;
	this->isVisible = true;
	this->isControl = false;
	this->m_uniqueID = next_uniqueID;

	vObjects.push_back(this);
}

cComplexObject::~cComplexObject() {
	// null the actor pointer, then delete it to prevent it deleting stuff that it should not
	// TODO: investigate to see if this should also delete the thing that it is pointing too as well
	physicsComponents.clear();
	modelPositions.clear();
	modelVector.clear();
	lightPositions.clear();
	lightVector.clear();
	delete _actor;
}

void cComplexObject::DeconstructEntities() {
	_actor = nullptr;
	for (cLightObject* light : lightVector)
		light->parentObject = nullptr;

	for (cModelObject* model : modelVector)
		model->parentObject = nullptr;
}

float cComplexObject::GetScale() { return this->scale; }
void cComplexObject::SetScale(float val) { this->scale = val; }

void cComplexObject::AddLight(std::string lightName) {
	if (mpLight.find(lightName) != mpLight.end()) {
		this->lightVector.push_back(mpLight[lightName]);
		this->lightVector.back()->parentObject = this;
		this->lightVector.back()->isControl = false;
		this->lightPositions.push_back(this->lightVector.back()->getPosition());
		//this->lightVector.back()->direction = glm::vec4(mainCamera.GetFront(), 1.0f);
	}
}

void cComplexObject::AddLight(cLightObject* theLight) {
	this->lightVector.push_back(theLight);
	this->lightVector.back()->parentObject = this;
	this->lightVector.back()->isControl = false;
	this->lightPositions.push_back(theLight->getPosition());
	//this->lightVector.back()->direction = glm::vec4(mainCamera.GetFront(), 1.0f);
}

void cComplexObject::AddModel(std::string friendlyName) {
	cModelObject* theModel = pFindObjectByFriendlyName(friendlyName);
	this->modelVector.push_back(theModel);
	this->modelVector.back()->parentObject = this;
	this->modelVector.back()->isControl = false;
}

void cComplexObject::AddModel(cModelObject* theModel) {
	this->modelVector.push_back(theModel);
	this->modelVector.back()->parentObject = this;
	this->modelVector.back()->isControl = false;
}

void cComplexObject::AddPhysicsComponent(nPhysics::iPhysicsComponent* component) {
	this->physicsComponents.push_back(component); 
	component->SetParentID(this->friendlyIDNumber);
	component->SetParentPointer(this);
}

void cComplexObject::AddActorComponent(cActorObject* component) { this->_actor = component; }

void cComplexObject::AddAnimation(cAnimationComponent* component) { 
	component->parentObject = this;
	component->parentIDNumber = this->friendlyIDNumber;
	this->animaComponent = component;
}

bool cComplexObject::QueueAnimation(std::string AnimaName) {
	if (HasAnimations()) {
		return animaComponent->QueueAnimation(AnimaName);
	}
}

bool cComplexObject::RemoveLight(cLightObject* theLight){
	for (size_t i = 0; i < lightVector.size(); i++)	{
		if (lightVector[i] = theLight) {
			lightPositions.erase(lightPositions.begin() + (i));
			lightVector.erase(lightVector.begin() + (i));
			return true;
		}
	}
	return false;
}

bool cComplexObject::RemovePhysicsComponent(nPhysics::iPhysicsComponent* component) {
	for (auto it = this->physicsComponents.begin();
		it != physicsComponents.end();
		it++) {
		if (component == *it) {
			physicsComponents.erase(it);
			return true;
		}
	}
	return false;
}

void cComplexObject::ApplyImpulse(glm::vec3 impulse) {
	for (auto component : this->physicsComponents) {
		switch (component->GetComponentType()) {
		case nPhysics::eComponentType::ball:
			((nPhysics::iBallComponent*)component)->ApplyImpulse(impulse);
		}
	}
}

void cComplexObject::ClearPhysicsComponents() {
	this->physicsComponents.clear();
}

unsigned cComplexObject::getUniqueID() { return this->m_uniqueID; }

glm::vec3 cComplexObject::getPosition() {
	glm::mat4 transform;
	this->physicsComponents[0]->GetTransform(transform);
	return transform[3];
}

glm::vec3 cComplexObject::getVelocity() {
	glm::vec3 vel(0.f);
	if (this->physicsComponents.size() != 0) {
		this->physicsComponents[0]->GetVelocity(vel);
	}
	return vel;
}

glm::vec3 cComplexObject::getEulerAngle() {
	return glm::eulerAngles(getQuaternion());
}

glm::quat cComplexObject::getQuaternion() {
	glm::mat4 transformation(1.0);
	if(this->physicsComponents.size() != 0)
		this->physicsComponents[0]->GetTransform(transformation);

	//decompose matrix to get quaternion!
	glm::vec3 scale;
	glm::quat rotation;
	glm::vec3 translation;
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::decompose(transformation, scale, rotation, translation, skew, perspective);

	rotation = glm::conjugate(rotation);

	return rotation;
}

glm::vec3 cComplexObject::getFrontVector() {
	return glm::vec3(0.f, 0.f, 1.f) * this->getQuaternion();
}

std::vector<cModelObject*> cComplexObject::GetModels() { return this->modelVector; }
std::vector<cLightObject*> cComplexObject::GetLights() { return this->lightVector; }

std::vector<nPhysics::iPhysicsComponent*> cComplexObject::GetPhysicsComponents() { return this->physicsComponents; }
cActorObject* cComplexObject::GetActor() { return this->_actor; }
