#include "fzx.h"

namespace fzx
{

#pragma region Polygon

	bool CollisionManager::CollidePolygonPolygon(CollisionData& data)
	{
		PolygonShape* a = (PolygonShape*)data.a->GetCollider(data.colliderIndexA).GetShape();
		PolygonShape* b = (PolygonShape*)data.b->GetCollider(data.colliderIndexB).GetShape();

		/*std::cout << "Do polygons collide: " << Intersection(a, b, data.a->transform, data.b->transform, nullptr)
			<< std::endl;*/

		EPACollisionData epaData;
		if (EPA(a, b, data.a->transform, data.b->transform, &epaData))
		{
			//now find the collision points using the clipping method.
			PolygonEdge reference = FindPolygonCollisionEdge(a, data.a->transform, -epaData.collisionNormal);
			PolygonEdge incident = FindPolygonCollisionEdge(b, data.b->transform, epaData.collisionNormal);

			//reference edge: this edge clips the incident edge to get the contact points

			//the edge most perpendicular to the normal (the one with the dot product closest to zero) is the reference edge, the other is the incident edge
			bool flipEdges = em::Sq(glm::dot(reference.pB - reference.pA, epaData.collisionNormal)) > em::Sq(glm::dot(incident.pB - incident.pA, epaData.collisionNormal));
			if (flipEdges)
			{
				PolygonEdge temp = reference;
				reference = incident;
				incident = temp;
			}

			Vector2 referenceTangent = glm::normalize(reference.pB - reference.pA);

			float referenceStart = glm::dot(reference.pA, referenceTangent);
			ClipInfo c;
			/*c.pointCount = 2;
			c.points[0] = incident.pA;
			c.points[1] = incident.pB;*/
			c = Clip(incident.pA, incident.pB, referenceTangent, referenceStart);
			if (c.pointCount < 2) return false;

			float referenceEnd = glm::dot(reference.pB, referenceTangent);
			c = Clip(c.points[0], c.points[1], -referenceTangent, -referenceEnd);
			if (c.pointCount < 2) return false;

			//delete vertices that are above the edge
			Vector2 referenceNormal = em::GetPerpendicularClockwise(referenceTangent);
			//if (flipEdges)
			//	referenceNormal = -referenceNormal;

			float max = glm::dot(reference.maxProjectionVertex, referenceNormal);
			if (glm::dot(c.points[0], referenceNormal) > max)
			{
				if (c.pointCount > 1)
					c.points[0] = c.points[1];
				c.pointCount--;
			}
			if (glm::dot(c.points[1], referenceNormal) > max)
			{
				c.pointCount--;
			}
			if (c.pointCount < 1) return false;

			data.collisionNormal = epaData.collisionNormal;
			data.penetration = epaData.depth;
			data.collisionPoints[0] = c.points[0];
			data.collisionPoints[1] = c.points[1];
			data.pointCount = c.pointCount;


			return true;
		}

		return false;
	}

	bool CollisionManager::CollidePolygonCapsule(CollisionData& data)
	{
		PolygonShape* a = (PolygonShape*)data.a->GetCollider(data.colliderIndexA).GetShape();
		CapsuleShape* b = (CapsuleShape*)data.b->GetCollider(data.colliderIndexB).GetShape();

		EPACollisionData epaData;
		if (EPA(a, b, data.a->transform, data.b->transform, &epaData))
		{
			data.collisionNormal = epaData.collisionNormal;
			data.penetration = epaData.depth;

			//data.collisionPoints[0] = em::ClosestPointOnLine(data.b->transform.TransformPoint(b->pointA), data.b->transform.TransformPoint(b->pointB),
			//	data.a->transform.TransformPoint(a->centrePoint)) - b->radius * data.collisionNormal; //<-- once again, this works really well for something that is not accurate.

			//Now find collision point by comparing 
			PolygonEdge aEdge = FindPolygonCollisionEdge(a, data.a->transform, -data.collisionNormal);
			Vector2 bPointA = data.b->GetTransform().TransformPoint(b->pointA),
				bPointB = data.b->GetTransform().TransformPoint(b->pointB);

			//Vector2 intersectionPoint;
			/*if (em::CalculateIntersectionPoint(aEdge.pA, aEdge.pB, bPointA, bPointB, intersectionPoint))
			{
				//in this case, when both polygon edge points are on the opposite side of the capsule centre line than the normal direction,
				//it probably returns the wrong value. but idk it's hard to say
			}*/
			//((b-a) x ((0,0)-a)) x (b-a)
			Vector2 bNormal = em::GetPerpendicularCounterClockwise(bPointB - bPointA);//, data.a->transform.position);
			bNormal = GetPerpendicularFacingInDirection(bPointB - bPointA, -data.collisionNormal);
			float stadiumDistance = glm::dot(bNormal, bPointA);

			float pA = glm::dot(aEdge.pA, bNormal);
			float pB = glm::dot(aEdge.pB, bNormal);
			if (pA - stadiumDistance < pB - stadiumDistance)
			{
				data.collisionPoints[0] = em::ClosestPointOnLine(bPointA, bPointB, aEdge.pA) + b->radius * data.collisionNormal;
			}
			else
				data.collisionPoints[0] = em::ClosestPointOnLine(bPointA, bPointB, aEdge.pB) + b->radius * data.collisionNormal;
			return true;
		}


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

			data.pointCount = 0;
			//now find collision points
			//now find the collision points using the clipping method.
			PolygonEdge incident = FindPolygonCollisionEdge(a, data.a->transform, -planeNormal);
			//the reference edge is perpendicular to the plane normal

			//add vertices that aren't above the plane
			if (glm::dot(incident.pA, planeNormal) <= planeDistance)
			{
				data.collisionPoints[0] = incident.pA;
				data.pointCount++;
			}
			if (glm::dot(incident.pB, planeNormal) <= planeDistance)
			{
				data.collisionPoints[data.pointCount] = incident.pB;
				data.pointCount++;
			}
			if (data.pointCount < 1)
				return false;
			else
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
			float sign = glm::sign(p1); //<-- could probably get rid of this somehow with the triple cross product thing
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

	static void FlipData(CollisionData& data)
	{
		auto temp = data.a;
		data.a = data.b;
		data.b = temp;

		auto temp2 = data.colliderIndexA;
		data.colliderIndexA = data.colliderIndexB;
		data.colliderIndexB = temp2;
	}
	bool CollisionManager::CollidePolygonCircle(CollisionData& data)
	{
		FlipData(data);
		return CollideCirclePolygon(data);
	}

	bool CollisionManager::CollideCapsuleCircle(CollisionData& data)
	{
		FlipData(data);
		return CollideCircleCapsule(data);
	}

	bool CollisionManager::CollidePlaneCircle(CollisionData& data)
	{
		FlipData(data);
		return CollideCirclePlane(data);
	}

	bool CollisionManager::CollideCapsulePolygon(CollisionData& data)
	{
		FlipData(data);
		return CollidePolygonCapsule(data);
	}

	bool CollisionManager::CollidePlanePolygon(CollisionData& data)
	{
		FlipData(data);
		return CollidePolygonPlane(data);
	}

	bool CollisionManager::CollidePlaneCapsule(CollisionData& data)
	{
		FlipData(data);
		return CollideCapsulePlane(data);
	}
#pragma endregion
}