#pragma once
#include "Maths.h"
#include "Shape.h"
#include "LineRenderer.h"
class PhysicsProgram;
class PhysicsObject;
class CollisionManager;
class Transform;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// BASE COLLIDER CLASS
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class Collider
{

public:
	Collider(Shape* shape, float density = 1, Vector3 colour = Vector3(1.0f, 1.0f, 1.0f));
	Collider(Shape** shapes, int shapeCount, float density = 1, Vector3 colour = Vector3(1.0f, 1.0f, 1.0f));

	void RenderShape(PhysicsProgram& program);

	//calculaters
	void CalculateMass(float& massVar, float& inertiaVar);
	void CalculateAABB(Transform& transform);

	Shape* GetShape(int index)	{ return shapes[index]; }
	int GetShapeCount()			{ return shapeCount; }

	//void AddShape(Shape* shape);
	//void ResetShapes(Shape* shape); //<< collider has to have at least one shape

	~Collider();
	Collider(Collider& other);
	Collider& operator=(Collider& other);

private:
	friend CollisionManager;
	friend PhysicsObject;
	
	//centres collider about 0,0 (for rotation reasons, since object always rotates around local coord (0,0))
	//returns the offset from rotation
	Vector2 CentreShapeAboutZero();

	void SetAttached(PhysicsObject* attached) { this->attached = attached; }
	bool CanBeDynamic();
	PhysicsObject* attached;
	AABB aABB;
	float density;

	Shape** shapes;
	int shapeCount;

	Vector3 colour;
};