#pragma once
#include "PhysicsObject.h"
#include "CollisionManager.h"
class PhysicsProgram;

struct GameObject
{
	GameObject(PhysicsData object, CollisionManager* manager, Vector3 colour);

	Vector3 colour;

	void Render(PhysicsProgram* program);
	PhysicsObject* GetPhysicsObject() { return body; }

	~GameObject();
	GameObject(GameObject&& other);
	GameObject& operator=(GameObject&& other);
	GameObject(const GameObject& other) = delete;
	GameObject& operator=(const GameObject& other) = delete;
private:

	typedef void(*DrawFunction)(Shape* shape, Transform& shapeTransform, Vector3 shapeColour, void* physicsProgram);
	static DrawFunction drawFunctions[4];

	//used to remove physicsObject
	CollisionManager* manager;
	PhysicsObject* body;
};

