#include "PlayerInput.h"
#include "PhysicsProgram.h"
static const Vector3 disabledColour = Vector3(0.3f, 0.3f, 0.3f);

#pragma region SHAPE TOOL BUTTON FUNCTIONS
static void SwitchShapeTool(Button& button, void* infoPointer, PlayerInput::HELD_SHAPE_TOOL tool)
{
	PlayerInput* playerInput = (PlayerInput*)infoPointer;
	playerInput->SetHeldShapeTool(tool);
	button.DisableButton(disabledColour);
}

static void SwitchToCircle(Button& button, void* infoPointer) {
	SwitchShapeTool(button, infoPointer, PlayerInput::HELD_SHAPE_TOOL::CIRCLE);
}
static void SwitchToPolygon(Button& button, void* infoPointer) {
	SwitchShapeTool(button, infoPointer, PlayerInput::HELD_SHAPE_TOOL::POLYGON);
}
static void SwitchToCapsule(Button& button, void* infoPointer) {
	SwitchShapeTool(button, infoPointer, PlayerInput::HELD_SHAPE_TOOL::CAPSULE);
}
static void SwitchToPlane(Button& button, void* infoPointer) {
	SwitchShapeTool(button, infoPointer, PlayerInput::HELD_SHAPE_TOOL::PLANE);
}
static void SwitchToLineTool(Button& button, void* infoPointer){
	SwitchShapeTool(button, infoPointer, PlayerInput::HELD_SHAPE_TOOL::LINE);
}
#pragma endregion

#pragma region MODIFIER TOOL BUTTON FUNCTIONS
static void SwitchModifierTool(Button& button, void* infoPointer, PlayerInput::HELD_MODIFIER_TOOL tool)
{
	PlayerInput* playerInput = (PlayerInput*)infoPointer;
	playerInput->SetHeldModifierTool(tool);
	button.DisableButton(disabledColour);
}

static void SwitchToGrabTool(Button& button, void* infoPointer){
	SwitchModifierTool(button, infoPointer, PlayerInput::HELD_MODIFIER_TOOL::GRAB);
}
static void SwitchToLaunchTool(Button& button, void* infoPointer) {
	SwitchModifierTool(button, infoPointer, PlayerInput::HELD_MODIFIER_TOOL::LAUNCH);
}
static void SwitchToDeleteTool(Button& button, void* infoPointer) {
	SwitchModifierTool(button, infoPointer, PlayerInput::HELD_MODIFIER_TOOL::DELETE);
}
static void SwitchToRotateTool(Button& button, void* infoPointer) {
	SwitchModifierTool(button, infoPointer, PlayerInput::HELD_MODIFIER_TOOL::ROTATE);
}
#pragma endregion

#pragma region OTHER UI FUNCTIONS

static void ClearPhysicsObjects(Button& button, void* infoPointer)
{
	PhysicsProgram* program = (PhysicsProgram*)infoPointer;
	program->ResetPhysics();
}

//static bool updateCollision = false;

static void PauseUnpause(Button& button, void* infoPointer)
{
	PhysicsProgram* program = (PhysicsProgram*)infoPointer;

	bool paused = program->GetPauseState();

	if (paused)
	{
		button.colour = Vector3(0, 0, 0);
		button.textColour = Vector3(1, 1, 1);
		button.colourOnHover = Vector3(0.2f, 0.2f, 0.2f);
		button.colourOnClick = Vector3(0.4f, 0.4f, 0.4f);
	}
	else {
		//updateCollision = false;
		button.colour = Vector3(1, 1, 1);
		button.textColour = Vector3(0, 0, 0);
		button.colourOnHover = Vector3(0.8f, 0.8f, 0.8f);
		button.colourOnClick = Vector3(0.6f, 0.6f, 0.6f);
	}

	program->GetPlayerInput().GetStepForwardButton()->SetEnabled(!paused);
	program->SetPauseState(!paused);
}

static void StepOnce(Button& button, void* infoPointer)
{
	PhysicsProgram* program = (PhysicsProgram*)infoPointer;

	//if (!updateCollision)
	program->UpdatePhysics();
	//else
	program->ResolveCollisions();

	//updateCollision = !updateCollision;
}

