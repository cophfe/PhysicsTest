#include "Collision.h"
#include "ExtraMath.hpp"
#include "CollisionManager.h"
#include <iostream>

#pragma region Circle
bool CollisionManager::CollideCircleCircle(CollisionData& data)
{
	CircleShape* a = (CircleShape*)data.a->GetCollider(data.colliderIndexA).GetShape();
	CircleShape* b = (CircleShape*)data.b->GetCollider(data.colliderIndexB).GetShape();

	Vector2 pA = data.a->GetTransform().TransformPoint(a->centrePoint), pB = data.b->GetTransform().TransformPoint(b->centrePoint);
	Vector2 delta = pA - pB;
	float deltaMagSq = delta.x * delta.x + delta.y * delta.y;
	float radiusSum = (a->radius + b->radius);

	if (deltaMagSq < radiusSum * radiusSum)
	{
		deltaMagSq = sqrtf(deltaMagSq);
		data.penetration = radiusSum - deltaMagSq;
		data.collisionNormal = delta / deltaMagSq;
		data.collisionPoints[0] = pA - data.collisionNormal * a->radius;
		return true;
	}

	return false;
}

bool CollisionManager::CollideCircleCapsule(CollisionData& data)
{
	CircleShape* a = (CircleShape*)data.a->GetCollider(data.colliderIndexA).GetShape();
	CapsuleShape* b = (CapsuleShape*)data.b->GetCollider(data.colliderIndexB).GetShape();

	//transform to global
	Vector2 pA = data.b->GetTransform().TransformPoint(b->pointA), pB = data.b->GetTransform().TransformPoint(b->pointB);
	Vector2 circleCentre = data.a->GetTransform().TransformPoint(a->centrePoint);

	//circle radius + capsule radius
	float radius = a->radius + b->radius;

	//this basically simplifies the problem to a circle-circle collision
	Vector2 pointOnCapsuleLine = em::ClosestPointOnLine(pA, pB, circleCentre);

	Vector2 delta = circleCentre - pointOnCapsuleLine;
	if (delta.x * delta.x + delta.y * delta.y < radius * radius)
	{
		//is colliding
		data.collisionNormal = glm::normalize(delta); //collision normal is always from b to a
		data.penetration = radius - glm::dot(delta, data.collisionNormal); //p = a->radius + b->radius - distancebetweencentreandclosestpointonLine
		data.collisionPoints[0] = pointOnCapsuleLine + data.collisionNormal * b->radius;
		return true;
	}

	return false;
}

bool CollisionManager::CollideCirclePlane(CollisionData& data)
{
	CircleShape* a = (CircleShape*)data.a->GetCollider(data.colliderIndexA).GetShape();
	PlaneShape* b = (PlaneShape*)data.b->GetCollider(data.colliderIndexB).GetShape();

	//transform to global
	Vector2 centre = data.a->GetTransform().TransformPoint(a->centrePoint);
	Vector2 planeDirection = data.b->GetTransform().TransformDirection(b->normal);
	float planeDistance = glm::dot(data.b->GetTransform().TransformPoint(b->distance * b->normal), planeDirection);

	//calculate penetration
	float centreDot = glm::dot(centre, planeDirection);
	float penetration = centreDot - a->radius - planeDistance;

	//if p < 0, is colliding
	if (penetration < 0)
	{
		data.collisionNormal = planeDirection;
		data.penetration = -penetration;
		data.collisionPoints[0] = centre - planeDirection * (a->radius);
		return true;
	}

	return false;
}
#pragma endregion

#pragma region Polygon

#pragma region GJK Functions
Vector2 GetPerpendicularTowardOrigin(Vector2 a, Vector2 b)
{
	//triple cross method in 2d
	Vector2 delta = b - a;
	float cross = em::Cross(delta, -a);
	return glm::normalize(Vector2(-cross * delta.y, cross * delta.x));
}

