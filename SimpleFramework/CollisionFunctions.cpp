#include "Collision.h"
#include "ExtraMath.hpp"

bool CollideCircleCircle(CollisionData& data)
{
	CircleShape* a = (CircleShape*)data.a->GetCollider()->GetShape(data.shapeIndexA);
	CircleShape* b = (CircleShape*)data.b->GetCollider()->GetShape(data.shapeIndexB);

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

bool CollideCirclePolygon(CollisionData& data)
{
	CircleShape* a = (CircleShape*)data.a->GetCollider()->GetShape(data.shapeIndexA);
	PolygonShape* b = (PolygonShape*)data.b->GetCollider()->GetShape(data.shapeIndexB);

	Vector2 circlePoint = data.b->GetTransform().TransformPoint(a->centrePoint);

	float minDistanceSqr = a->radius * a->radius;
	Vector2 normal;

	for (size_t i = 0; i < b->pointCount; i++)
	{
		Vector2 point = data.a->GetTransform().TransformPoint(b->points[i]);
		float distSqr = em::SquareLength(point - circlePoint);

		if (distSqr < minDistanceSqr)
		{
			minDistanceSqr = distSqr;
			normal = point - circlePoint;
		}
	}

	if (minDistanceSqr < a->radius * a->radius)
	{
		minDistanceSqr = sqrtf(minDistanceSqr);
		data.penetration = a->radius - minDistanceSqr;
		data.collisionNormal = normal / minDistanceSqr;
		data.collisionPoints[0] = circlePoint + data.collisionNormal * a->radius;
		return true;
	}
	return false;
}

bool CollideCircleCapsule(CollisionData& data)
{
	CircleShape* a = (CircleShape*)data.a->GetCollider()->GetShape(data.shapeIndexA);
	CapsuleShape* b = (CapsuleShape*)data.b->GetCollider()->GetShape(data.shapeIndexB);

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

bool CollideCirclePlane(CollisionData& data)
{
	CircleShape* a = (CircleShape*)data.a->GetCollider()->GetShape(data.shapeIndexA);
	PlaneShape* b = (PlaneShape*)data.b->GetCollider()->GetShape(data.shapeIndexB);

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

bool CollidePolygonPolygon(CollisionData& data)
{
	return false;
}

bool CollidePolygonCapsule(CollisionData& data)
{
	PolygonShape* a = (PolygonShape*)data.a->GetCollider()->GetShape(data.shapeIndexA);
	CapsuleShape* b = (CapsuleShape*)data.b->GetCollider()->GetShape(data.shapeIndexB);

	Vector2 stadPointA = data.b->GetTransform().TransformPoint(b->pointA),
		stadPointB = data.b->GetTransform().TransformPoint(b->pointB);
	//Vector2 stadTangent = glm::normalize(stadPointA - stadPointB);
	//Vector2 stadNormal = { -stadTangent.y, stadTangent.x };

	float minDistanceSqr = b->radius * b->radius + 1;
	Vector2 collisionPoint;
	Vector2 normal;

	for (size_t i = 0; i < a->pointCount; i++)
	{
		Vector2 point = data.a->GetTransform().TransformPoint(a->points[i]);
		Vector2 linePoint = em::ClosestPointOnLine(stadPointA, stadPointB, point);
		float distSqr = em::SquareLength(point - linePoint);

		if (distSqr < minDistanceSqr)
		{
			collisionPoint = linePoint;
			minDistanceSqr = distSqr;
			normal = point - linePoint;
		}
	}

	if (minDistanceSqr < b->radius * b->radius)
	{
		minDistanceSqr = sqrtf(minDistanceSqr);

		data.penetration = b->radius - minDistanceSqr;
		data.collisionNormal = normal / minDistanceSqr;
		data.collisionPoints[0] = collisionPoint + data.collisionNormal * b->radius; // this is wrong!!!!
		return true;
	}


	return false;
}

bool CollidePolygonPlane(CollisionData& data) 
{
	PolygonShape* a = (PolygonShape*)data.a->GetCollider()->GetShape(data.shapeIndexA);
	PlaneShape* b = (PlaneShape*)data.b->GetCollider()->GetShape(data.shapeIndexB);

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

bool CollideCapsuleCapsule(CollisionData& data)
{
	CapsuleShape* a = (CapsuleShape*)data.a->GetCollider()->GetShape(data.shapeIndexA);
	CapsuleShape* b = (CapsuleShape*)data.b->GetCollider()->GetShape(data.shapeIndexB);

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
		float sign = glm::sign(p1);
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

bool CollideCapsulePlane(CollisionData& data)
{
	CapsuleShape* a = (CapsuleShape*)data.a->GetCollider()->GetShape(data.shapeIndexA);
	PlaneShape* b = (PlaneShape*)data.b->GetCollider()->GetShape(data.shapeIndexB);

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

bool CollideInvalid(CollisionData& data)
{
	return false;
}


#pragma region Flipped Functions

bool CollidePolygonCircle(CollisionData& data)
{
	auto temp = data.a;
	data.a = data.b;
	data.b = temp;
	return CollideCirclePolygon(data);
}

bool CollideCapsuleCircle(CollisionData& data)
{
	auto temp = data.a;
	data.a = data.b;
	data.b = temp;
	return CollideCircleCapsule(data);
}

bool CollidePlaneCircle(CollisionData& data)
{
	auto temp = data.a;
	data.a = data.b;
	data.b = temp;
	return CollideCirclePlane(data);
}

bool CollideCapsulePolygon(CollisionData& data)
{
	auto temp = data.a;
	data.a = data.b;
	data.b = temp;
	return CollidePolygonCapsule(data);
}

bool CollidePlanePolygon(CollisionData& data)
{
	auto temp = data.a;
	data.a = data.b;
	data.b = temp;
	return CollidePolygonPlane(data);
}

bool CollidePlaneCapsule(CollisionData& data)
{
	auto temp = data.a;
	data.a = data.b;
	data.b = temp;
	return CollideCapsulePlane(data);
}
#pragma endregion
