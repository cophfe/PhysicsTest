#pragma once
#include "Maths.h"
#include "Shape.h"

namespace fzx 
{
	class PhysicsObject;
	class CollisionManager;
	class Transform;

	class Collider
	{

	public:

		//calculaters
		void CalculateMass(float& massVar, float& inertiaVar);
		AABB& CalculateAABB(Transform& transform);

		Shape* GetShape() { return shape; }

		//void AddShape(Shape* shape);
		//void ResetShapes(Shape* shape); //<< collider has to have at least one shape


	private:
		friend CollisionManager;
		friend PhysicsObject;

		Collider(Shape* shape, float density = 1);
		Collider(const Collider& other);
		Collider& operator=(const Collider& other);
		Collider& operator=(Collider&& other);
		~Collider();



		//void SetAttached(PhysicsObject* attached) { this->attached = attached; }
		bool CanBeDynamic();

		//PhysicsObject* attached;
		AABB aABB;

		Shape* shape;
		float iInertia;
		float iMass;

		float density;
		bool isStatic = false;
	};
}
