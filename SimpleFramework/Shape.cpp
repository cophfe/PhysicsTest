#include "Shape.h"
#include "PhysicsObject.h"
#include "PhysicsProgram.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// POLYGON
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

PolygonShape::PolygonShape(Vector2* vertices, int vertexCount, Vector2 centrePoint)
{
	memcpy(points, vertices, sizeof(Vector2) * vertexCount);
	
	//memcpy(points, vertices, sizeof(Vector2) * vertexCount);
	pointCount = vertexCount;
	this->centrePoint = centrePoint;

}

float PolygonShape::CalculateArea()
{
	float area = 0;

	for (size_t i = 0; i < pointCount - 1; i++)
	{
		area += centrePoint.x * (points[i].y - points[i + 1].y) + points[i].x * (points[i + 1].y - centrePoint.y) + points[i + 1].x * (centrePoint.y - points[i].y);
	}
	area += centrePoint.x * (points[pointCount - 1].y - points[0].y) + points[pointCount - 1].x * (points[0].y - centrePoint.y) + points[0].x * (centrePoint.y - points[pointCount - 1].y);
	area *= 0.5f;

	return area;

}

void PolygonShape::RenderShape(PhysicsProgram& program, Transform& transform)
{
	auto& lines = program.GetLineRenderer();
	for (size_t i = 0; i < pointCount; i++)
	{
		lines.AddPointToLine(transform.TransformPoint(points[i]));
	}
	lines.FinishLineLoop();
}

AABB PolygonShape::CalculateAABB(Transform& transform)
{
	float xMax = points[0].x;
	float xMin = points[0].x;

	float yMax = points[0].y;
	float yMin = points[0].y;

	for (size_t i = 1; i < pointCount; i++)
	{
		if (points[i].x > xMax)
			xMax = points[i].x;

		if (points[i].y > yMax)
			yMax = points[i].y;

		if (points[i].x < xMin)
			xMin = points[i].x;

		if (points[i].y < yMin)
			yMin = points[i].y;
	}

	return { Vector2(xMax, yMax), Vector2(xMin, yMin) };
}

SHAPE_TYPE PolygonShape::GetType()
{
	return SHAPE_TYPE::POLYGON;
}

Shape* PolygonShape::Clone()
{
	return new PolygonShape(*this);
}

PolygonShape* PolygonShape::GetRegularPolygonCollider(float radius, int pointCount)
{
	Vector2* points = new Vector2[pointCount];

	float iPointCount = glm::two_pi<float>() / pointCount;
	for (size_t i = 0; i < pointCount; i++)
	{
		points[i] = Vector2(radius * cos((i + 0.5f) * iPointCount), radius * sin((i + 0.5f) * iPointCount));
	}

	return new PolygonShape(points, pointCount, Vector2(0, 0));
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// CIRCLE
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

CircleShape::CircleShape(float radius, Vector2 centrePoint)
{
	this->radius = radius;
	this->centrePoint = centrePoint;
}

float CircleShape::CalculateArea()
{
	return radius * radius * glm::pi<float>();
}

void CircleShape::RenderShape(PhysicsProgram& program, Transform& transform)
{
	program.GetLineRenderer().DrawCircle(transform.TransformPoint(centrePoint), radius);
	program.GetLineRenderer().DrawLineSegment(transform.TransformPoint(Vector2(0, radius)), transform.TransformPoint(Vector2(0, radius * 0.5f)));
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

LineShape::LineShape(Vector2 a, Vector2 b)
{
	pointA = a;
	pointB = b;
	normal = glm::normalize(b - a);
	normal = { -normal.y, normal.x };
}

float LineShape::CalculateArea()
{
	return 0.0f;
}

void LineShape::RenderShape(PhysicsProgram& program, Transform& transform)
{
	LineRenderer& lR = program.GetLineRenderer();
	lR.DrawLineSegment(transform.TransformPoint(pointA),
		transform.TransformPoint(pointB));
}

AABB LineShape::CalculateAABB(Transform& transform)
{
	AABB aABB;
	if (pointA.x > pointB.x) {
		aABB.max.x = pointA.x;
		aABB.min.x = pointB.x;
	}
	else 
	{
		aABB.max.x = pointB.x;
		aABB.min.x = pointA.x;
	}

	if (pointA.y > pointB.y) {
		aABB.max.y = pointA.y;
		aABB.min.y = pointB.y;
	}
	else
	{
		aABB.max.y = pointB.y;
		aABB.min.y = pointA.y;
	}
	return aABB;
}

SHAPE_TYPE LineShape::GetType()
{
	return SHAPE_TYPE::LINE;
}

Shape* LineShape::Clone()
{
	return new LineShape(*this);
}

SausageShape::SausageShape(float radius, float height)
{
	this->radius = radius;
	pointA = Vector2(0, height / 2);
	pointB = -pointA;
}

float SausageShape::CalculateArea()
{
	return 2 * radius * glm::length(pointA - pointB) + radius * radius * glm::pi<float>();
}

void SausageShape::RenderShape(PhysicsProgram& program, Transform& transform)
{
	Vector2 a = transform.TransformPoint(pointA);
	Vector2 b = transform.TransformPoint(pointB);
	auto& lR = program.GetLineRenderer();
	lR.DrawCircle(a, radius, 32);
	lR.DrawCircle(b, radius, 32);

	Vector2 radiusAddition = glm::normalize(a - b);
	radiusAddition = { radiusAddition.y, -radiusAddition.x };

	lR.DrawLineSegment(pointA + radiusAddition, pointB + radiusAddition);
	lR.DrawLineSegment(pointA - radiusAddition, pointB - radiusAddition);
}

AABB SausageShape::CalculateAABB(Transform& transform)
{
	AABB aABB;
	if (pointA.x > pointB.x) {
		aABB.max.x = pointA.x + radius;
		aABB.min.x = pointB.x - radius;
	}
	else
	{
		aABB.max.x = pointB.x + radius;
		aABB.min.x = pointA.x - radius;
	}

	if (pointA.y > pointB.y) {
		aABB.max.y = pointA.y + radius;
		aABB.min.y = pointB.y - radius;
	}
	else
	{
		aABB.max.y = pointB.y + radius;
		aABB.min.y = pointA.y - radius;
	}
	return aABB;
}

SHAPE_TYPE SausageShape::GetType()
{
	return SHAPE_TYPE::CAPSULE;
}

Shape* SausageShape::Clone()
{
	return new SausageShape(*this);
}

void Shape::RenderShape(Transform transform, PhysicsProgram& program)
{
	RenderShape(program, transform);
}
