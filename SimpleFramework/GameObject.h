#pragma once
#include "PhysicsObject.h"
class PhysicsProgram;

struct GameObject
{
	GameObject(PhysicsObject* object, Vector3 colour) : body(object), colour(colour) {}

	Vector3 colour;
	PhysicsObject* body;

	void Render(PhysicsProgram* program);

private:

	typedef void(*DrawFunction)(Shape* shape, Transform& shapeTransform, Vector3 shapeColour, void* physicsProgram);
	static DrawFunction drawFunctions[4];
};

