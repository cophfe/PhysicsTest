#pragma once
#include "PhysicsObject.h"
#include <vector>
#include <iostream>
class PhysicsProgram;

enum class COLLISION_TYPE
{
	CIRCLECIRCLE,
	CIRCLEPOLYGON,
	CIRCLECAPSULE,
	CIRCLEPLANE,
	POLYGONPOLYGON,
	POLYGONCAPSULE,
	POLYGONPLANE,
	CAPSULECAPSULE,
	CAPSULEPLANE,
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
	Vector2 collisionPoints[1];
	Vector2 collisionNormal;
	float penetration;
	COLLISION_TYPE type;
};

class CollisionManager
{
public:
	CollisionManager(PhysicsProgram* program) : program(program) {}

	void ResolveCollisions(std::vector<PhysicsObject>& pObjects);

	PhysicsObject* PointCast(Vector2 point, std::vector<PhysicsObject>& pObjects, bool includeStatic = false);
private:
	bool CheckAABBCollision(AABB& a, AABB& b);
	
	void ResolveCollision(CollisionManifold& manifold);
	bool EvaluateCollision(CollisionManifold& manifold);
	void GetCollisionType(CollisionManifold& manifold);

	std::vector<CollisionManifold> collisions;
	
	//for debug drawing
	PhysicsProgram* program;
};

