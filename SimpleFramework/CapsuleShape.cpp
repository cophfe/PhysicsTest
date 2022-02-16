#include "Shape.h"
#include "PhysicsObject.h"
#include "PhysicsProgram.h"
#include "CollisionManager.h"
#include "ExtraMath.hpp"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Stadium
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

CapsuleShape::CapsuleShape(Vector2 a, Vector2 b, float radius)
{
	pointA = a;
	pointB = b;

	if (a == b)
		pointB.y += 0.00001f;
	this->radius = radius;
}

bool CapsuleShape::PointCast(Vector2 point, Transform& transform)
{
	//capsule is basically 2 circles and a rect

	//inverse transform point instead of transforming the points, one less calculation
	point = transform.InverseTransformPoint(point);

	//if point is inside the two circles, it is colliding
	if (em::SquareLength(pointA - point) > radius * radius || em::SquareLength(pointB - point) > radius * radius)
		return true;

	//if the point is inside the oriented bounding box, it is colliding
	Vector2 tangent = glm::normalize(pointA - pointB);
	Vector2 normal = { -tangent.y, tangent.x };

	//y axis, with 0 being the min of the OBB
	float distanceFromLine = glm::dot(normal, point) - glm::dot(normal, pointA);
	//x axis, 0 being the min of the OBB
	float distanceAlongLine = glm::dot(tangent, point) - glm::dot(tangent, pointA);

	if (glm::abs(distanceFromLine) < radius
		&& distanceAlongLine > 0 && distanceAlongLine < glm::dot(tangent, pointB))
	{
		return true;
	}
	else return false;
}

void CapsuleShape::CalculateMass(float& mass, float& inertia, float density)
{
	float len = glm::length(pointA - pointB);
	mass = density * (2 * radius * len + radius * radius * glm::pi<float>());
	inertia = 0;

	//inertia is equal to 2 translated semicircles + inertia of rectangle
	float semiCircleMass = 0.5f * density * radius * radius * glm::pi<float>();
	float semiCircleInertia = semiCircleMass * radius * radius * 0.5f;
	//translate inertia by adding mr^2 (proof in notebook)
	//the semi circle is currently rotating around what would be it's centre if it was a full circle
	//so need to move it by half of length of the capsule's rectangle
	semiCircleInertia += semiCircleMass * 0.25 * len * len;
	//now get the rect inertia
	float width = radius * 2;
	float rectMass = len * width * density;
	float rectInertia = rectMass * (width * width + len * len) / 12.0f;
	//rect inertia is already about the right axis (relative to the semicircles.
	//now add all inertias together 
	inertia = rectInertia + semiCircleInertia * 2;
	//now translate it by the distance between the centrepoint of the capsule and the centrepoint of the physicsObject (0,0), so it is correct when rotating about that axis
	float dist = glm::length(0.5f * (pointA + pointB));
	inertia += mass * dist * dist;

	//success! (probably, I have no idea how to check)


}

void CapsuleShape::RenderShape(PhysicsProgram& program, Transform& transform, Vector3 colour)
{
	//if the buffer is big enough, this is a capsule, otherwise treat it as a line
	if (radius > 0.01f)
	{
		Vector2 a = transform.TransformPoint(pointA);
		Vector2 b = transform.TransformPoint(pointB);
		auto& lR = program.GetLineRenderer();
		lR.DrawCircle(a, radius, colour, 32);
		lR.DrawCircle(b, radius, colour, 32);

		Vector2 radiusAddition = glm::normalize(a - b) * radius;
		radiusAddition = { radiusAddition.y, -radiusAddition.x };

		lR.DrawLineSegment(a + radiusAddition, b + radiusAddition, colour);
		lR.DrawLineSegment(a - radiusAddition, b - radiusAddition, colour);
	}
	else {
		LineRenderer& lR = program.GetLineRenderer();
		lR.DrawLineSegment(transform.TransformPoint(pointA),
			transform.TransformPoint(pointB), colour);
	}

}

AABB CapsuleShape::CalculateAABB(Transform& transform)
{
	AABB aABB;
	Vector2 pA = transform.TransformPoint(pointA), pB = transform.TransformPoint(pointB);

	if (pA.x > pB.x) {
		aABB.max.x = pA.x + radius;
		aABB.min.x = pB.x - radius;
	}
	else
	{
		aABB.max.x = pB.x + radius;
		aABB.min.x = pA.x - radius;
	}

	if (pA.y > pB.y) {
		aABB.max.y = pA.y + radius;
		aABB.min.y = pB.y - radius;
	}
	else
	{
		aABB.max.y = pB.y + radius;
		aABB.min.y = pA.y - radius;
	}
	return aABB;
}

SHAPE_TYPE CapsuleShape::GetType()
{
	return SHAPE_TYPE::CAPSULE;
}

Shape* CapsuleShape::Clone()
{
	return new CapsuleShape(*this);
}