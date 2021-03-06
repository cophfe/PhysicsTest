#pragma once
#include "Maths.h"

#ifndef FZX_MAX_VERTICES
#define FZX_MAX_VERTICES 8
#endif // !FZX_MAX_VERTICES

namespace fzx
{
	class Transform;
	class PhysicsSystem;

	struct AABB 
	{
	public:
		Vector2 max;
		Vector2 min;

		bool PointCast(Vector2 point)
		{
			return point.x < max.x&& point.x > min.x
				&& point.y < max.y&& point.y > min.y;
		}
	};

	enum class SHAPE_TYPE : unsigned char {
		CIRCLE,
		POLYGON,
		CAPSULE,
		PLANE,
		COUNT
	};



	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// BASE CLASS
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class Shape
	{
	public:
		virtual bool PointCast(Vector2 point, Transform& transform) = 0;
		virtual void CalculateMass(float& mass, float& inertia, float density) = 0;
		virtual Vector2 GetCentrePoint() = 0;
		virtual AABB CalculateAABB(Transform& transform) = 0;
		virtual SHAPE_TYPE GetType() = 0;
		virtual Shape* Clone() = 0;
		virtual Vector2 Support(Vector2 v, Transform& transform) = 0;

		virtual ~Shape() = default;
	private:
		friend PhysicsSystem;

	};

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// POLYGON CLASS
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	class PolygonShape : public Shape
	{
	public:

		PolygonShape(Vector2* vertices, int vertexCount);

		bool PointCast(Vector2 point, Transform& transform);
		void CalculateMass(float& mass, float& inertia, float density);
		Vector2 GetCentrePoint();
		AABB CalculateAABB(Transform& transform);
		SHAPE_TYPE GetType();
		Shape* Clone();
		Vector2 Support(Vector2 v, Transform& transform);

		static PolygonShape* GetRegularPolygonCollider(float radius, int pointCount);
		Vector2 points[FZX_MAX_VERTICES];
		//Vector2 normals[max_vertices];
		char pointCount;
		Vector2 centrePoint;

		~PolygonShape() = default;

	private:
		friend PhysicsSystem;
		//void CalculateNormals();
		void CalculateCentrePoint();
		//clip points clips the points if they go over the max vertex count, instead of throwing an error
		bool OrganisePoints(Vector2* points, int pointCount, bool clipPoints = true);


	};

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// CIRCLE CLASS
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	class CircleShape : public Shape
	{
	public:
		CircleShape(float radius, Vector2 centrePoint);

		bool PointCast(Vector2 point, Transform& transform);
		void CalculateMass(float& mass, float& inertia, float density);
		Vector2 GetCentrePoint();
		AABB CalculateAABB(Transform& transform);
		SHAPE_TYPE GetType();
		Shape* Clone();
		Vector2 Support(Vector2 v, Transform& transform);

		float radius;
		Vector2 centrePoint;

		~CircleShape() = default;

	private:
		friend PhysicsSystem;

	};

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// STADIUM CLASS
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	class CapsuleShape : public Shape
	{
	public:

		CapsuleShape(Vector2 a, Vector2 b, float radius = 0.01f);

		bool PointCast(Vector2 point, Transform& transform);
		void CalculateMass(float& mass, float& inertia, float density); // change to 
		AABB CalculateAABB(Transform& transform);
		Vector2 GetCentrePoint();
		SHAPE_TYPE GetType();
		Shape* Clone();
		Vector2 Support(Vector2 v, Transform& transform);

		float radius;
		Vector2 pointA;
		Vector2 pointB;

		~CapsuleShape() = default;

	private:
		friend PhysicsSystem;

	};

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// PLANE CLASS
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	class PlaneShape : public Shape
	{
	public:
		PlaneShape(Vector2 normal, float d);
		PlaneShape(Vector2 normal, Vector2 startPosition);
		PlaneShape(Vector2 pointA, Vector2 pointB, void* null);

		bool PointCast(Vector2 point, Transform& transform);
		void CalculateMass(float& mass, float& inertia, float density);
		Vector2 GetCentrePoint();
		AABB CalculateAABB(Transform& transform);
		SHAPE_TYPE GetType();
		Shape* Clone();
		Vector2 Support(Vector2 v, Transform& transform);

		Vector2 normal;
		float distance;

		~PlaneShape() = default;
	private:
		friend PhysicsSystem;
	};
}