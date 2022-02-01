#pragma once
#include "Maths.h"
#include "LineRenderer.h"
class PhysicsProgram;
class PhysicsObject;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// SUPPLIMENTARY CLASSES
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct AABB {
public:
	Vector2 max;
	Vector2 min;
};
enum class COLLIDER_TYPE : char
{
	POLYGON,
	CIRCLE
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// BASE COLLIDER CLASS
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class Collider
{
public:
	virtual void RenderShape(PhysicsProgram& program) = 0;
	virtual COLLIDER_TYPE GetType() = 0;
	virtual float CalculateArea() = 0;
	virtual void CalculateGlobal() = 0;
	virtual void CalculateAABB() = 0;

	AABB& GetAABB() { return aABB; };
protected:
	PhysicsObject* attached;
	Vector2 centrePoint;
	Vector2 globalCentrePoint;
	AABB aABB;
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// CIRCLE COLLIDER CLASS
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class CircleCollider : public Collider
{
public:

	CircleCollider(float radius, Vector2 centrePoint, PhysicsObject* attached);

	void RenderShape(PhysicsProgram& program);
	COLLIDER_TYPE GetType() { return COLLIDER_TYPE::CIRCLE; }
	float CalculateArea();
	void CalculateGlobal();
	void CalculateAABB();

protected:
	float radius;
	float globalRadius;
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// POLYGON COLLIDER CLASS
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class PolygonCollider : public Collider 
{
public:
	PolygonCollider(Vector2* vertices, int vertexCount, Vector2 centrePoint, PhysicsObject* attached);
	void RenderShape(PhysicsProgram& program);
	COLLIDER_TYPE GetType() { return COLLIDER_TYPE::POLYGON; }
	float CalculateArea();
	void CalculateGlobal();
	void CalculateAABB();

	static PolygonCollider&& GetRegularPolygonCollider(float radius, int pointCount, PhysicsObject* attached);

protected:
	Vector2* points;
	Vector2* globalPoints;
	int pointCount;
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~