#include "PlayerInput.h"
#include "PhysicsProgram.h"
static const Vector3 disabledColour = Vector3(0.3f, 0.15f, 0.3f);

static void SwitchToCircle(Button& button, void* infoPointer) {
	PlayerInput* playerInput = (PlayerInput*)infoPointer;
	playerInput->SetHeldTool(PlayerInput::HELD_TOOL::CIRCLE);
	button.DisableButton(disabledColour);
}
static void SwitchToPolygon(Button& button, void* infoPointer) {
	PlayerInput* playerInput = (PlayerInput*)infoPointer;
	playerInput->SetHeldTool(PlayerInput::HELD_TOOL::POLYGON);
	button.DisableButton(disabledColour);
}
static void SwitchToCapsule(Button& button, void* infoPointer) {
	PlayerInput* playerInput = (PlayerInput*)infoPointer;
	playerInput->SetHeldTool(PlayerInput::HELD_TOOL::CAPSULE);
	button.DisableButton(disabledColour);
}
static void SwitchToPlane(Button& button, void* infoPointer) {
	PlayerInput* playerInput = (PlayerInput*)infoPointer;
	playerInput->SetHeldTool(PlayerInput::HELD_TOOL::PLANE);
	button.DisableButton(disabledColour);
}
static void SwitchToLineTool(Button& button, void* infoPointer)
{
	PlayerInput* playerInput = (PlayerInput*)infoPointer;
	playerInput->SetHeldTool(PlayerInput::HELD_TOOL::LINE);
	button.DisableButton(disabledColour);
}
static void SwitchToGrabTool(Button& button, void* infoPointer)
{
	PlayerInput* playerInput = (PlayerInput*)infoPointer;
	playerInput->SetHeldTool(PlayerInput::HELD_TOOL::GRAB);
	button.DisableButton(disabledColour);
}

PlayerInput::PlayerInput(PhysicsProgram& program) : program(program)
{
	const Vector2 size = Vector2(100, 30);
	const Vector2 startPos = Vector2(65, 30);
	const Vector2 offset = Vector2(110, 0);
	const Vector3 textColour(0,0,0);
	const Vector3 backgroundColour(0.8f, 0.5f, 0.8f);
	const UIObject::ANCHOR_POINT anchor = UIObject::ANCHOR_POINT::TOP_RIGHT;

	int i = 0;
	Button* newButton = (Button*)program.AddUIObject(new Button(size, (float)i * offset + startPos, anchor, "Grab", textColour, backgroundColour, program, 0, 8));
	buttons.push_back(newButton);
	buttons[i]->SetOnClick(SwitchToGrabTool, this);
	i++;
	newButton = (Button*)program.AddUIObject(new Button(size, (float)i * offset + startPos, anchor, "Line", textColour, backgroundColour, program, 0, 8));
	buttons.push_back(newButton);
	buttons[i]->SetOnClick(SwitchToLineTool, this);
	i++;
	newButton = (Button*)program.AddUIObject(new Button(size, (float)i * offset + startPos, anchor, "Plane", textColour, backgroundColour, program, 0, 8));
	buttons.push_back(newButton);
	buttons[i]->SetOnClick(SwitchToPlane, this);
	i++;
	newButton = (Button*)program.AddUIObject(new Button(size, (float)i * offset + startPos, anchor, "Capsule", textColour, backgroundColour, program, 0, 8));
	buttons.push_back(newButton);
	buttons[i]->SetOnClick(SwitchToCapsule, this);
	i++;
	newButton = (Button*)program.AddUIObject(new Button(size, (float)i * offset + startPos, anchor, "Polygon", textColour, backgroundColour, program, 0, 8));
	buttons.push_back(newButton);
	buttons[i]->SetOnClick(SwitchToPolygon, this);
	i++;																
	newButton = (Button*)program.AddUIObject(new Button(size, (float)i * offset + startPos, anchor, "Circle", textColour, backgroundColour, program, 0, 8));
	buttons.push_back(newButton);																											  			  
	buttons[i]->SetOnClick(SwitchToCircle, this);		
	newButton->DisableButton(disabledColour);
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

		switch (heldTool)
		{
		case HELD_TOOL::CIRCLE: //pass through
		case HELD_TOOL::POLYGON:
		{
			if (heldShape == nullptr) break;

			program.GetLineRenderer().DrawLineSegment(startingPosition, program.GetCursorPos(), Vector3(1, 0, 0));

			float angle = GetAngleOfVector2(glm::normalize(program.GetCursorPos() - startingPosition));
			heldShape->RenderShape(Transform(startingPosition, angle), program, heldColour);
			break;
		}
		case HELD_TOOL::LINE:
			program.GetLineRenderer().DrawLineSegment(startingPosition, program.GetCursorPos(), heldColour);
			break;
		case HELD_TOOL::CAPSULE:
			auto t = Transform(startingPosition, 0);
			CapsuleShape(startingPosition, program.GetCursorPos(), shapeRadius)
				.RenderShape(program, t, heldColour);
			break;
		case HELD_TOOL::GRAB:
			if (grabbedObject != nullptr)
			{
				Vector2 startPos = grabbedObject->GetTransform().TransformPoint(startingPosition);
				program.GetLineRenderer().DrawLineSegment(startPos, program.GetCursorPos(), heldColour);
				grabbedObject->AddForceAtPosition(10.0f * (program.GetCursorPos() - startPos), startPos);
			}
			break;
		}
		
	}
}