static void SpeedUnspeed(Button& button, void* infoPointer)
{
	static bool capped = false;
	PhysicsProgram* program = (PhysicsProgram*)infoPointer;
	program->CapFPS(capped);

	if (capped)
	{
		button.colour = Vector3(0, 0, 0);
		button.textColour = Vector3(1, 1, 1);
		button.colourOnHover = Vector3(0.2f, 0.2f, 0.2f);
		button.colourOnClick = Vector3(0.4f, 0.4f, 0.4f);
	}
	else {
		button.colour = Vector3(1, 1, 1);
		button.textColour = Vector3(0, 0, 0);
		button.colourOnHover = Vector3(0.8f, 0.8f, 0.8f);
		button.colourOnClick = Vector3(0.6f, 0.6f, 0.6f);
	}

	capped = !capped;
}
static void RadiusChanged(Slider& slider, void* infoPointer, float value)
{
	PlayerInput* playerInput = (PlayerInput*)infoPointer;
	playerInput->SetShapeRadius(value);
}
#pragma endregion

PlayerInput::PlayerInput(PhysicsProgram& program) : program(program)
{
	const Vector2 size = Vector2(70, 30);
	Vector2 startPos = Vector2(45, 30);
	const Vector2 offset = Vector2(0, 30);
	const Vector3 textColour(1,1,1);
	const Vector3 backgroundColour(0, 0, 0);
	Vector3 edgeColour(1, 1, 1);
	const UIObject::ANCHOR_POINT anchor = UIObject::ANCHOR_POINT::TOP_RIGHT;

	
	Button* speedUpButton = (Button*)program.AddUIObject(new Button(Vector2(30, 30), Vector2(30, 110), UIObject::ANCHOR_POINT::TOP_LEFT, ">>", textColour, backgroundColour, program, 0, 8, edgeColour));
	speedUpButton->SetOnClick(SpeedUnspeed, &program);

	Button* pauseButton = (Button*)program.AddUIObject(new Button(Vector2(30, 30), Vector2(30, 70), UIObject::ANCHOR_POINT::TOP_LEFT, "I I", textColour, backgroundColour, program, 0, 8, edgeColour));
	pauseButton->SetOnClick(PauseUnpause, &program);
	stepForwardButton = (Button*)program.AddUIObject(new Button(Vector2(30, 30), Vector2(70, 70), UIObject::ANCHOR_POINT::TOP_LEFT, ">", textColour, backgroundColour, program, 0, 8, edgeColour));
	stepForwardButton->SetEnabled(false);
	stepForwardButton->SetOnClick(StepOnce, &program);

	Button* clearButton= (Button*)program.AddUIObject(new Button(size, Vector2(50, 30), UIObject::ANCHOR_POINT::TOP_LEFT, "Clear", textColour, Vector3(0.9f, 0.1f, 0.1f), program, 0, 8, edgeColour));
	clearButton->SetOnClick(ClearPhysicsObjects, &program);

	edgeColour = Vector3(0, 1, 0);
	// SHAPE TOOL BUTTONS
	int i = 0;
	Button* newButton = (Button*)program.AddUIObject(new Button(size, (float)i * offset + startPos, anchor, "Circle", textColour, backgroundColour, program, 0, 8, edgeColour));
	shapeButtons.push_back(newButton);
	shapeButtons[i]->SetOnClick(SwitchToCircle, this);
	newButton->DisableButton(disabledColour);
	i++;
	newButton = (Button*)program.AddUIObject(new Button(size, (float)i * offset + startPos, anchor, "Polygon", textColour, backgroundColour, program, 0, 8, edgeColour));
	shapeButtons.push_back(newButton);
	shapeButtons[i]->SetOnClick(SwitchToPolygon, this);
	i++;
	newButton = (Button*)program.AddUIObject(new Button(size, (float)i * offset + startPos, anchor, "Capsule", textColour, backgroundColour, program, 0, 8, edgeColour));
	shapeButtons.push_back(newButton);
	shapeButtons[i]->SetOnClick(SwitchToCapsule, this);
	i++;
	newButton = (Button*)program.AddUIObject(new Button(size, (float)i * offset + startPos, anchor, "Line", textColour, backgroundColour, program, 0, 8, edgeColour));
	shapeButtons.push_back(newButton);
	shapeButtons[i]->SetOnClick(SwitchToLineTool, this);
	/*i++;
	newButton = (Button*)program.AddUIObject(new Button(size, (float)i * offset + startPos, anchor, "Plane", textColour, backgroundColour, program, 0, 8, edgeColour));
	buttons.push_back(newButton);
	buttons[i]->SetOnClick(SwitchToPlane, this);*/

	i++;
	Slider* radiusSlider = (Slider*)program.AddUIObject(
		new Slider(Vector2(145, 30), anchor, (float)i * offset + startPos + Vector2(36.25f + 2, 5), 0.1f, 10.0f, 1.0f, backgroundColour, textColour * 0.3f, program
			, 4, true, true, "Radius: ", textColour));
	radiusSlider->SetOnValueChangedCallback(RadiusChanged, this);

	edgeColour = Vector3(0, 0, 1);
	startPos.x += size.x + 5;
	// MODIFIER TOOL BUTTONS
	i = 0;
	newButton = (Button*)program.AddUIObject(new Button(size, (float)i * offset + startPos, anchor, "Launch", textColour, backgroundColour, program, 0, 8, edgeColour));
	modifierButtons.push_back(newButton);
	modifierButtons[i]->SetOnClick(SwitchToLaunchTool, this);
	newButton->DisableButton(disabledColour);
	i++;
	newButton = (Button*)program.AddUIObject(new Button(size, (float)i * offset + startPos, anchor, "Grab", textColour, backgroundColour, program, 0, 8, edgeColour));
	modifierButtons.push_back(newButton);
	modifierButtons[i]->SetOnClick(SwitchToGrabTool, this);
	i++;
	newButton = (Button*)program.AddUIObject(new Button(size, (float)i * offset + startPos, anchor, "Rotate", textColour, backgroundColour, program, 0, 8, edgeColour));
	modifierButtons.push_back(newButton);
	modifierButtons[i]->SetOnClick(SwitchToRotateTool, this);
	i++;
	newButton = (Button*)program.AddUIObject(new Button(size, (float)i * offset + startPos, anchor, "Delete", textColour, backgroundColour, program, 0, 8, edgeColour));
	modifierButtons.push_back(newButton);
	modifierButtons[i]->SetOnClick(SwitchToDeleteTool, this);
	
	
	

}

