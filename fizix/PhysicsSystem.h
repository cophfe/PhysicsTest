#pragma once
#include "fzx.h"
#include "Collision.h"

namespace fzx
{
	typedef bool (*CollideFunction)(fzx::CollisionData& data);

	constexpr int FZX_DEFAULT_GRAVITY = 5;
	constexpr int FZX_DEFAULT_COLLISION_ITERATIONS = 1;

	class PhysicsSystem
	{
	public:
		PhysicsSystem(float deltaTime, Vector2 gravity = Vector2{ 0, -FZX_DEFAULT_GRAVITY }, int collisionIterations = FZX_DEFAULT_COLLISION_ITERATIONS) : deltaTime(deltaTime), gravity(gravity), collisionIterations(collisionIterations) {}
		
		PhysicsObject* PointCast(Vector2 point, bool includeStatic = false, bool includeTriggers = false, short collisionMask = 0xFFFF);
		std::vector<PhysicsObject*>&& PointCastMultiple(Vector2 point, bool includeStatic = false, bool includeTriggers = false, short collisionMask = 0xFFFF);

		void Update();
		
		PhysicsObject* CreatePhysicsObject(PhysicsData& data);
		void DeletePhysicsBody(PhysicsObject* body);
		void ClearPhysicsBodies();

		inline float GetDeltaTime() { return deltaTime; }
		inline void SetDeltaTime(float newDeltaTime) { deltaTime = newDeltaTime; }

		inline void SetCollisionCallback(CollisionCallback callback, void* infoPointer) { this->cCallback = callback; cCallbackPtr = infoPointer; };
		inline CollisionCallback GetCollisionCallback() { return cCallback; };

		//seperate function from destructor just so it is clear what order things are destroyed in
		~PhysicsSystem();
		PhysicsSystem(const PhysicsSystem& other) = delete;
		PhysicsSystem& operator=(const PhysicsSystem& other) = delete;

	private:

		void ResolveCollisions();
		void UpdatePhysics();
		
		bool CheckAABBCollision(AABB& a, AABB& b);

		void ResolveCollision(CollisionData& data);
		bool EvaluateCollision(CollisionData& data);

		//individual bodies could be accessed from other scripts, so this should mean they are kept in the same place no matter what
		std::vector<PhysicsObject*> bodies;
		std::vector<CollisionData> collisions;

		float deltaTime;
		Vector2 gravity;
		const int collisionIterations;

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

		//epa + gjk + other stuff
		struct Simplex
		{
			Vector2 a, b, c;
			Vector2 dir;
		};
		struct EPACollisionData
		{
			float depth;
			Vector2 collisionNormal;
		};
		struct PolygonEdge {
			Vector2 pA,
				pB;

			Vector2 maxProjectionVertex;
		};
		struct ClipInfo
		{
			Vector2 points[2];
			int pointCount;
		};


		static Vector2 GetPerpendicularTowardOrigin(Vector2 a, Vector2 b);
		static Vector2 GetPerpendicularFacingInDirection(Vector2 line, Vector2 direction);
		static Vector2 GetSupport(Shape* a, Shape* b, Transform& tA, Transform& tB, Vector2 d);
		static	Vector2 ClosestPointToOrigin(Vector2 a, Vector2 b);
		static	bool GJK(Shape* a, Shape* b, Transform& tA, Transform& tB, Simplex* finalSimplex);
		static	bool EPA(Shape* a, Shape* b, Transform& tA, Transform& tB, EPACollisionData* data);
		static	PolygonEdge FindPolygonCollisionEdge(PolygonShape* pS, Transform& t, Vector2 normal);
		static	ClipInfo Clip(Vector2 pointToClip1, Vector2 pointToClip2, Vector2 clippingNormal, float clipDist);
	};
}