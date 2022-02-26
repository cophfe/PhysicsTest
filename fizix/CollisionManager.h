#pragma once
#include "fzx.h"
#include "Collision.h"

namespace fzx
{
	typedef bool (*CollideFunction)(fzx::CollisionData& data);

	constexpr int DEFAULT_GRAVITY = 5;
	constexpr int COLLISION_ITERATIONS = 10;

	class CollisionManager
	{
	public:
		CollisionManager(float deltaTime, Vector2 gravity = Vector2{ 0, -DEFAULT_GRAVITY }) : deltaTime(deltaTime), gravity(gravity) {}
		
		PhysicsObject* PointCast(Vector2 point, bool includeStatic = false, bool includeTriggers = false);

		void Update();
		//void DrawShapes();

		void ResolveCollisions();
		void UpdatePhysics();
		PhysicsObject* CreatePhysicsObject(PhysicsData& data);
		void DeletePhysicsBody(PhysicsObject* body);
		void ClearPhysicsBodies();

		inline float GetDeltaTime() { return deltaTime; }
		inline void SetDeltaTime(float newDeltaTime) { deltaTime = newDeltaTime; }

		inline void SetCollisionCallback(CollisionCallback callback, void* infoPointer) { this->cCallback = callback; cCallbackPtr = infoPointer; };
		inline CollisionCallback GetCollisionCallback() { return cCallback; };

		//inline void SetPhysicsDrawer(PhysicsDrawer drawer) { this->drawer = drawer; };
		//inline PhysicsDrawer& GetPhysicsDrawer() { return drawer; };

		//seperate function from destructor just so it is clear what order things are destroyed in
		void Destroy();
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

		static CollideFunction collisionFunctions[4][4];

		//called when two objects are colliding. If this returns false, the collision will not be evaluated.
		CollisionCallback cCallback = nullptr;
		void* cCallbackPtr = nullptr;

		//return true if collision occured
		static bool CollideCircleCircle(CollisionData& data);
		static bool CollideCirclePolygon(CollisionData& data);
		static bool CollideCircleCapsule(CollisionData& data);
		static bool CollideCirclePlane(CollisionData& data);
		static bool CollidePolygonPolygon(CollisionData& data);
		static bool CollidePolygonCapsule(CollisionData& data);
		static bool CollidePolygonPlane(CollisionData& data);
		static bool CollideCapsuleCapsule(CollisionData& data);
		static bool CollideCapsulePlane(CollisionData& data);
		static bool CollideInvalid(CollisionData& data);

		//flipped functions
		static bool CollidePolygonCircle(CollisionData& data);
		static bool CollideCapsuleCircle(CollisionData& data);
		static bool CollidePlaneCircle(CollisionData& data);
		static bool CollideCapsulePolygon(CollisionData& data);
		static bool CollidePlanePolygon(CollisionData& data);
		static bool CollidePlaneCapsule(CollisionData& data);

	};
}