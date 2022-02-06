#pragma once
#include "Maths.h"
#include "PhysicsObject.h"
#include "Button.h"

class PhysicsProgram;

class PlayerInput
{
public:
	PlayerInput(PhysicsProgram& program);

	void Update();
	void Render();

	void OnMouseClick(int mouseButton);
	void OnMouseRelease(int mouseButton);

	void OnKeyPressed(int key);
	void OnKeyReleased(int key);

private:
	bool holdingObject;
	bool makingObject;
	Vector2 startingPosition;
	PhysicsProgram& program;

	float shapeRadius = 1;
	Vector3 afterCreatedColour = Vector3(0.8f, 1.0f, 0.8f);
	Vector3 heldColour = Vector3(0.6f, 0.6f, 0.6f);
	Shape* heldShape;

	std::vector<Button> buttons;
};

