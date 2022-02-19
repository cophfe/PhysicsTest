#pragma once
#include "PhysicsObject.h"
#define MAX_COLLISION_POINTS 2
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

struct CollisionData
{
	CollisionData() { a = nullptr; b = nullptr; penetration = 0; type = (COLLISION_TYPE)0; pointCount = 1; colliderIndexA = 0; colliderIndexB = 0; }
	CollisionData(PhysicsObject* a, PhysicsObject* b, char colliderIndexA = 0, char colliderIndexB = 0) 
		: a(a), b(b), colliderIndexA(colliderIndexA), colliderIndexB(colliderIndexB)
	{ penetration = 0; type = (COLLISION_TYPE)0; pointCount = 1; }

	PhysicsObject* a;
	PhysicsObject* b;
	char colliderIndexA;
	char colliderIndexB;
	Vector2 collisionPoints[MAX_COLLISION_POINTS];
	char pointCount;//pointCount is 1 unless explicitly set to something else
	Vector2 collisionNormal;
	float penetration;
	COLLISION_TYPE type;


};

