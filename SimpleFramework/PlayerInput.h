#pragma once
#include "Maths.h"
#include "PhysicsObject.h"
class PhysicsProgram;

class PlayerInput
{
public:
	PlayerInput(PhysicsProgram& program);

	void Update();
	void Render();

	void OnMouseClick(int mouseButton);
	void OnMouseRelease(int mouseButton);

private:
	bool holdingObject;
	Vector2 startingPosition;
	PhysicsProgram& program;

	float shapeRadius = 1;
	Shape* heldShape;
};

