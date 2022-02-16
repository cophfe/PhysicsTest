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

void PlaneShape::RenderShape(PhysicsProgram& program, Transform& transform, Vector3 colour)
{
	auto& lR = program.GetLineRenderer();

	Vector2 tNormal = transform.TransformDirection(normal);

	Vector2 tPlanePoint = transform.TransformPoint(normal * distance);
	float tDist = glm::dot(normal, tPlanePoint);

	Vector2 tangent = 1000.0f * Vector2{ tNormal.y, -tNormal.x };
	lR.DrawLineSegment(tPlanePoint + tangent, tPlanePoint - tangent, colour);
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