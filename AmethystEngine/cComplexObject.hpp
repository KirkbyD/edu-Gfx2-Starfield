#pragma once
#include "cModelObject.hpp"
#include "cLightObject.hpp"
#include "iGameObject.hpp"
#include <vector>
#include <string>
#include <glm/gtx/quaternion.hpp>


#include <physics/interfaces/iPhysicsComponent.h>
#include <physics/interfaces/iBallComponent.h>
#include <physics/interfaces/iPlaneComponent.h>

#include "cActorObject.hpp"
#include "cAnimationComponent.hpp"

extern class cLightObject;

struct collisionPoint {
	glm::vec3 RelativePos;
	glm::vec3 Position;
	float Collided;
};

class cComplexObject : public iGameObject {
private:
	// Graphics
	float scale;
	// TRANSFORM MATRIX

	// Physics
	std::vector<nPhysics::iPhysicsComponent*> physicsComponents;	

	// Actors
	cActorObject* _actor;

	//Animations
	cAnimationComponent* animaComponent;

	// Members
	std::vector<glm::vec3> modelPositions;
	std::vector<cModelObject*> modelVector;
	std::vector<glm::vec3> lightPositions;
	std::vector<cLightObject*> lightVector;

	void RotateChildren();

	const static size_t System_Hex_Value = (0x04 << 16);
	const static size_t Module_Hex_Value = ((uint64_t)0x000 << 32);


public:
	
	cComplexObject(std::vector<cComplexObject*> &vObjects);
	~cComplexObject();

	void DeconstructEntities();

	float GetScale();
	void SetScale(float val);

	std::vector<cModelObject*> GetModels();
	std::vector<cLightObject*> GetLights();
	std::vector<nPhysics::iPhysicsComponent*> GetPhysicsComponents();
	cActorObject* GetActor();

	bool HasAnimations() { return (animaComponent != nullptr) ? true : false; }

	void AddLight(std::string);
	void AddLight(cLightObject* theLight);
	void AddModel(std::string);	
	void AddModel(cModelObject* theModel);
	void AddPhysicsComponent(nPhysics::iPhysicsComponent* component);
	void AddActorComponent(cActorObject* component);
	void AddAnimation(cAnimationComponent* component);

	bool QueueAnimation(std::string AnimaName);

	bool RemoveLight(cLightObject* theLight);
	bool RemovePhysicsComponent(nPhysics::iPhysicsComponent* component);

	//Apply an impulse to all member components
	void ApplyImpulse(glm::vec3 impulse);

	//Clear all existing physics components from entity (used in physics terminate)
	void ClearPhysicsComponents();

	glm::vec3 getVelocity();
	glm::vec3 getEulerAngle();
	glm::quat getQuaternion();
	glm::vec3 getFrontVector();

	virtual unsigned getUniqueID();
	virtual glm::vec3 getPosition();
};
