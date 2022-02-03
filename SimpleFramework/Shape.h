#pragma once
#include "Maths.h"
#define max_vertices 8

class PhysicsProgram;
class Transform;

struct AABB {
public:
	Vector2 max;
	Vector2 min;
};

enum class SHAPE_TYPE {
	CIRCLE,
	POLYGON,
	CAPSULE,
	LINE
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// BASE CLASS
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class Shape
{
public:
	virtual float CalculateArea() = 0;
	virtual void RenderShape(PhysicsProgram& program, Transform& transform) = 0;
	virtual AABB CalculateAABB(Transform& transform) = 0;
	virtual SHAPE_TYPE GetType() = 0;
	virtual Shape* Clone() = 0;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// POLYGON CLASS
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class PolygonShape : public Shape 
{
public:

	PolygonShape(Vector2* vertices, int vertexCount, Vector2 centrePoint);

	float CalculateArea();
	void RenderShape(PhysicsProgram& program, Transform& transform);
	AABB CalculateAABB(Transform& transform);
	SHAPE_TYPE GetType();
	Shape* Clone();
	PolygonShape&& GetRegularPolygonCollider(float radius, int pointCount);
private:
	Vector2 points[max_vertices];
	Vector2 normals[max_vertices];
	char pointCount;
	Vector2 centrePoint;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// CIRCLE CLASS
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class CircleShape : public Shape
{
public:
	CircleShape(float radius, Vector2 centrePoint);
	float CalculateArea();
	void RenderShape(PhysicsProgram& program, Transform& transform);
	AABB CalculateAABB(Transform& transform);
	SHAPE_TYPE GetType();
	Shape* Clone();

private:
	float radius;
	Vector2 centrePoint;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// LINE CLASS
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class LineShape : public Shape 
{
public:

	LineShape(Vector2 a, Vector2 b);
	float CalculateArea();
	void RenderShape(PhysicsProgram& program, Transform& transform);
	AABB CalculateAABB(Transform& transform);
	SHAPE_TYPE GetType();
	Shape* Clone();

private:
	Vector2 pointA;
	Vector2 pointB;
	Vector2 normal;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// CAPSULE CLASS
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//is cheap because it is a line segment with a radius, like how a circle is a point with a radius
class SausageShape : public Shape 
{
public:

	SausageShape(float radius, float height);
	float CalculateArea();
	void RenderShape(PhysicsProgram& program, Transform& transform);
	AABB CalculateAABB(Transform& transform);
	SHAPE_TYPE GetType();
	Shape* Clone();

private:
	float radius;
	//(can construct a line from this and the centrepoint)
	Vector2 pointA;
	Vector2 pointB;
};

