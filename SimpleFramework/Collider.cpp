#include "Collider.h"
#include "PhysicsProgram.h"
#include "CollisionManager.h"
#include "PhysicsObject.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// POLYGON COLLIDER
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//PolygonCollider::PolygonCollider(Vector2* vertices, int vertexCount, Vector2 centrePoint, PhysicsObject *attached)
//{
//	points = vertices;
//	//memcpy(points, vertices, sizeof(Vector2) * vertexCount);
//	pointCount = vertexCount;
//	this->centrePoint = centrePoint;
//	globalPoints = new Vector2[pointCount];
//	this->attached = attached;
//	CalculateGlobal();
//}
//void PolygonCollider::RenderShape(PhysicsProgram& program)
//{
//	program.GetLineRenderer().DrawRawShape((float*)globalPoints, pointCount);
//}
//
//void PolygonCollider::CalculateGlobal()
//{
//	Matrix2x2 &mat = attached->GetScaleRotationMatrix();
//	globalCentrePoint = centrePoint * mat;
//
//	for (size_t i = 0; i < pointCount; i++)
//	{
//		globalPoints[i] = points[i] * mat + globalCentrePoint;
//	}
//}
//
//
//
//
////~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//// CIRCLE COLLIDER
////~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//CircleCollider::CircleCollider(float radius, Vector2 centrePoint, PhysicsObject* attached)
//{
//	this->radius = radius;
//	this->centrePoint = centrePoint;
//	this->attached = attached;
//
//	CalculateGlobal();
//}
//
//void CircleCollider::RenderShape(PhysicsProgram& program)
//{
//	program.GetLineRenderer().DrawCircle(attached->GetPosition() + centrePoint, radius);
//}
//
//void CircleCollider::CalculateGlobal()
//{
//	Matrix2x2& mat = attached->GetScaleRotationMatrix();
//	globalCentrePoint = centrePoint * mat;
//	globalRadius = radius * attached->GetScale();
//
//}

Collider::Collider(Shape* shape, float density, PhysicsObject* attached)
{
}

Collider::Collider(Shape** shapes, int shapeCount, float density, PhysicsObject* attached)
{
}

float Collider::CalculateInertia()
{
	return 0.0f;
}

float Collider::CalculateMass()
{
	float mass = 0;
	for (size_t i = 0; i < shapeCount; i++)
	{
		mass += shapes[i]->CalculateArea() * density;
	}
	return mass;
}

void Collider::CalculateAABB(Transform& transform)
{
	switch (shapeCount) {
	case 0:
		return;
	case 1:
		aABB = shapes[0]->CalculateAABB(transform);
		return;
	default:
	{
		AABB* aabbs = new AABB[shapeCount];
		aabbs[0] = shapes[0]->CalculateAABB(transform);
		aABB = aabbs[0];

		for (size_t i = 1; i < shapeCount; i++)
		{
			aabbs[i] = shapes[i]->CalculateAABB(transform);

			if (aabbs[i].max.x > aABB.max.x)
				aABB.max.x = aabbs[i].max.x;
			if (aabbs[i].min.x < aABB.min.x)
				aABB.min.x = aabbs[i].min.x;
			if (aabbs[i].max.y > aABB.max.y)
				aABB.max.y = aabbs[i].max.y;
			if (aabbs[i].min.y > aABB.min.y)
				aABB.min.y = aabbs[i].min.y;
		}
	}
	}
	
}

Collider::~Collider()
{
	for (size_t i = 0; i < shapeCount; i++)
	{
		delete shapes[i];
		shapes[i] = nullptr;
	}
	delete[] shapes;
	shapes = nullptr;
}

Collider::Collider(Collider& other)
{
	aABB = other.aABB;
	shapeCount = other.shapeCount;
	attached = other.attached;
	density = other.density;

	shapes = new Shape*[shapeCount];
	for (size_t i = 0; i < shapeCount; i++)
	{
		shapes[i] = other.shapes[i]->Clone();
	}
}

Collider& Collider::operator=(Collider& other)
{
	for (size_t i = 0; i < shapeCount; i++)
	{
		delete shapes[i];
	}
	delete[] shapes;

	aABB = other.aABB;
	shapeCount = other.shapeCount;
	attached = other.attached;
	density = other.density;

	shapes = new Shape * [shapeCount];
	for (size_t i = 0; i < shapeCount; i++)
	{
		shapes[i] = other.shapes[i]->Clone();
	}

	return *this;
}


