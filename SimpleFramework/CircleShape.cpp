#include "Shape.h"
#include "PhysicsObject.h"
#include "PhysicsProgram.h"
#include "CollisionManager.h"
#include "ExtraMath.hpp"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// CIRCLE
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

CircleShape::CircleShape(float radius, Vector2 centrePoint)
{
	this->radius = radius;
	this->centrePoint = centrePoint;
}

bool CircleShape::PointCast(Vector2 point, Transform& transform)
{
	Vector2 centre = transform.TransformPoint(centrePoint);
	return em::SquareLength(centre - point) < radius * radius;
}

void CircleShape::CalculateMass(float& mass, float& inertia, float density)
{
	mass = density * radius * radius * glm::pi<float>();
	//circle is a cylinder with thickness of one, meaning the mass moment of inertia is mr^2/2
	inertia = mass * radius * radius * 0.5f;
	//im a bit confused on how translating inertia tensor stuff works but I think this is correct
	inertia += mass * em::SquareLength(centrePoint);
}

void CircleShape::RenderShape(PhysicsProgram& program, Transform& transform, Vector3 colour)
{
	program.GetLineRenderer().DrawCircle(transform.TransformPoint(centrePoint), radius, colour);
	program.GetLineRenderer().DrawLineSegment(transform.TransformPoint(Vector2(0, radius)), transform.TransformPoint(Vector2(0, radius * 0.5f)), colour);
}

AABB CircleShape::CalculateAABB(Transform& transform)
{
	AABB aABB;
	Vector2 globalCentrePoint = transform.TransformPoint(centrePoint);
	aABB.max.x = globalCentrePoint.x + radius;
	aABB.max.y = globalCentrePoint.y + radius;
	aABB.min.x = globalCentrePoint.x - radius;
	aABB.min.y = globalCentrePoint.y - radius;
	return aABB;
}

SHAPE_TYPE CircleShape::GetType()
{
	return SHAPE_TYPE::CIRCLE;
}

Shape* CircleShape::Clone()
{
	return new CircleShape(*this);
}
