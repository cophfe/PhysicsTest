#pragma once
#include "Maths.h"
#define max_vertices 8
#include <iostream>

class PhysicsProgram;
class Transform;
class CollisionManager;

struct AABB {
public:
	Vector2 max;
	Vector2 min;

	bool PointCast(Vector2 point)
	{
		return point.x < max.x && point.x > min.x
			&& point.y < max.y && point.y > min.y;
	}
};

enum class SHAPE_TYPE : unsigned char {
	CIRCLE = 1,
	POLYGON = 4,
	CAPSULE = 16,
	PLANE = 64,
	COUNT = 4
};



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// BASE CLASS
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class Shape
{
public:
	virtual bool PointCast(Vector2 point, Transform& transform) = 0;
	virtual void CalculateMass(float& mass, float& inertia, float density) = 0;
	virtual void RenderShape(PhysicsProgram& program, Transform& transform, Vector3 colour) = 0;
	virtual void RenderShape(Transform transform, PhysicsProgram& program, Vector3 colour );
	virtual AABB CalculateAABB(Transform& transform) = 0;
	virtual SHAPE_TYPE GetType() = 0;
	virtual Shape* Clone() = 0;

private:
	friend CollisionManager;

};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// POLYGON CLASS
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class PolygonShape : public Shape 
{
public:

	PolygonShape(Vector2* vertices, int vertexCount);

	bool PointCast(Vector2 point, Transform& transform);
	void CalculateMass(float& mass, float& inertia, float density);
	void RenderShape(PhysicsProgram& program, Transform& transform, Vector3 colour );
	AABB CalculateAABB(Transform& transform);
	SHAPE_TYPE GetType();
	Shape* Clone();
	static PolygonShape* GetRegularPolygonCollider(float radius, int pointCount);

private:
	friend CollisionManager;
	void CalculateNormals();
	void CalculateCentrePoint();
	bool OrganisePoints(Vector2* points, int pointCount);

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
	
	bool PointCast(Vector2 point, Transform& transform);
	void CalculateMass(float& mass, float& inertia, float density);
	void RenderShape(PhysicsProgram& program, Transform& transform, Vector3 colour );
	AABB CalculateAABB(Transform& transform);
	SHAPE_TYPE GetType();
	Shape* Clone();

private:
	friend CollisionManager;


	float radius;
	Vector2 centrePoint;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// LINE CLASS
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class CapsuleShape : public Shape 
{
public:

	CapsuleShape(Vector2 a, Vector2 b, float radius = 0.01f);

	bool PointCast(Vector2 point, Transform& transform);
	void CalculateMass(float& mass, float& inertia, float density); // change to 
	void RenderShape(PhysicsProgram& program, Transform& transform, Vector3 colour );
	AABB CalculateAABB(Transform& transform);
	SHAPE_TYPE GetType();
	Shape* Clone();

private:
	friend CollisionManager;

	float radius;
	Vector2 pointA;
	Vector2 pointB;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// PLANE CLASS
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class PlaneShape : public Shape 
{
public:
	PlaneShape(Vector2 normal, float d);
	PlaneShape(Vector2 normal, Vector2 startPosition);
	PlaneShape(Vector2 pointA, Vector2 pointB, void* null);

	bool PointCast(Vector2 point, Transform& transform);
	void CalculateMass(float& mass, float& inertia, float density);
	void RenderShape(PhysicsProgram& program, Transform& transform, Vector3 colour);
	AABB CalculateAABB(Transform& transform);
	SHAPE_TYPE GetType();
	Shape* Clone();

private:
	friend CollisionManager;

	Vector2 normal;
	float distance;
};



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// CAPSULE CLASS
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//is cheap because it is a line segment with a radius, like how a circle is a point with a radius
//class SausageShape : public Shape 
//{
//public:
//
//	SausageShape(float radius, float height);
//	void CalculateMass(float& mass, float& inertia, float density);
//	void RenderShape(PhysicsProgram& program, Transform& transform);
//	AABB CalculateAABB(Transform& transform);
//	SHAPE_TYPE GetType();
//	Shape* Clone();
//
//private:
//	float radius;
//	//(can construct a line from this and the centrepoint)
//	Vector2 pointA;
//	Vector2 pointB;
//};

