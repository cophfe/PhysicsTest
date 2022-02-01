#include "Collider.h"
#include "PhysicsProgram.h"
#include "PhysicsObject.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// POLYGON COLLIDER
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PolygonCollider::PolygonCollider(Vector2* vertices, int vertexCount, Vector2 centrePoint, PhysicsObject *attached)
{
	points = vertices;
	//memcpy(points, vertices, sizeof(Vector2) * vertexCount);
	pointCount = vertexCount;
	this->centrePoint = centrePoint;
	globalPoints = new Vector2[pointCount];
	this->attached = attached;
	CalculateGlobal();
}
void PolygonCollider::RenderShape(PhysicsProgram& program)
{
	program.GetLineRenderer().DrawRawShape((float*)globalPoints, pointCount);
}

float PolygonCollider::CalculateArea()
{
	//needs to be multipled by scale.x, scale.y & density
	float area = 0;

	for (size_t i = 0; i < pointCount - 1; i++)
	{
		area += centrePoint.x* (points[i].y - points[i + 1].y) + points[i].x * (points[i + 1].y - centrePoint.y) + points[i + 1].x * (centrePoint.y - points[i].y);
	}
	area += centrePoint.x* (points[pointCount - 1].y - points[0].y) + points[pointCount - 1].x * (points[0].y - centrePoint.y) + points[0].x * (centrePoint.y - points[pointCount - 1].y);
	area *= 0.5f;

	return area;
}

void PolygonCollider::CalculateGlobal()
{
	Matrix2x2 &mat = attached->GetTransform();
	globalCentrePoint = centrePoint * mat;

	for (size_t i = 0; i < pointCount; i++)
	{
		globalPoints[i] = points[i] * mat + globalCentrePoint;
	}
}

void PolygonCollider::CalculateAABB()
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

	aABB.max.x = xMax;
	aABB.max.y = yMax;

	aABB.min.x = xMin;
	aABB.min.y = yMin;
}

PolygonCollider&& PolygonCollider::GetRegularPolygonCollider(float radius, int pointCount, PhysicsObject* attached)
{
	Vector2 *points = new Vector2[pointCount];

	float iPointCount = 1.0f / pointCount;
	for (size_t i = 0; i < pointCount; i++)
	{
		points[i] = Vector2(radius * cos(i * iPointCount), radius * sin(i * iPointCount));
	}

	return PolygonCollider(points, pointCount, Vector2(0, 0), attached);
}





//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// CIRCLE COLLIDER
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CircleCollider::CircleCollider(float radius, Vector2 centrePoint, PhysicsObject* attached)
{
	this->radius = radius;
	this->centrePoint = centrePoint;
	this->attached = attached;

	CalculateGlobal();
}

void CircleCollider::RenderShape(PhysicsProgram& program)
{
	program.GetLineRenderer().DrawCircle(attached->GetPosition() + centrePoint, radius);
}

float CircleCollider::CalculateArea()
{
	return radius * radius * glm::pi<float>();
}

void CircleCollider::CalculateGlobal()
{
	Matrix2x2& mat = attached->GetTransform();
	globalCentrePoint = centrePoint * mat;
	globalRadius = radius * attached->GetScale();

}

void CircleCollider::CalculateAABB()
{
	aABB.max.x = globalCentrePoint.x + globalRadius;
	aABB.max.y = globalCentrePoint.y + globalRadius;

	aABB.min.x = globalCentrePoint.x - globalRadius;
	aABB.min.y = globalCentrePoint.y - globalRadius;
}
