#pragma once
#include "PhysicsObject.h"
#include <vector>

enum class COLLISION_TYPE
{
	CIRCLECIRCLE,
	POLYGONCIRCLE,
	POLYGONPOLYGON,
	LINECIRCLE,
	LINELINE,
	POLYGONLINE,

};

struct CollisionManifold
{
	CollisionManifold() { a = nullptr; b = nullptr; penetration = 0; type = (COLLISION_TYPE)0; }
	CollisionManifold(PhysicsObject* a, PhysicsObject* b) : a(a), b(b) { penetration = 0; type = (COLLISION_TYPE)0; }

	PhysicsObject* a;
	PhysicsObject* b;
	Vector2 collisionNormal;
	float penetration;
	Vector2 hitPosition;
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

