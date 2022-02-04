#pragma once
#include "PhysicsObject.h"
#include <vector>
#include <iostream>

enum class COLLISION_TYPE
{
	CIRCLECIRCLE,
	CIRCLEPOLYGON,
	CIRCLELINE,
	POLYGONPOLYGON,
	POLYGONLINE,
	INVALID,
	COUNT
};

struct CollisionManifold
{
	CollisionManifold() { a = nullptr; b = nullptr; penetration = 0; type = (COLLISION_TYPE)0; }
	CollisionManifold(PhysicsObject* a, PhysicsObject* b) : a(a), b(b) { penetration = 0; type = (COLLISION_TYPE)0; }

	PhysicsObject* a;
	PhysicsObject* b;
	//not used rn, will be used to factor in rotation speed of objects + apply torque
	//Vector2 collisionPoints[1];
	Vector2 collisionNormal;
	float penetration;
	COLLISION_TYPE type;
};

class CollisionManager
{
public:
	void ResolveCollisions(std::vector<PhysicsObject>& pObjects);

private:
	bool CheckAABBCollision(AABB& a, AABB& b);
	
	void ResolveCollision(CollisionManifold& manifold);
	bool EvaluateCollision(CollisionManifold& manifold);
	void GetCollisionType(CollisionManifold& manifold);

	std::vector<CollisionManifold> collisions;
	
};

