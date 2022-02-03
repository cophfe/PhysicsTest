#include "PhysicsObject.h"
#include "PhysicsProgram.h"
#include "CollisionManager.h"

PhysicsObject::PhysicsObject(PhysicsData& data, Collider* collider)
{

}

void PhysicsObject::Update(PhysicsProgram& program)
{
	transform.position += velocity * program.GetDeltaTime();
	
	//(apply drag)
	velocity += force / iMass * program.GetDeltaTime();

	transform.rotation += angularVelocity * program.GetDeltaTime();
	//(apply angular drag)
	angularVelocity += torque * iMomentOfInertia * program.GetDeltaTime();

	//clear force stuff
	force = Vector2(0, 0);
	torque = 0;
}

void PhysicsObject::Render(PhysicsProgram& program)
{
	if (collider)
		collider->RenderShape(program);
}

//z value of cross product in 3D with a and b (x and y values equal 0)
float Cross(Vector2 a, Vector2 b)
{
	return a.x * b.y - a.y * b.x;
}

void PhysicsObject::AddForceAtPosition(Vector2 force, Vector2 point)
{
	this->force += force;
	//transform.position should actually be the center point of the collider
	this->torque += Cross(point - transform.position, force);
}

PhysicsObject::~PhysicsObject()
{
	delete collider;
	collider = nullptr;
}

PhysicsObject::PhysicsObject(const PhysicsObject& other)
{
	//get memory
	collider = (Collider*)(new char[sizeof(Collider)]);
	*collider = *other.collider;

	transform = other.transform;
	scaleRotationMatrix = other.scaleRotationMatrix;
	velocity = other.velocity;
	angularVelocity = other.angularVelocity;
	force = other.force;
	torque = other.torque;
	bounciness = other.bounciness;
	drag = other.drag;
	angularDrag = other.angularDrag;
	iMass = other.iMass;
	iMomentOfInertia = other.iMomentOfInertia;
	sleeping = other.sleeping;
}

PhysicsObject::PhysicsObject(PhysicsObject&& other)
{
	collider = other.collider;
	other.collider = nullptr;

	transform = other.transform;
	scaleRotationMatrix = other.scaleRotationMatrix;
	velocity = other.velocity;
	angularVelocity = other.angularVelocity;
	force = other.force;
	torque = other.torque;
	bounciness = other.bounciness;
	drag = other.drag;
	angularDrag = other.angularDrag;
	iMass = other.iMass;
	iMomentOfInertia = other.iMomentOfInertia;
	sleeping = other.sleeping;
}

PhysicsObject& PhysicsObject::operator=(const PhysicsObject& other)
{
	collider = (Collider*)(new char[sizeof(Collider)]);
	*collider = *other.collider;

	transform = other.transform;
	scaleRotationMatrix = other.scaleRotationMatrix;
	velocity = other.velocity;
	angularVelocity = other.angularVelocity;
	force = other.force;
	torque = other.torque;
	bounciness = other.bounciness;
	drag = other.drag;
	angularDrag = other.angularDrag;
	iMass = other.iMass;
	iMomentOfInertia = other.iMomentOfInertia;
	sleeping = other.sleeping;

	return *this;
}

PhysicsObject& PhysicsObject::operator=(PhysicsObject&& other)
{
	delete collider;
	collider = other.collider;
	other.collider = nullptr;

	transform = other.transform;
	scaleRotationMatrix = other.scaleRotationMatrix;
	velocity = other.velocity;
	angularVelocity = other.angularVelocity;
	force = other.force;
	torque = other.torque;
	bounciness = other.bounciness;
	drag = other.drag;
	angularDrag = other.angularDrag;
	iMass = other.iMass;
	iMomentOfInertia = other.iMomentOfInertia;
	sleeping = other.sleeping;

	return *this;
}


