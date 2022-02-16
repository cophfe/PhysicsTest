#pragma once
#include "PhysicsObject.h"
#include "Collision.h"
#include <vector>
#include <iostream>
class PhysicsProgram;

typedef bool (*CollideFunction)(CollisionData& data);

constexpr int DEFAULT_GRAVITY = 5;
class CollisionManager
{
public:
	CollisionManager(PhysicsProgram* program, float deltaTime, Vector2 gravity = Vector2{ 0, -DEFAULT_GRAVITY }) : program(program), deltaTime(deltaTime), gravity(gravity) {}
	PhysicsObject* PointCast(Vector2 point, bool includeStatic = false);

	void Update();
	void DrawShapes();

	void ResolveCollisions();
	void UpdatePhysics();
	PhysicsObject& AddPhysicsObject(PhysicsObject&& body);
	PhysicsObject& AddPhysicsObject(PhysicsObject body);
	void ClearPhysicsBodies();


	inline float GetDeltaTime() { return deltaTime; }
	inline void SetDeltaTime(float newDeltaTime) { deltaTime = newDeltaTime; }
private:
	bool CheckAABBCollision(AABB& a, AABB& b);
	
	void ResolveCollision(CollisionData& data);
	bool EvaluateCollision(CollisionData& data);

	std::vector<PhysicsObject> bodies;
	std::vector<CollisionData> collisions;
	
	float deltaTime;
	Vector2 gravity;

	//for debug drawing
	PhysicsProgram* program;
	static CollideFunction collisionFunctions[4][4];
};


