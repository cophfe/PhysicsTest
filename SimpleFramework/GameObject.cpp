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
}

GameObject::~GameObject()
{
	if (manager && body)
	{
		body->SetPointer(nullptr);
		manager->DeletePhysicsBody(body);
		body = nullptr;
	}
	
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