bool ContainsOrigin(Vector2 a, Vector2 b, Vector2 c)
{
	Vector2 v = GetPerpendicularTowardOrigin(a, b);
	if (glm::dot(-a, v) > 0)
		return false;
	v = GetPerpendicularTowardOrigin(a, c);
	if (glm::dot(-a, v) < 0)
		return false;
	return true;
}

inline Vector2 GetSupport(Shape* a, Shape* b, Transform& tA, Transform& tB, Vector2 d)
{
	return a->Support(d, tA) - b->Support(-d, tB);
}

struct Simplex 
{
	Vector2 a, b, c;
	Vector2 dir;
};

//GJK function
//the final version of this function is based on this video: https://youtu.be/ajv46BSqcK4 and this page: https://dyn4j.org/2010/04/gjk-distance-closest-points/
static bool GJK(Shape* a, Shape* b, Transform& tA, Transform& tB, Simplex* finalSimplex)
{
	//GJK checks if the minkowski difference of two shapes encloses the origin or not.
	//if the minkowski difference does enclose the origin it means the two shapes are intersecting, because it means at least one point in the area inclosed by shape a is in the same position as a point in the area inclosed by shape b.
	//this algorythm is a very interesting abstraction of that idea that breaks down the enclosing the origin check into a small number of triangle enclosing the origin checks.
	//the triangles chosen are composed of 3 points on the minkowski difference, and those three points are chosen intelligently to minimise the tri checks
	Simplex tri;
	//first direction can be anything, but is often the direction from shape a to b (it is probably more efficient on average then a random direction, idk)
	tri.dir = glm::normalize(tB.position - tA.position);
	
	//get furthest point on the minkowski difference in the direction of tri.dir
	tri.a = GetSupport(a, b, tA, tB, tri.dir);
	//the best next direction to choose is towards the origin
	tri.dir = glm::normalize(- tri.a);
	//get furthest point in the direction of the origin from point a
	tri.b = GetSupport(a, b, tA, tB, tri.dir);

	//line case
	//if point b is not on the opposite side of the origin from point a, the minkowski difference does not enclose the origin and the shapes aren't colliding
	if (glm::dot(tri.b, tri.dir) < 0)
		return false;

	//from now on the new support point direction will be perpendicular to point a and b, towards the origin. 
	//This is because all the points on the other side of the line AB are further away from the origin than
	tri.dir = GetPerpendicularTowardOrigin(tri.a, tri.b);

	while (true)
	{
		//at this point in the loop, tri.c is always undefined.
		tri.c = GetSupport(a, b, tA, tB, tri.dir);

		if (glm::dot(tri.c, tri.dir) < 0)
			return false;

		//check if inside the triangle defined by a,b,c
		//this method is based on voronoi regions and uses our previous knowledge about the shape to simplify the calculations

		//check if point is in voronoi region defined by line CB
		Vector2 v = GetPerpendicularTowardOrigin(tri.c, tri.b);
		if (glm::dot(v, tri.c) > 0)
		{
			//in this case, origin is in the region in the direction v from the triangle we created.
			//set the tri.dir vector to this direction, and remove point a from the triangle
			tri.dir = v;
			tri.a = tri.b;
			tri.b = tri.c;
			continue;
		}
		//check if point is in voronoi region defined by line CA
		v = GetPerpendicularTowardOrigin(tri.c, tri.a);
		if (glm::dot(v, tri.c) > 0)
		{
			//in this case, origin is in the region in the direction v from the triangle we created.
			//set the tri.dir vector to this direction, and remove point b from the triangle
			tri.dir = v;
			tri.b = tri.c;
			continue;
		}
		//if not in those regions, the triangle MUST contain the origin, so it it colliding
		if (finalSimplex != nullptr)
			*finalSimplex = tri; //return final simplex for EPA

		return true;
	}
}

struct Edge {
	Vector2 a, b;
};

Edge FindClosestEdge()
{
	
}

void EPA(Simplex gjkTri, Shape* a, Shape* b, Transform& tA, Transform& tB)
{

}

