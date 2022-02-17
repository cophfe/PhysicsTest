#include "GameObject.h"
#include "PhysicsProgram.h"

GameObject::DrawFunction GameObject::drawFunctions[4] =
{
	PhysicsProgram::DrawCircle,
	PhysicsProgram::DrawPolygon,
	PhysicsProgram::DrawCapsule,
	PhysicsProgram::DrawPlane
};

void GameObject::Render(PhysicsProgram* program)
{
	Collider* collider = body->GetCollider();
	if (collider != nullptr)
	{
		int shapeCount = collider->GetShapeCount();
		for (int i = 0; i < shapeCount; i++)
		{
			Shape* shape = collider->GetShape(i);
			int type = (int)shape->GetType();
			(drawFunctions[type])(shape, body->GetTransform(), colour, program);
		}
	}
}
