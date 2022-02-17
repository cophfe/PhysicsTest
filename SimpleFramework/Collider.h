#pragma once
#include "Maths.h"
#include "Shape.h"
#include "LineRenderer.h"
class PhysicsProgram;
class PhysicsObject;
class CollisionManager;
class Transform;

//struct ShapeData 
//{
//	Shape* shape;
//	float iMass;
//	float iInertia;
//};

class Collider
{

public:
	Collider(Shape* shape, float density = 1);
	Collider(Shape** shapes, int shapeCount, float density = 1);

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
	//returns the applied offset (could be used to keep the collider's shapes in the same worldspace position
	Vector2 CentreShapeAboutZero();

	void SetAttached(PhysicsObject* attached) { this->attached = attached; }
	bool CanBeDynamic();
	PhysicsObject* attached;
	AABB aABB;
	float density;

	Shape** shapes;
	int shapeCount;
};