#pragma endregion

bool CollisionManager::CollideCirclePolygon(CollisionData& data)
{
	CircleShape* a = (CircleShape*)data.a->GetCollider(data.colliderIndexA).GetShape();
	PolygonShape* b = (PolygonShape*)data.b->GetCollider(data.colliderIndexB).GetShape();

	Simplex tri;
	if (GJK(a, b, data.a->transform, data.b->transform, &tri))
	{
		Vector2 circlePoint = data.b->GetTransform().TransformPoint(a->centrePoint);

	}

	return false;
}

bool CollisionManager::CollidePolygonPolygon(CollisionData& data)
{
	PolygonShape* a = (PolygonShape*)data.a->GetCollider(data.colliderIndexA).GetShape();
	PolygonShape* b = (PolygonShape*)data.b->GetCollider(data.colliderIndexB).GetShape();

	/*std::cout << "Do polygons collide: " << Intersection(a, b, data.a->transform, data.b->transform, nullptr)
		<< std::endl;*/

	Simplex tri;
	if (GJK(a, b, data.a->transform, data.b->transform, &tri))
	{
		/*while (true)
		{*/
			struct
			{
				Vector2 start,
					end;
			} closestEdge;

			std::cout << "Intersection: (point A), penetration: " << glm::length(tri.a) <<
			"\n\t(point B), penetration: " << glm::dot(tri.b) <<
			"(point C), penetration: " << glm::length(tri.c) << "\n";
				
		//}
	}
	else
	{
		std::cout << "Intersection: 0\n";

	}

	return false;
}

bool CollisionManager::CollidePolygonCapsule(CollisionData& data)
{
	PolygonShape* a = (PolygonShape*)data.a->GetCollider(data.colliderIndexA).GetShape();
	CapsuleShape* b = (CapsuleShape*)data.b->GetCollider(data.colliderIndexB).GetShape();

	//PolygonShape* a = (PolygonShape*)data.a->GetCollider(data.colliderIndexA).GetShape();
	//CapsuleShape* b = (CapsuleShape*)data.b->GetCollider(data.colliderIndexB).GetShape();

	//Vector2 stadPointA = data.b->GetTransform().TransformPoint(b->pointA),
	//	stadPointB = data.b->GetTransform().TransformPoint(b->pointB);
	////Vector2 stadTangent = glm::normalize(stadPointA - stadPointB);
	////Vector2 stadNormal = { -stadTangent.y, stadTangent.x };

	//float minDistanceSqr = b->radius * b->radius + 1;
	//Vector2 collisionPoint;
	//Vector2 normal;

	//for (size_t i = 0; i < a->pointCount; i++)
	//{
	//	Vector2 point = data.a->GetTransform().TransformPoint(a->points[i]);
	//	Vector2 linePoint = em::ClosestPointOnLine(stadPointA, stadPointB, point);
	//	float distSqr = em::SquareLength(point - linePoint);

	//	if (distSqr < minDistanceSqr)
	//	{
	//		collisionPoint = linePoint;
	//		minDistanceSqr = distSqr;
	//		normal = point - linePoint;
	//	}
	//}

	//if (minDistanceSqr < b->radius * b->radius)
	//{
	//	minDistanceSqr = sqrtf(minDistanceSqr);

	//	data.penetration = b->radius - minDistanceSqr;
	//	data.collisionNormal = normal / minDistanceSqr;
	//	data.collisionPoints[0] = collisionPoint + data.collisionNormal * b->radius; // this is wrong!!!!
	//	return true;
	//}


	return false;
}