void PlayerInput::Update()
{
	
}

float GetAngleOfVector2(Vector2 vec) 
{
	//angle between 0,1 and vector
	return atan2(-vec.x, vec.y);
}

void PlayerInput::Render()
{
	if (usingTool)
	{
		if (isToolTypeShape)
		{
			switch (heldShapeTool)
			{
				//pass through
			case HELD_SHAPE_TOOL::CIRCLE:
			case HELD_SHAPE_TOOL::POLYGON:
			case HELD_SHAPE_TOOL::PLANE:
			{
				if (heldShape == nullptr) break;

				program.GetLineRenderer().DrawLineSegment(startingPosition, program.GetCursorPos(), Vector3(1, 0, 0));

				float angle = GetAngleOfVector2(glm::normalize(program.GetCursorPos() - startingPosition));

				heldShape->RenderShape(Transform(startingPosition, angle), program, heldColour);
				break;
			}
			case HELD_SHAPE_TOOL::LINE:
				program.GetLineRenderer().DrawLineSegment(startingPosition, program.GetCursorPos(), heldColour);
				break;
			case HELD_SHAPE_TOOL::CAPSULE:
				auto t = Transform(Vector2(0, 0), 0);
				CapsuleShape(startingPosition, program.GetCursorPos(), shapeRadius)
					.RenderShape(program, t, heldColour);
				break;
			}
		}
		else
		{
			switch (heldModifierTool)
			{
			case HELD_MODIFIER_TOOL::LAUNCH:
				break;
			case HELD_MODIFIER_TOOL::GRAB:
				if (grabbedObject != nullptr)
				{
					Vector2 startPos = grabbedObject->GetTransform().TransformPoint(startingPosition);
					program.GetLineRenderer().DrawLineSegment(startPos, program.GetCursorPos(), heldColour);
					grabbedObject->AddForceAtPosition(10.0f * (program.GetCursorPos() - startPos), startPos);
				}
				break;
			case HELD_MODIFIER_TOOL::ROTATE:
				break;
			case HELD_MODIFIER_TOOL::DELETE:
				break;
			}
		}
		
	}
}

void PlayerInput::OnMouseClick(int mouseButton)
{
	switch (mouseButton)
	{
	case 1:
	{
		if (!usingTool) {
			if (!program.GetUIHeldDown())
			{
				isToolTypeShape = true;
				usingTool = true;
				startingPosition = program.GetCursorPos();

				switch (heldShapeTool)
				{
				case HELD_SHAPE_TOOL::CIRCLE:
					heldShape = new CircleShape(shapeRadius, Vector2(0, 0));
					break;
				case HELD_SHAPE_TOOL::POLYGON:
					heldShape = PolygonShape::GetRegularPolygonCollider(shapeRadius, 4);
					break;
				case HELD_SHAPE_TOOL::PLANE:
					//it is at 0,0 because the shape position is relative to the transform position
					heldShape = new PlaneShape(Vector2(0, 1), 0);
					break;
				}
			}
		}
	}
	break;
	case 0:
	{
		leftClickHeld = true;
		if (!usingTool) {
			if (!program.GetUIHeldDown())
			{
				isToolTypeShape = false;
				usingTool = true;
				startingPosition = program.GetCursorPos();

				switch (heldModifierTool)
				{
				case HELD_MODIFIER_TOOL::LAUNCH:
					break;
				case HELD_MODIFIER_TOOL::GRAB:
					//grab tool startingPosition will be relative to physicsObject transform, so it can be updated over time
					//do something like program.GetCollisionManager().PointCast(startingPosition) to get an object under the cursor
					grabbedObject = program.GetObjectUnderPoint(startingPosition, false);
					if (grabbedObject == nullptr)
					{
						usingTool = false;
						return;
					}
					startingPosition = grabbedObject->GetTransform().InverseTransformPoint(startingPosition);
					break;
				case HELD_MODIFIER_TOOL::ROTATE:
					break;
				case HELD_MODIFIER_TOOL::DELETE:
					break;
				}
			}
		}
	}
	break;
	}
	
}

