#include "GameObject.h"
#include "PhysicsProgram.h"

GameObject::DrawFunction GameObject::drawFunctions[4] =
{
	PhysicsProgram::DrawCircle,
	PhysicsProgram::DrawPolygon,
	PhysicsProgram::DrawCapsule,
	PhysicsProgram::DrawPlane
};

GameObject::GameObject(PhysicsData object, CollisionManager* manager, Vector3 colour)
	: colour(colour), manager(manager)
{
	body = manager->CreatePhysicsObject(object);
	body->SetPointer(&body);
}

void GameObject::Render(PhysicsProgram* program)
{
	for (int i = 0; i < body->GetColliderCount(); i++)
	{
		Shape* shape = body->GetCollider(i).GetShape();
		int type = (int)shape->GetType();
		(drawFunctions[type])(shape, body->GetTransform(), colour, program);
	}

	auto& lR = program->GetLineRenderer();
	lR.DrawLineSegment(Vector2(body->GetAABB().max.x, body->GetAABB().min.y), body->GetAABB().max);
	lR.DrawLineSegment(body->GetAABB().max, Vector2(body->GetAABB().min.x, body->GetAABB().max.y));
	lR.DrawLineSegment(Vector2(body->GetAABB().min.x, body->GetAABB().max.y), body->GetAABB().min);
	lR.DrawLineSegment(body->GetAABB().min, Vector2(body->GetAABB().max.x, body->GetAABB().min.y));
}

GameObject::~GameObject()
{
	
	
}

GameObject::GameObject(GameObject&& other)
{
	body = other.body;
	colour = other.colour;
	manager = other.manager;
	other.body = nullptr;

}

GameObject& GameObject::operator=(GameObject&& other)
{
	body = other.body;
	colour = other.colour;
	manager = other.manager;
	other.body = nullptr;

	return *this;
}
