#pragma once
#include "Maths.h"
#include "PhysicsObject.h"
#include "Button.h"
#include "Slider.h"

class PhysicsProgram;

class PlayerInput
{
public:
	enum class HELD_MODIFIER_TOOL {
		LAUNCH,
		GRAB,
		ROTATE,
		DELETE,
		COUNT
	};
	enum class HELD_SHAPE_TOOL {
		CIRCLE,
		POLYGON,
		CAPSULE,
		PLANE,
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

	void SetShapeRadius(float rad);
	
	void SetHeldShapeTool(HELD_SHAPE_TOOL type);
	HELD_SHAPE_TOOL GetHeldShapeTool() { return heldShapeTool; };

	void SetHeldModifierTool(HELD_MODIFIER_TOOL type);
	HELD_MODIFIER_TOOL GetHeldModifierTool() { return heldModifierTool; };

	Button* GetStepForwardButton() { return stepForwardButton; };
private:
	bool usingTool = false;
	bool isToolTypeShape = true;
	bool leftClickHeld = false;
	Vector2 startingPosition = Vector2(0,0);
	PhysicsProgram& program;

	float shapeRadius = 1;
	Vector3 afterCreatedColour = Vector3(0.8f, 1.0f, 0.8f);
	Vector3 heldColour = Vector3(0.6f, 0.6f, 0.6f);
	Shape* heldShape = nullptr;

	std::vector<Button*> shapeButtons;
	std::vector<Button*> modifierButtons;
	Button* stepForwardButton;
	HELD_SHAPE_TOOL heldShapeTool = HELD_SHAPE_TOOL::CIRCLE;
	HELD_MODIFIER_TOOL heldModifierTool = HELD_MODIFIER_TOOL::LAUNCH;

	PhysicsObject* grabbedObject = nullptr;
};