bool CollisionManager::CollidePolygonPlane(CollisionData& data) 
{
	PolygonShape* a = (PolygonShape*)data.a->GetCollider(data.colliderIndexA).GetShape();
	PlaneShape* b = (PlaneShape*)data.b->GetCollider(data.colliderIndexB).GetShape();

	Vector2 planeNormal = data.b->GetTransform().TransformPoint(b->normal);
	float planeDistance = glm::dot(data.b->GetTransform().TransformPoint(b->distance * b->normal), planeNormal);

	float minPenetration = 1;
	Vector2 collisionPoint;
	for (size_t i = 0; i < a->pointCount; i++)
	{
		Vector2 point = data.a->GetTransform().TransformPoint(a->points[i]);
		float p = glm::dot(point, planeNormal) - planeDistance;

		if (p < minPenetration)
		{
			collisionPoint = point;
			minPenetration = p;
		}
	}

	if (minPenetration < 0)
	{
		data.collisionNormal = planeNormal;
		data.penetration = -minPenetration;
		data.collisionPoints[0] = collisionPoint;
		return true;
	}
	return false;
}
#pragma endregion

#pragma region Capsule
bool CollisionManager::CollideCapsuleCapsule(CollisionData& data)
{
	CapsuleShape* a = (CapsuleShape*)data.a->GetCollider(data.colliderIndexA).GetShape();
	CapsuleShape* b = (CapsuleShape*)data.b->GetCollider(data.colliderIndexB).GetShape();

	Vector2 aPointA = data.a->GetTransform().TransformPoint(a->pointA),
		aPointB = data.a->GetTransform().TransformPoint(a->pointB)
		, bPointA = data.b->GetTransform().TransformPoint(b->pointA),
		bPointB = data.b->GetTransform().TransformPoint(b->pointB);;

	//stadium checking comes down to 4 point-line distance checks, an intersection test, and 4 point normal tests

	Vector2 intersection;
	if (em::CalculateIntersectionPoint(aPointA, aPointB, bPointA, bPointB, intersection))
	{
		Vector2 aTangent = glm::normalize(aPointB - aPointA);
		Vector2 bTangent = glm::normalize(bPointB - bPointA);

		Vector2 aNormal = { -aTangent.y, aTangent.x };
		Vector2 bNormal = { -bTangent.y, bTangent.x };

		//this is essentially 4 point-plane checks
		//the point with the smallest penetration wins, and the capsule will be pushed out along that axis
		Vector2 normal = aNormal;
		float aPoint = glm::dot(aNormal, aPointA);
		float bPoint = glm::dot(bNormal, bPointA);
		float p1 = aPoint - glm::dot(aNormal, bPointA);
		float p2 = aPoint - glm::dot(aNormal, bPointB);

		if (p1 * p1 > p2 * p2)
			p1 = p2;

		p2 = glm::dot(bNormal, aPointA) - bPoint;
		if (p1 * p1 > p2 * p2)
		{
			normal = bNormal;
			p1 = p2;
		}

		p2 = glm::dot(bNormal, aPointB) - bPoint;

		if (p1 * p1 > p2 * p2)
		{
			p1 = p2;
			normal = bNormal;
		}

		//since we know they are intersecting there is no need to perform a check here
		float sign = glm::sign(p1); //<-- could probably get rid of this somehow with the triple cross product check thing
		data.collisionNormal = sign * -normal;
		data.penetration = sign * p1 + a->radius + b->radius;
		data.collisionPoints[0] = intersection + data.collisionNormal * a->radius;
		return true;
	}
	else
	{
		//this is essentially 4 different circle checks
		//we will use the one with the smallest distance between circle centres

		struct {
			Vector2 collisionDelta;
			float collisionDistanceSq;
		} collision, newCollision;

		float collisionMul = -a->radius;
		Vector2 collisionPoint = aPointA;
		collision.collisionDelta = aPointA - em::ClosestPointOnLine(bPointA, bPointB, aPointA);
		collision.collisionDistanceSq = em::SquareLength(collision.collisionDelta);

		newCollision.collisionDelta = aPointB - em::ClosestPointOnLine(bPointA, bPointB, aPointB);
		newCollision.collisionDistanceSq = em::SquareLength(newCollision.collisionDelta);
		if (collision.collisionDistanceSq > newCollision.collisionDistanceSq)
		{
			collision = newCollision;
			collisionPoint = aPointB;
		}

		newCollision.collisionDelta = em::ClosestPointOnLine(aPointA, aPointB, bPointA) - bPointA;
		newCollision.collisionDistanceSq = em::SquareLength(newCollision.collisionDelta);
		if (collision.collisionDistanceSq > newCollision.collisionDistanceSq)
		{
			collision = newCollision;
			collisionPoint = bPointA;
			collisionMul = b->radius;
		}

		newCollision.collisionDelta = em::ClosestPointOnLine(aPointA, aPointB, bPointB) - bPointB;
		newCollision.collisionDistanceSq = em::SquareLength(newCollision.collisionDelta);
		if (collision.collisionDistanceSq > newCollision.collisionDistanceSq)
		{
			collision = newCollision;
			collisionPoint = bPointB;
			collisionMul = b->radius;
		}

		float radius = a->radius + b->radius;

		//if the smallest collision distance is less than the sum of the radii, it is colliding
		if (collision.collisionDistanceSq < radius * radius)
		{
			float length = sqrtf(collision.collisionDistanceSq);
			data.penetration = radius - length;
			data.collisionNormal = collision.collisionDelta / length;
			data.collisionPoints[0] = collisionPoint + data.collisionNormal * collisionMul;
			return true;
		}
	}

	return false;
}

