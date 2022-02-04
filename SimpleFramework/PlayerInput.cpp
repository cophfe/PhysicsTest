#include "PlayerInput.h"
#include "PhysicsProgram.h"

PlayerInput::PlayerInput(PhysicsProgram& program) : program(program)
{
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
	if (holdingObject) 
	{
		program.GetLineRenderer().DrawLineSegment(startingPosition, program.GetCursorPos(), Vector3(1, 0, 0));

		float angle = GetAngleOfVector2(glm::normalize(program.GetCursorPos() - startingPosition));
		heldShape->RenderShape(Transform(startingPosition, angle), program);
	}
	else if (makingObject)
	{
		program.GetLineRenderer().DrawLineSegment(startingPosition, program.GetCursorPos(), Vector3(1, 0, 0));
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
		
		auto* collider = new Collider(heldShape);
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
		auto* collider = new Collider(new LineShape(startingPosition, program.GetCursorPos()));
		program.AddPhysicsObject(PhysicsObject(data, collider)).AddImpulse(3.0f * (program.GetCursorPos() - startingPosition));

	}
}