void PlayerInput::OnMouseClick(int mouseButton)
{
	if (!usingTool) {
		switch (mouseButton)
		{
		case 0:
		{
			if (!program.GetUIHeldDown())
			{
				usingTool = true;
				startingPosition = program.GetCursorPos();

				switch (heldTool)
				{
				case HELD_TOOL::CIRCLE:
					heldShape = new CircleShape(shapeRadius, Vector2(0, 0)); //PolygonShape::GetRegularPolygonCollider(shapeRadius, 4);
					break;
				case HELD_TOOL::POLYGON:
					heldShape = PolygonShape::GetRegularPolygonCollider(shapeRadius, 4);
					break;
				case HELD_TOOL::PLANE:
					heldShape = new PlaneShape(Vector2(1,0), glm::dot(startingPosition, Vector2(1,0)));
					break;
				case HELD_TOOL::GRAB:
					//grab tool startingPosition will be relative to physicsObject transform, so it can be updated over time
					//do something like program.GetCollisionManager().PointCast(startingPosition) to get an object under the cursor
					break;
				}
			}
		}
		break;
		case 1: 
		{
			//startingPosition = program.GetCursorPos();

		}
		break;
		}
	}
	
}

void PlayerInput::OnMouseRelease(int mouseButton)
{
	switch (mouseButton)
	{
	case 0:
	{
		if (usingTool)
		{
			usingTool = false;

			switch (heldTool)
			{
			case HELD_TOOL::CIRCLE: //pass through
			case HELD_TOOL::POLYGON:
			{
				auto* collider = new Collider(heldShape, 1.0f, afterCreatedColour);
				PhysicsData data = PhysicsData(
					startingPosition,
					GetAngleOfVector2(glm::normalize(program.GetCursorPos() - startingPosition)),
					true,
					true);

				heldShape = nullptr;

				program.AddPhysicsObject(PhysicsObject(data, collider)).AddImpulse(8.0f * (program.GetCursorPos() - startingPosition));
			}
			break;
			case HELD_TOOL::LINE:
			{
				PhysicsData data = PhysicsData(
					Vector2(0, 0),
					0,
					false,
					false);
				auto* collider = new Collider(new CapsuleShape(startingPosition, program.GetCursorPos()), 1.0f, afterCreatedColour);
				program.AddPhysicsObject(PhysicsObject(data, collider)).AddImpulse(3.0f * (program.GetCursorPos() - startingPosition));
			}
			break;
			case HELD_TOOL::CAPSULE:
			{
				PhysicsData data = PhysicsData(
					Vector2(0, 0),
					0,
					false,
					false);
				auto* collider = new Collider(new CapsuleShape(startingPosition, program.GetCursorPos(), shapeRadius), 1.0f, afterCreatedColour);
				program.AddPhysicsObject(PhysicsObject(data, collider)).AddImpulse(3.0f * (program.GetCursorPos() - startingPosition));
			}
			break;
			
			}
		}
			
		break;
	}
	case 1:
	{
		
		break;
	}
	}
}

void PlayerInput::SetHeldTool(HELD_TOOL type)
{
	if (heldShape != nullptr)
	{
		delete heldShape;
		heldShape = nullptr;
	}
	usingTool = false;

	this->heldTool = type;
	for (size_t i = 0; i < buttons.size(); i++)
	{
		buttons[i]->EnableButton();
	}
}

void PlayerInput::OnKeyPressed(int key)
{
}

void PlayerInput::OnKeyReleased(int key)
{

}
