#pragma once
#include "Maths.h"
#include "Shape.h"
#include "LineRenderer.h"
class PhysicsProgram;
class PhysicsObject;
class CollisionManager;
class Transform;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// BASE COLLIDER CLASS
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class Collider
{
public:
	Collider(Shape* shape, float density = 1);
	Collider(Shape** shapes, int shapeCount, float density = 1);

	virtual void RenderShape(PhysicsProgram& program);

	//calculaters
	float CalculateInertia();
	float CalculateMass();
	void CalculateAABB(Transform& transform);

	~Collider();
	Collider(Collider& other);
	Collider& operator=(Collider& other);

private:
	friend CollisionManager;
	friend PhysicsObject;
	
	void SetAttached(PhysicsObject* attached) { this->attached = attached; }
	bool CanBeDynamic();
	PhysicsObject* attached;
	AABB aABB;
	float density;

	Shape** shapes;
	int shapeCount;
};


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// CIRCLE COLLIDER CLASS
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//class CircleCollider : public Collider
//{
//public:
//
//	CircleCollider(float radius, Vector2 centrePoint, PhysicsObject* attached);
//
//	void RenderShape(PhysicsProgram& program);
//	COLLIDER_TYPE GetType() { return COLLIDER_TYPE::CIRCLE; }
//	float CalculateArea();
//	void CalculateGlobal();
//	void CalculateAABB();
//
//protected:
//	float radius;
//	float globalRadius;
//};
////~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
////~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
////~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//// POLYGON COLLIDER CLASS
////~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//class PolygonCollider : public Collider 
//{
//public:
//	PolygonCollider(Vector2* vertices, int vertexCount, Vector2 centrePoint, PhysicsObject* attached);
//	void RenderShape(PhysicsProgram& program);
//	COLLIDER_TYPE GetType() { return COLLIDER_TYPE::POLYGON; }
//	float CalculateArea();
//	void CalculateGlobal();
//	void CalculateAABB();
//
//	static PolygonCollider&& GetRegularPolygonCollider(float radius, int pointCount, PhysicsObject* attached);
//
//protected:
//	Vector2* points;
//	Vector2* globalPoints;
//	int pointCount;
//};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~