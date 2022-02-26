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

		inline void SetCollisionLayer(short cLayer) { collisionLayer = cLayer; }
		inline void SetCollisionMask(short cMask) { collisionMask = cMask; }
		inline short GetCollisionLayer() { return collisionLayer; }
		inline short GetCollisionMask() { return collisionMask; }

		inline void SetIInertia(float iI) { iInertia = iI; }
		inline void SetIMass(float iM) { iMass = iM; }
		inline float GetIInertia() { return iInertia; }
		inline float GetIMass() { return iMass; }

		bool GetIsTrigger() { return isTrigger; }
		float GetDensity() { return density; }

		Shape* GetShape() { return shape; }

	private:
		friend CollisionManager;
		friend PhysicsObject;

		Collider(Shape* shape, float density = 1, bool isTrigger = false);
		Collider() = default; // this is only used to initialize an array of colliders
		Collider(const Collider& other);
		Collider& operator=(const Collider& other);
		Collider& operator=(Collider&& other);
		~Collider();

		//void SetAttached(PhysicsObject* attached) { this->attached = attached; }
		bool CanBeDynamic();

		//should be a value like this: 0b0000000000001000 (the 1 corrosponds to the layer)
		unsigned short collisionLayer = 1;
		//by default is 0b1111111111111111. Each one corrosponds to a layer that can be collided with, zero bits mean it cannot collide with it
		unsigned short collisionMask = 0xFFFF;
		//this cannot be changed after the collider is created because that would cause problems
		bool isTrigger;
		//PhysicsObject* attached;
		AABB aABB;

		Shape* shape;
		float iInertia;
		float iMass;

		float density;
		bool isStatic = false;
	};
}
