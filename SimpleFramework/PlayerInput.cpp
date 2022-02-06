#include "PlayerInput.h"
#include "PhysicsProgram.h"

PlayerInput::PlayerInput(PhysicsProgram& program) : program(program)
{
	buttons.push_back(Button(Vector2(200, 50), Vector2(300, 100), "TEXT BUTTON", Vector3(0, 0, 0), Vector3(1, 0, 0), program, 0.4f));
}

void PlayerInput::Update()
{
	for (size_t i = 0; i < buttons.size(); i++)
	{
		buttons[i].Update(program);
	}
}

float GetAngleOfVector2(Vector2 vec) 
{
	//angle between 0,1 and vector
	return atan2(-vec.x, vec.y);
}

void PlayerInput::Render()
{
	if (holdingObject) 
	{
		program.GetLineRenderer().DrawLineSegment(startingPosition, program.GetCursorPos(), Vector3(1, 0, 0));

		float angle = GetAngleOfVector2(glm::normalize(program.GetCursorPos() - startingPosition));
		heldShape->RenderShape(Transform(startingPosition, angle), program, heldColour);
	}
	else if (makingObject)
	{
		program.GetLineRenderer().DrawLineSegment(startingPosition, program.GetCursorPos(), heldColour);
	}

	for (size_t i = 0; i < buttons.size(); i++)
	{
		buttons[i].Draw(program);
	}
}

void PlayerInput::OnMouseClick(int mouseButton)
{
	if (!makingObject && !holdingObject) {
		switch (mouseButton)
		{
		case 0:
		{
			holdingObject = true;
			startingPosition = program.GetCursorPos();

			if (heldShape == nullptr)
			{
				heldShape = new CircleShape(shapeRadius, Vector2(0, 0)); //PolygonShape::GetRegularPolygonCollider(shapeRadius, 4);
			}
		}
		break;
		case 1: 
		{
			makingObject = true;
			startingPosition = program.GetCursorPos();

		}
		break;
		}
	}
	
}

void PlayerInput::OnMouseRelease(int mouseButton)
{
	if (holdingObject && mouseButton == 0) {
		holdingObject = false;
		
		auto* collider = new Collider(heldShape, 1.0f, afterCreatedColour);
		PhysicsData data = PhysicsData(
			startingPosition,
			GetAngleOfVector2(glm::normalize(program.GetCursorPos() - startingPosition )),
			true,
			true);

		heldShape = nullptr;

		program.AddPhysicsObject(PhysicsObject(data, collider)).AddImpulse(3.0f * (program.GetCursorPos() - startingPosition));
	}
	else if (makingObject && mouseButton == 1) {
		makingObject = false;
		PhysicsData data = PhysicsData(
			Vector2(0,0),
			0,
			false,
			false);
		auto* collider = new Collider(new LineShape(startingPosition, program.GetCursorPos()), 1.0f, afterCreatedColour);
		program.AddPhysicsObject(PhysicsObject(data, collider)).AddImpulse(3.0f * (program.GetCursorPos() - startingPosition));

	}
}

void PlayerInput::OnKeyPressed(int key)
{
}

void PlayerInput::OnKeyReleased(int key)
{
}
