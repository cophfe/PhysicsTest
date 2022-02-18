#include "PhysicsObject.h"
#include "PhysicsProgram.h"
#include "CollisionManager.h"
#include "ExtraMath.hpp"
const float sleepVelocityMag = 0.0001f; //(these are also squared)
const float sleepAngularVelocityMag = 0.0001f;
const float sleepTime = 0.2f;

PhysicsObject::PhysicsObject(PhysicsData& data) : transform(Transform(data.position, data.rotation)), bounciness(data.bounciness), drag(data.drag), angularDrag(data.angularDrag)
	,staticFriction(data.staticFriction), dynamicFriction(data.dynamicFriction), isDynamic(data.isDynamic), isRotatable(data.isRotatable)
{
	iMass = 0;
	iInertia = 0;

	colliders = nullptr;
	colliderCount = 0;

	pointer = nullptr;
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
	switch (colliderCount) {
	case 0:
		return;
	case 1:
		colliderAABB = colliders[0].CalculateAABB(transform);
		return;
	default:
	{
		AABB aabbs[2];
		aabbs[0] = colliders[0].CalculateAABB(transform);

		for (size_t i = 1; i < colliderCount; i++)
		{
			aabbs[1] = colliders[i].CalculateAABB(transform);

			if (aabbs[1].max.x > aabbs[0].max.x)
				aabbs[0].max.x = aabbs[1].max.x;
			if (aabbs[1].min.x < aabbs[0].min.x)
				aabbs[0].min.x = aabbs[1].min.x;
			if (aabbs[1].max.y > aabbs[0].max.y)
				aabbs[0].max.y = aabbs[1].max.y;
			if (aabbs[1].min.y > aabbs[0].min.y)
				aabbs[0].min.y = aabbs[1].min.y;
		}

		colliderAABB = aabbs[0];
	}
	}
}

void PhysicsObject::AddCollider(Shape* shape, float density, bool recalculateMass)
{
	if (colliders == nullptr)
	{
		colliders = new Collider[1] { Collider(shape, density) };
		colliderCount = 1;

	}
	else 
	{
		Collider* newColliders = (Collider*)new char[sizeof(Collider) * colliderCount + 1];
		memcpy(newColliders, colliders, colliderCount * sizeof(Collider));
		newColliders[colliderCount] = Collider(shape, density);

		delete[] colliders;
		colliders = newColliders;
		colliderCount++;
	}

	if (recalculateMass)
	{
		CalculateMass();
	}
}

void PhysicsObject::SetPointer(PhysicsObject** ptr)
{
	if (ptr)
	{
		*ptr = this;
	}
	pointer = ptr;
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
	if (colliders)
		delete[] colliders;
	colliders = nullptr;
	*pointer = nullptr;
}

void PhysicsObject::CalculateMass()
{
	float mass = 0;
	float inertia = 0;

	if (!isDynamic || !CanBeDynamic())
	{
		iMass = 0;
		iInertia = 0;

		for (size_t i = 0; i < colliderCount; i++)
		{
			colliders[i].iMass = 0;
			colliders[i].iInertia = 0;
		}
		return;
	}

	for (size_t i = 0; i < colliderCount; i++)
	{
		float colliderMass = 0;
		//inertia of composite shape = sum of individual inertias
		float colliderInertia = 0;

		colliders[i].CalculateMass(colliderMass, colliderInertia);
		mass += colliderMass;
		inertia += colliderInertia;
	}
	iInertia = inertia == 0 ? 0 : 1.0f / inertia;
	iMass = mass == 0 ? 0 : 1.0f / mass;

	if (!isRotatable)
	{
		iInertia = 0;
		for (size_t i = 0; i < colliderCount; i++)
		{
			colliders[i].iInertia = 0;
		}
	}
}

bool PhysicsObject::CanBeDynamic()
{
	for (size_t i = 0; i < colliderCount; i++)
	{
		if (!colliders[i].CanBeDynamic())
			return false;
	}
	return true;
}

PhysicsObject::PhysicsObject(const PhysicsObject& other) : staticFriction(other.staticFriction), dynamicFriction(other.dynamicFriction)
{
	colliders = (Collider*)(new char[sizeof(Collider) * other.colliderCount]);
	memcpy(colliders, other.colliders, sizeof(Collider) * other.colliderCount);
	colliderCount = other.colliderCount;
	/*for (size_t i = 0; i < colliderCount; i++)
	{
		colliders[i].SetAttached(this);
	}*/

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
	isDynamic = other.isDynamic;
	isRotatable = other.isRotatable;
	pointer = other.pointer;
}

PhysicsObject::PhysicsObject(PhysicsObject&& other) : staticFriction(other.staticFriction), dynamicFriction(other.dynamicFriction)
{
	colliders = other.colliders;
	other.colliders = nullptr;
	colliderCount = other.colliderCount;

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
	isDynamic = other.isDynamic;
	isRotatable = other.isRotatable;
	pointer = other.pointer;
}

PhysicsObject& PhysicsObject::operator=(const PhysicsObject& other)
{
	if (colliders)
		delete[] colliders;
	colliders = (Collider*)(new char[sizeof(Collider) * other.colliderCount]);
	memcpy(colliders, other.colliders, sizeof(Collider) * other.colliderCount);
	colliderCount = other.colliderCount;


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

	isDynamic = other.isDynamic;
	isRotatable = other.isRotatable;
	pointer = other.pointer;

	return *this;
}

PhysicsObject& PhysicsObject::operator=(PhysicsObject&& other) 
{
	if (colliders)
		delete[] colliders;
	colliders = other.colliders;
	/*for (size_t i = 0; i < colliderCount; i++)
	{
		colliders[i].SetAttached(this);

	}*/
	colliderCount = other.colliderCount;
	other.colliders = nullptr;

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

	isDynamic = other.isDynamic;
	isRotatable = other.isRotatable;
	pointer = other.pointer;

	return *this;
}