bool CollisionManager::CollideCapsulePlane(CollisionData& data)
{
	CapsuleShape* a = (CapsuleShape*)data.a->GetCollider(data.colliderIndexA).GetShape();
	PlaneShape* b = (PlaneShape*)data.b->GetCollider(data.colliderIndexB).GetShape();

	Vector2 pointA = data.a->GetTransform().TransformPoint(a->pointA), pointB = data.a->GetTransform().TransformPoint(a->pointB);
	Vector2 planeDirection = data.b->GetTransform().TransformDirection(b->normal);
	float planeDistance = glm::dot(data.b->GetTransform().TransformPoint(b->distance * b->normal), planeDirection);

	Vector2 collisionPoint = pointA;
	float start = glm::dot(pointA, planeDirection);
	float end = glm::dot(pointB, planeDirection);
	if (start > end)
	{
		float temp = start;
		start = end;
		end = temp;
		collisionPoint = pointB;
	}

	start -= a->radius;
	end += a->radius;

	//if colliding
	if (planeDistance >= start)
	{
		data.collisionNormal = planeDirection;
		data.penetration = planeDistance - start;
		data.collisionPoints[0] = collisionPoint - planeDirection * a->radius;
		return true;
	}

	return false;
}

bool CollisionManager::CollideInvalid(CollisionData& data)
{
	return false;
}
#pragma endregion

#pragma region Flipped Functions

bool CollisionManager::CollidePolygonCircle(CollisionData& data)
{
	auto temp = data.a;
	data.a = data.b;
	data.b = temp;
	return CollideCirclePolygon(data);
}

bool CollisionManager::CollideCapsuleCircle(CollisionData& data)
{
	auto temp = data.a;
	data.a = data.b;
	data.b = temp;
	return CollideCircleCapsule(data);
}

bool CollisionManager::CollidePlaneCircle(CollisionData& data)
{
	auto temp = data.a;
	data.a = data.b;
	data.b = temp;
	return CollideCirclePlane(data);
}

bool CollisionManager::CollideCapsulePolygon(CollisionData& data)
{
	auto temp = data.a;
	data.a = data.b;
	data.b = temp;
	return CollidePolygonCapsule(data);
}

bool CollisionManager::CollidePlanePolygon(CollisionData& data)
{
	auto temp = data.a;
	data.a = data.b;
	data.b = temp;
	return CollidePolygonPlane(data);
}

bool CollisionManager::CollidePlaneCapsule(CollisionData& data)
{
	auto temp = data.a;
	data.a = data.b;
	data.b = temp;
	return CollideCapsulePlane(data);
}
#pragma endregion
