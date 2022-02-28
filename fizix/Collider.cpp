#include "fzx.h"

namespace fzx
{
	Collider::Collider(Shape* shape, float density, bool isTrigger)
	{
		this->shape = shape;
		this->density = density;
		this->isTrigger = isTrigger;

		float massVar, inertiaVar;
		CalculateMass(massVar, inertiaVar);
	}

	void Collider::CalculateMass(float& massVar, float& inertiaVar)
	{
		shape->CalculateMass(massVar, inertiaVar, density);
		area = massVar / density;
	}

	AABB& Collider::CalculateAABB(Transform& transform)
	{
		aABB = shape->CalculateAABB(transform);
		return aABB;
	}

	Collider::~Collider()
	{
		delete shape;
		shape = nullptr;
	}

	Collider::Collider(const Collider& other)
	{
		aABB = other.aABB;
		shape = other.shape->Clone();
		//attached = other.attached;
		density = other.density;
		collisionLayer = other.collisionLayer;
		collisionMask = other.collisionMask;
		isTrigger = other.isTrigger;
	}

	Collider& Collider::operator=(const Collider& other)
	{
		delete shape;
		shape = other.shape->Clone();

		aABB = other.aABB;
		density = other.density;
		collisionLayer = other.collisionLayer;
		collisionMask = other.collisionMask;
		isTrigger = other.isTrigger;

		return *this;
	}

	Collider& Collider::operator=(Collider&& other)
	{
		shape = other.shape;
		other.shape = nullptr;

		aABB = other.aABB;
		density = other.density;
		collisionLayer = other.collisionLayer;
		collisionMask = other.collisionMask;
		isTrigger = other.isTrigger;

		return *this;
	}

	bool Collider::CanBeDynamic()
	{
		//planes cannot be dynamic, because they don't have any mass and it dont make sense.
		return shape->GetType() != SHAPE_TYPE::PLANE;
	}
}