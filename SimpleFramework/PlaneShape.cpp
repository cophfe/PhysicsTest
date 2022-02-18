#include "Shape.h"
#include "PhysicsObject.h"
#include "PhysicsProgram.h"
#include "CollisionManager.h"
#include "ExtraMath.hpp"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Plane
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

PlaneShape::PlaneShape(Vector2 normal, float d) : normal(normal), distance(d)
{}

PlaneShape::PlaneShape(Vector2 normal, Vector2 startPosition)
{
	this->normal = glm::normalize(normal);
	distance = glm::dot(normal, startPosition);
}

PlaneShape::PlaneShape(Vector2 pointA, Vector2 pointB, void* null)
{
	Vector2 normal = glm::normalize(pointA - pointB);
	this->normal = Vector2(-normal.y, normal.x);
	distance = glm::dot(normal, pointB);
}

bool PlaneShape::PointCast(Vector2 point, Transform& transform)
{
	return glm::dot(transform.InverseTransformPoint(point), normal) < distance;
}

void PlaneShape::CalculateMass(float& mass, float& inertia, float density)
{
	mass = 0;
	inertia = 0;
}

AABB PlaneShape::CalculateAABB(Transform& transform)
{
	return AABB{ Vector2{INFINITY, INFINITY}, Vector2{-INFINITY, -INFINITY} };
}

SHAPE_TYPE PlaneShape::GetType()
{
	return SHAPE_TYPE::PLANE;
}

Shape* PlaneShape::Clone()
{
	return new PlaneShape(*this);
}

Vector2 PlaneShape::Support(Vector2 v, Transform& transform)
{
	v = transform.InverseTransformDirection(v);

	if (v == normal)
		return transform.TransformPoint(distance * v);
	else
		return Vector2(INFINITY, INFINITY);
}

Vector2 PlaneShape::GetCentrePoint()
{
    return normal * distance; 
}
