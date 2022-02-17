#include "PhysicsObject.h"
#include "PhysicsProgram.h"
#include "CollisionManager.h"
#include "ExtraMath.hpp"
const float sleepVelocityMag = 0.0001f; //(these are also squared)
const float sleepAngularVelocityMag = 0.0001f;
const float sleepTime = 0.2f;

PhysicsObject::PhysicsObject(PhysicsData& data, Collider* collider) : transform(Transform(data.position, data.rotation)), bounciness(data.bounciness), drag(data.drag), angularDrag(data.angularDrag)
	,staticFriction(data.staticFriction), dynamicFriction(data.dynamicFriction)
{
	if (!data.isDynamic)
	{
		iMass = 0;
		iInertia = 0;
	}
	else if (!data.isRotatable)
	{
		iInertia = 0;
	}
	else if (data.mass == -1) 
	{
		if (collider != nullptr) {
			float mass, inertia;
			collider->CalculateMass(mass, inertia);
			if (mass == 0)
				iMass = 0;
			else
				iMass = 1 / mass;

			if (inertia == 0)
				iInertia = 0;
			else
				iInertia = 1 / inertia;
		}
		else {
			iMass = 0;
			iInertia = 0;
		}
	}
	
	this->collider = collider;
	if (collider != nullptr)
	{
		collider->SetAttached(this);
		if (!collider->CanBeDynamic()) 
		{
			iMass = 0;
			iInertia = 0;
		}
		collider->CalculateAABB(transform);
	}
}

void PhysicsObject::Update(float deltaTime)
{
	transform.position += velocity * deltaTime;
	transform.rotation += angularVelocity * deltaTime;

	//update transform
	transform.UpdateData();
	
	velocity += force * iMass * deltaTime;
	angularVelocity += torque * iInertia * deltaTime;
	
	//based on box2d's drag method
	velocity /= (1.0f + drag * deltaTime);
	angularVelocity /= (1.0f + angularDrag * deltaTime);

	//clear force stuff
	force = Vector2(0, 0);
	torque = 0;
}

void PhysicsObject::GenerateAABB() {

	if (collider)
		collider->CalculateAABB(transform);
}

void PhysicsObject::AddForceAtPosition(Vector2 force, Vector2 point)
{
	this->force += force;
	//transform.position should actually be the center point of the collider
	this->torque += em::Cross(point - transform.position, force);
}

void PhysicsObject::AddImpulseAtPosition(Vector2 impulse, Vector2 point)
{
	this->velocity += impulse * iMass;

	//transform.position should be the centre of mass
	this->angularVelocity += em::Cross(point - transform.position, impulse) * iInertia;
}

void PhysicsObject::AddVelocityAtPosition(Vector2 velocity, Vector2 point)
{
	this->velocity += velocity;

	//transform.position should be the centre of mass
	this->angularVelocity += em::Cross(point - transform.position, velocity);
}

PhysicsObject::~PhysicsObject()
{
	delete collider;
	collider = nullptr;
}

PhysicsObject::PhysicsObject(const PhysicsObject& other) : staticFriction(other.staticFriction), dynamicFriction(other.dynamicFriction)
{
	collider = new Collider(*other.collider);
	collider->SetAttached(this);

	transform = other.transform;
	velocity = other.velocity;
	angularVelocity = other.angularVelocity;
	force = other.force;
	torque = other.torque;
	bounciness = other.bounciness;
	drag = other.drag;
	angularDrag = other.angularDrag;
	iMass = other.iMass;
	iInertia = other.iInertia;
}

PhysicsObject::PhysicsObject(PhysicsObject&& other) : staticFriction(other.staticFriction), dynamicFriction(other.dynamicFriction)
{
	collider = other.collider;
	collider->SetAttached(this);

	other.collider = nullptr;

	transform = other.transform;
	velocity = other.velocity;
	angularVelocity = other.angularVelocity;
	force = other.force;
	torque = other.torque;
	bounciness = other.bounciness;
	drag = other.drag;
	angularDrag = other.angularDrag;
	iMass = other.iMass;
	iInertia = other.iInertia;
}

PhysicsObject& PhysicsObject::operator=(const PhysicsObject& other)
{
	delete collider;
	collider = new Collider(*other.collider);
	collider->SetAttached(this);

	staticFriction = other.staticFriction;
	dynamicFriction = other.dynamicFriction;
	transform = other.transform;
	velocity = other.velocity;
	angularVelocity = other.angularVelocity;
	force = other.force;
	torque = other.torque;
	bounciness = other.bounciness;
	drag = other.drag;
	angularDrag = other.angularDrag;
	iMass = other.iMass;
	iInertia = other.iInertia;

	return *this;
}

PhysicsObject& PhysicsObject::operator=(PhysicsObject&& other) 
{
	delete collider;
	collider = other.collider;
	collider->SetAttached(this);

	other.collider = nullptr;
	staticFriction = other.staticFriction;
	dynamicFriction = other.dynamicFriction;
	transform = other.transform;
	velocity = other.velocity;
	angularVelocity = other.angularVelocity;
	force = other.force;
	torque = other.torque;
	bounciness = other.bounciness;
	drag = other.drag;
	angularDrag = other.angularDrag;
	iMass = other.iMass;
	iInertia = other.iInertia;

	return *this;
}


