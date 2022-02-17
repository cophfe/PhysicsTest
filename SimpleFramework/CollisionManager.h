#pragma once
#include "PhysicsObject.h"
#include "Collision.h"
#include <vector>
#include <iostream>
class PhysicsProgram;

typedef bool (*CollideFunction)(CollisionData& data);

constexpr int DEFAULT_GRAVITY = 5;

//typedef void (*PhysicsDrawFunction)(Shape* shape, Transform& transform, void* infoPointer);
//struct PhysicsDrawer
//{
//	PhysicsDrawer() : callbacks({ DefaultDrawFunction, DefaultDrawFunction, DefaultDrawFunction, DefaultDrawFunction })
//	{
//		infoPointer = nullptr; drawingEnabled = false; 
//	}
//
//	PhysicsDrawer(PhysicsDrawFunction circle, PhysicsDrawFunction polygon, PhysicsDrawFunction capsule, PhysicsDrawFunction plane, void* infoPointer)
//		: infoPointer(infoPointer), drawingEnabled(true),
//		callbacks({ circle ? circle : DefaultDrawFunction, polygon ? polygon : DefaultDrawFunction, capsule ? capsule : DefaultDrawFunction, plane ? plane : DefaultDrawFunction })
//	{}
//
//	PhysicsDrawer& operator=(const PhysicsDrawer& other) 
//	{
//		memcpy(this, &other, sizeof(other));
//		return *this;
//	};
//
//	union {
//		struct {
//			const PhysicsDrawFunction circle;
//			const PhysicsDrawFunction polygon;
//			const PhysicsDrawFunction capsule;
//			const PhysicsDrawFunction plane;
//		} const callbacks;
//		const PhysicsDrawFunction shapes[4];
//	};
//
//	void CallDraw(Shape* shape, Transform& transform) 
//	{
//		int type = (int)shape->GetType();
//		(shapes[type])(shape, transform, infoPointer);
//	}
//
//	static void DefaultDrawFunction(Shape* shape, Transform& transform, void* infoPointer) {}
//
//	void* infoPointer;
//	bool drawingEnabled;
//};

class CollisionManager
{
public:
	CollisionManager(PhysicsProgram* program, float deltaTime, Vector2 gravity = Vector2{ 0, -DEFAULT_GRAVITY }) : program(program), deltaTime(deltaTime), gravity(gravity) {}
	PhysicsObject* PointCast(Vector2 point, bool includeStatic = false);

	void Update();
	//void DrawShapes();

	void ResolveCollisions();
	void UpdatePhysics();
	PhysicsObject* AddPhysicsObject(PhysicsObject* body);
	void ClearPhysicsBodies();


	inline float GetDeltaTime() { return deltaTime; }
	inline void SetDeltaTime(float newDeltaTime) { deltaTime = newDeltaTime; }

	//inline void SetPhysicsDrawer(PhysicsDrawer drawer) { this->drawer = drawer; };
	//inline PhysicsDrawer& GetPhysicsDrawer() { return drawer; };

	~CollisionManager();
	CollisionManager(const CollisionManager& other) = delete;
	CollisionManager& operator=(const CollisionManager& other) = delete;

private:
	bool CheckAABBCollision(AABB& a, AABB& b);
	
	void ResolveCollision(CollisionData& data);
	bool EvaluateCollision(CollisionData& data);

	//individual bodies could be accessed from other scripts, so this should mean they are kept in the same place no matter what
	std::vector<PhysicsObject*> bodies;
	std::vector<CollisionData> collisions;
	
	float deltaTime;
	Vector2 gravity;

	//for debug drawing
	//PhysicsDrawer drawer;
	PhysicsProgram* program;
	static CollideFunction collisionFunctions[4][4];
};


