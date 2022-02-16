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
	CollisionData() { a = nullptr; b = nullptr; penetration = 0; type = (COLLISION_TYPE)0; pointCount = 1; shapeIndexA = 0; shapeIndexB = 0; }
	CollisionData(PhysicsObject* a, PhysicsObject* b) : a(a), b(b) { penetration = 0; type = (COLLISION_TYPE)0; pointCount = 1; shapeIndexA = 0; shapeIndexB = 0; }

	PhysicsObject* a;
	PhysicsObject* b;
	char shapeIndexA;
	char shapeIndexB;
	Vector2 collisionPoints[MAX_COLLISION_POINTS];
	char pointCount;//pointCount is 1 unless explicitly set to something else
	Vector2 collisionNormal;
	float penetration;
	COLLISION_TYPE type;
};

//return true if collision occured
bool CollideCircleCircle(CollisionData& data);
bool CollideCirclePolygon(CollisionData& data);
bool CollideCircleCapsule(CollisionData& data);
bool CollideCirclePlane(CollisionData& data);
bool CollidePolygonPolygon(CollisionData& data);
bool CollidePolygonCapsule(CollisionData& data);
bool CollidePolygonPlane(CollisionData& data);
bool CollideCapsuleCapsule(CollisionData& data);
bool CollideCapsulePlane(CollisionData& data);
bool CollideInvalid(CollisionData& data);

//flipped functions
bool CollidePolygonCircle(CollisionData& data);
bool CollideCapsuleCircle(CollisionData& data);
bool CollidePlaneCircle(CollisionData& data);
bool CollideCapsulePolygon(CollisionData& data);
bool CollidePlanePolygon(CollisionData& data);
bool CollidePlaneCapsule(CollisionData& data);