void PlayerInput::OnMouseRelease(int mouseButton)
{
	switch (mouseButton)
	{
	case 1:
	{
		if (usingTool && isToolTypeShape)
		{
			usingTool = false;

			switch (heldShapeTool)
			{
			case HELD_SHAPE_TOOL::PLANE:
			case HELD_SHAPE_TOOL::CIRCLE: //pass through
			case HELD_SHAPE_TOOL::POLYGON:
			{
				auto* collider = new Collider(heldShape, 1.0f, afterCreatedColour);
				PhysicsData data = PhysicsData(
					startingPosition,
					GetAngleOfVector2(glm::normalize(program.GetCursorPos() - startingPosition)),
					true,
					true);

				heldShape = nullptr;

				program.AddPhysicsObject(PhysicsObject(data, collider)).AddVelocity( (program.GetCursorPos() - startingPosition));
			}
			break;
			case HELD_SHAPE_TOOL::LINE:
			{
				PhysicsData data = PhysicsData(
					Vector2(0, 0),
					0,
					false,
					false);
				auto* collider = new Collider(new CapsuleShape(startingPosition, program.GetCursorPos()), 1.0f, afterCreatedColour);
				program.AddPhysicsObject(PhysicsObject(data, collider));
			}
			break;
			case HELD_SHAPE_TOOL::CAPSULE:
			{
				Vector2 centrePos = 0.5f * (startingPosition + program.GetCursorPos());
				Vector2 delta = (startingPosition - program.GetCursorPos());
				float distance = 0.5f * glm::length(delta);

				PhysicsData data = PhysicsData(
					centrePos,
					GetAngleOfVector2(delta),
					true,
					true);
				Collider* collider;
				if (delta.x == 52 && delta.y == 0) //capsules NEED pA to be different from pB or they will not collide properly
				{
					collider = new Collider(new CircleShape(shapeRadius, Vector2(0,0)), 1.0f, afterCreatedColour);
				}
				else {
					collider = new Collider(new CapsuleShape(Vector2(0, distance), -Vector2(0, distance), shapeRadius), 1.0f, afterCreatedColour);
				}

				
				program.AddPhysicsObject(PhysicsObject(data, collider));
			}
			break;
			
			}
		}
			
		break;
	}
	case 0:
	{
		leftClickHeld = false;

		if (usingTool)
		{
			if (isToolTypeShape)
			{
				OnMouseRelease(1);
				break;
			}
			usingTool = false;
			switch (heldModifierTool)
			{
			case HELD_MODIFIER_TOOL::LAUNCH:
				break;
			case HELD_MODIFIER_TOOL::GRAB:
				break;
			case HELD_MODIFIER_TOOL::ROTATE:
				break;
			case HELD_MODIFIER_TOOL::DELETE:
				break;
			}
		}

		break;
	}
	}
}

void PlayerInput::SetHeldShapeTool(HELD_SHAPE_TOOL type)
{
	if (heldShape != nullptr)
	{
		delete heldShape;
		heldShape = nullptr;
	}
	usingTool = false;

	this->heldShapeTool = type;
	for (size_t i = 0; i < shapeButtons.size(); i++)
	{
		shapeButtons[i]->EnableButton();
	}
}

void PlayerInput::SetHeldModifierTool(HELD_MODIFIER_TOOL type)
{
	if (heldShape != nullptr)
	{
		delete heldShape;
		heldShape = nullptr;
	}
	usingTool = false;

	this->heldModifierTool = type;
	for (size_t i = 0; i < modifierButtons.size(); i++)
	{
		modifierButtons[i]->EnableButton();
	}
}

void PlayerInput::SetShapeRadius(float rad)
{
	shapeRadius = rad;
}

void PlayerInput::OnKeyPressed(int key)
{
}

void PlayerInput::OnKeyReleased(int key)
{
	if (key == GLFW_KEY_ESCAPE)
	{
		SetHeldShapeTool(heldShapeTool);
		SetHeldModifierTool(heldModifierTool);
	}
}
