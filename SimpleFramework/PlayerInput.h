#pragma once
#include "Maths.h"
#include "PhysicsObject.h"
#include "Button.h"

class PhysicsProgram;

class PlayerInput
{
public:
	enum class HELD_TOOL {
		CIRCLE,
		POLYGON,
		CAPSULE,
		PLANE,
		GRAB,
		LINE,

		COUNT
	};

	PlayerInput(PhysicsProgram& program);

	void Update();
	void Render();

	void OnMouseClick(int mouseButton);
	void OnMouseRelease(int mouseButton);

	void OnKeyPressed(int key);
	void OnKeyReleased(int key);

	

	void SetHeldTool(HELD_TOOL type);
	HELD_TOOL GetHeldTool() { return heldTool; };

private:
	bool usingTool = false;
	Vector2 startingPosition = Vector2(0,0);
	PhysicsProgram& program;

	float shapeRadius = 1;
	Vector3 afterCreatedColour = Vector3(0.8f, 1.0f, 0.8f);
	Vector3 heldColour = Vector3(0.6f, 0.6f, 0.6f);
	Shape* heldShape = nullptr;

	std::vector<Button*> buttons;
	HELD_TOOL heldTool = HELD_TOOL::CIRCLE;

	PhysicsObject* grabbedObject = nullptr;
};

