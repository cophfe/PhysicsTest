#pragma once
#include "Maths.h"
#include "GameObject.h"
#include "Button.h"
#include "Slider.h"

class PhysicsProgram;

//This class has the messiest code ever made, but since it's not part of the physics library I put a minimal amount of effort into making it 'good'.
//it is reeeeeaal messy tho

class PlayerInput
{
public:
	enum class HELD_MODIFIER_TOOL {
		LAUNCH,
		GRAB,
		ROTATE,
		DELETE,
		TRANSLATE,
		MERGE,
		COUNT
	};
	enum class HELD_SHAPE_TOOL {
		CIRCLE,
		POLYGON,
		CAPSULE,
		PLANE,
		LINE,
		CREATEPOLYGON,
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
	void SetPolygonPoints(int pointCount) { polygonPointCount = std::max(3, pointCount); };
	
	void SetHeldShapeTool(HELD_SHAPE_TOOL type);
	HELD_SHAPE_TOOL GetHeldShapeTool() { return heldShapeTool; };

	void SetHeldModifierTool(HELD_MODIFIER_TOOL type);
	HELD_MODIFIER_TOOL GetHeldModifierTool() { return heldModifierTool; };

	Button* GetStepForwardButton() { return stepForwardButton; };

	~PlayerInput();
	//PlayerInput(const PlayerInput& other) = delete;
	//PlayerInput& operator= (const PlayerInput& other) = delete;
private:
	bool usingTool = false;
	bool isToolTypeShape = true;
	bool leftClickHeld = false;
	Vector2 startingPosition = Vector2(0,0);
	PhysicsProgram& program;

	float shapeRadius = 1;
	int polygonPointCount = 4;
	Vector3 afterCreatedColour = Vector3(0.8f, 1.0f, 0.8f);
	Vector3 highlightedColour = Vector3(1.0f, 0.6f, 0.6f);
	Vector3 heldColour = Vector3(0.6f, 0.6f, 0.6f);
	Shape* heldShape = nullptr;

	//UI Object references
	std::vector<Button*> shapeButtons;
	std::vector<Button*> modifierButtons;
	Button* stepForwardButton;
	Button* pauseButton;
	Button* speedUpButton;
	Slider* polygonSlider;
	Slider* radiusSlider;
	Button* cancelPolyButton;
	Button* createPolyButton;
	//used for launch
	float lastMass;
	float lastInertia;
	std::vector<Vector2> customPolyPoints;
	bool isCreatingPolygon = false;
	HELD_SHAPE_TOOL heldShapeTool = HELD_SHAPE_TOOL::CIRCLE;
	HELD_MODIFIER_TOOL heldModifierTool = HELD_MODIFIER_TOOL::GRAB;

	GameObject* highlighted = nullptr;
	GameObject* secondHighlighted = nullptr; //pretty much just used for merge tool
	PhysicsObject* heldObject = nullptr;

	static void SwitchToCircle(Button& button, void* infoPointer);
	static void SwitchToPolygon(Button& button, void* infoPointer);
	static void SwitchToCapsule(Button& button, void* infoPointer);
	static void SwitchToPlane(Button& button, void* infoPointer);
	static void SwitchToLineTool(Button& button, void* infoPointer);
	static void SwitchToCreatePolygonTool(Button& button, void* infoPointer);

	static void SwitchToGrabTool(Button& button, void* infoPointer);
	static void SwitchToLaunchTool(Button& button, void* infoPointer);
	static void SwitchToTranslateTool(Button& button, void* infoPointer);
	static void SwitchToMergeTool(Button& button, void* infoPointer);
	static void SwitchToDeleteTool(Button& button, void* infoPointer);
	static void SwitchToRotateTool(Button& button, void* infoPointer);
	
	static void SpeedUnspeed(Button& button, void* infoPointer);
	static void StepOnce(Button& button, void* infoPointer);
	static void PauseUnpause(Button& button, void* infoPointer);
	static void ClearPhysicsObjects(Button& button, void* infoPointer);

	static void CancelCreatePolygon(Button& button, void* infoPointer);
	static void FinishCreatePolygon(Button& button, void* infoPointer);

	static void RadiusChanged(Slider& slider, void* infoPointer, float value);
	static void PolygonPointCountChanged(Slider& slider, void* infoPointer, float value);
};

