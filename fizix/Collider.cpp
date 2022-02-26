#include "fzx.h"

namespace fzx
{
	Collider::Collider(Shape* shape, float density)
	{
		this->shape = shape;
		this->density = density;

		float massVar, inertiaVar;
		CalculateMass(massVar, inertiaVar);
	}

	void Collider::CalculateMass(float& massVar, float& inertiaVar)
	{
		shape->CalculateMass(massVar, inertiaVar, density);

		iInertia = inertiaVar == 0 ? 0 : 1.0f / inertiaVar;
		iMass = massVar == 0 ? 0 : 1.0f / massVar;
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
	}

	Collider& Collider::operator=(const Collider& other)
	{
		delete shape;
		shape = other.shape->Clone();

		aABB = other.aABB;
		//attached = other.attached;
		density = other.density;

		return *this;
	}

	Collider& Collider::operator=(Collider&& other)
	{
		shape = other.shape;
		other.shape = nullptr;

		aABB = other.aABB;
		//attached = other.attached;
		density = other.density;

		return *this;
	}

	bool Collider::CanBeDynamic()
	{
		//planes cannot be dynamic, because they don't have any mass and it dont make sense.
		return shape->GetType() != SHAPE_TYPE::PLANE;
	}
}