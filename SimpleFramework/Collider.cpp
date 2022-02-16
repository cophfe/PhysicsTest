#include "Collider.h"
#include "PhysicsProgram.h"
#include "CollisionManager.h"
#include "PhysicsObject.h"

Collider::Collider(Shape* shape, float density, Vector3 colour)
{
	shapes = new Shape *[1];
	shapes[0] = shape;
	shapeCount = 1;
	this->density = density;
	this->colour = colour;
}

Collider::Collider(Shape** shapes, int shapeCount, float density, Vector3 colour)
{
	this->shapes = shapes;
	this->shapeCount = shapeCount;
	this->density = density;
	this->colour = colour;
}

void Collider::RenderShape(PhysicsProgram& program) {
	for (size_t i = 0; i < shapeCount; i++)
	{
		shapes[i]->RenderShape(attached->transform, program, colour);
		
	}
}

void Collider::CalculateMass(float& massVar, float& inertiaVar)
{
	float mass = 0;
	float inertia = 0;
	for (size_t i = 0; i < shapeCount; i++)
	{
		float shapeMass = 0;
		float shapeInertia = 0; //looks like the MOI of a composite shape is the sum of the individual parts MOI
		shapes[i]->CalculateMass(shapeMass, shapeInertia, density);
		mass += shapeMass;
		inertia += shapeInertia;
	}

	massVar = mass;
	inertiaVar = inertia;
}

void Collider::CalculateAABB(Transform& transform)
{
	switch (shapeCount) {
	case 0:
		return;
	case 1:
		aABB = shapes[0]->CalculateAABB(transform);
		return;
	default:
	{
		AABB* aabbs = new AABB[shapeCount];
		aabbs[0] = shapes[0]->CalculateAABB(transform);
		aABB = aabbs[0];

		for (size_t i = 1; i < shapeCount; i++)
		{
			aabbs[i] = shapes[i]->CalculateAABB(transform);

			if (aabbs[i].max.x > aABB.max.x)
				aABB.max.x = aabbs[i].max.x;
			if (aabbs[i].min.x < aABB.min.x)
				aABB.min.x = aabbs[i].min.x;
			if (aabbs[i].max.y > aABB.max.y)
				aABB.max.y = aabbs[i].max.y;
			if (aabbs[i].min.y > aABB.min.y)
				aABB.min.y = aabbs[i].min.y;
		}
	}
	}
	
}

Collider::~Collider()
{
	for (size_t i = 0; i < shapeCount; i++)
	{
		delete shapes[i];
		shapes[i] = nullptr;
	}
	delete[] shapes;
	shapes = nullptr;
}

Collider::Collider(Collider& other)
{
	aABB = other.aABB;
	shapeCount = other.shapeCount;
	attached = other.attached;
	density = other.density;
	colour = other.colour;

	shapes = new Shape*[shapeCount];
	for (size_t i = 0; i < shapeCount; i++)
	{
		shapes[i] = other.shapes[i]->Clone();
	}
}

Collider& Collider::operator=(Collider& other)
{
	for (size_t i = 0; i < shapeCount; i++)
	{
		delete shapes[i];
	}
	delete[] shapes;

	aABB = other.aABB;
	shapeCount = other.shapeCount;
	attached = other.attached;
	density = other.density;
	colour = other.colour;

	shapes = new Shape * [shapeCount];
	for (size_t i = 0; i < shapeCount; i++)
	{
		shapes[i] = other.shapes[i]->Clone();
	}

	return *this;
}

bool Collider::CanBeDynamic()
{
	//planes cannot be dynamic, because they don't have any mass and it dont make sense.

	for (size_t i = 0; i < shapeCount; i++)
	{
		if (shapes[i]->GetType() == SHAPE_TYPE::PLANE)
			return false;
	}
	return true;
}


