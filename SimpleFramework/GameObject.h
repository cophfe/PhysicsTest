#pragma once
#include "fzx.h"
using namespace fzx;

class PhysicsProgram;

struct GameObject
{
	Vector3 colour;

	void Render(PhysicsProgram* program);
	PhysicsObject* GetPhysicsObject() { return body; }

	GameObject(const GameObject& other) = delete;
	GameObject& operator=(const GameObject& other) = delete;
private:
	GameObject(PhysicsData object, CollisionManager* manager, Vector3 colour);
	~GameObject();
	GameObject& operator=(GameObject&& other);
	GameObject(GameObject&& other);
	friend PhysicsProgram;

	typedef void(*DrawFunction)(Shape* shape, Transform& shapeTransform, Vector3 shapeColour, void* physicsProgram);
	static DrawFunction drawFunctions[4];

	//used to remove physicsObject
	CollisionManager* manager;
	PhysicsObject* body;
};

