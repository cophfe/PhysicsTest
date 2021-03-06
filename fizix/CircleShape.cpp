#include "fzx.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// CIRCLE
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
namespace fzx
{
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

	Vector2 CircleShape::Support(Vector2 v, Transform& transform)
	{
		return transform.TransformPoint(centrePoint) + v * radius;
	}

	Vector2 CircleShape::GetCentrePoint()
	{
		return centrePoint;
	}
}