#include "CollisionManager.h"
#include "ExtraMath.hpp"

void CollisionManager::ResolveCollisions(std::vector<PhysicsObject>& pObjects)
{
	if (pObjects.size() < 2)
		return;

	collisions.clear();

	for (size_t i = 0; i < pObjects.size(); i++)
	{
		pObjects[i].GenerateAABB();
	}

	for (int i = 0; i < pObjects.size() - 1; i++)
	{
		//skip over if is not active
		if (pObjects[i].GetCollider() == nullptr)
		{
			continue;
		}

		for (int j = i + 1; j < pObjects.size(); j++)
		{
			//check if the objects are compatible layer wise, if collider exists, and 
			if (pObjects[i].GetCollider() != nullptr)
			{
				//broad phase
				//this checks if the AABBs are colliding

				if (CheckAABBCollision(pObjects[i].collider->aABB, pObjects[j].collider->aABB))
				{
					//in this case we need to check if collision is valid, and if so, resolve it
					//we add it to collisions for this frame
					collisions.emplace_back(CollisionManifold(&pObjects[i], &pObjects[j]));
				}
			}
		}
	}

	//now that all the potential collisions have been found, resolve collisions
	for (int i = 0; i < collisions.size(); i++)
	{
		ResolveCollision(collisions[i]);
	}
}

PhysicsObject* CollisionManager::PointCast(Vector2 point, std::vector<PhysicsObject>& pObjects, bool includeStatic)
{
	for (size_t i = 0; i < pObjects.size(); i++)
	{
		if (pObjects[i].collider)
		{
			if ((includeStatic || pObjects[i].iMass != 0) && pObjects[i].collider->shapes[0]->PointCast(point, pObjects[i].transform))
			{
				std::cout << "Grabbed object with mass " << pObjects[i].iMass << std::endl;
				return &pObjects[i];
			}
		}
	}
	return nullptr;
}

void CollisionManager::ResolveCollision(CollisionManifold& manifold)
{
	//if collision happened (data is added into manifold about collision)
	if (EvaluateCollision(manifold) && (manifold.a->iMass + manifold.b->iMass != 0))
	{
		//resolve collision
		Vector2 rV = manifold.b->GetVelocity() - manifold.a->GetVelocity();
		float projectedRV = glm::dot(manifold.collisionNormal, rV);
		
		//bounciness is average of the two
		float e = 0.5f * (manifold.a->bounciness + manifold.b->bounciness);
		//calculate impulse magnitude
		float impulseMagnitude = -(1 + e) * projectedRV;
		//divide by inverse masses add together to ratio by mass
		impulseMagnitude /= (manifold.a->GetInverseMass() + manifold.b->GetInverseMass());
		//turn into vector
		Vector2 impulse = manifold.collisionNormal * impulseMagnitude;

		//calculate impulse to add
		//manifold.a->AddImpulseAtPosition(impulse * -1, manifold->hitPosition);
		//manifold.b->AddImpulseAtPosition(impulse, manifold->hitPosition);
		manifold.a->AddImpulse(-impulse);
		manifold.b->AddImpulse(impulse);

		//teleport shapes out of each other based on mass
		manifold.a->SetPosition(manifold.a->GetPosition() + manifold.collisionNormal * (manifold.penetration * manifold.a->GetInverseMass() / (manifold.a->GetInverseMass() + manifold.b->GetInverseMass())));
		manifold.b->SetPosition(manifold.b->GetPosition() - manifold.collisionNormal * (manifold.penetration * manifold.b->GetInverseMass() / (manifold.a->GetInverseMass() + manifold.b->GetInverseMass())));
	}
}

bool CollisionManager::EvaluateCollision(CollisionManifold& manifold)
{
	//set up manifold
	GetCollisionType(manifold);

	switch (manifold.type) 
	{
	case COLLISION_TYPE::CIRCLECIRCLE:
	{
		CircleShape* a = (CircleShape*)*manifold.a->collider->shapes;
		CircleShape* b = (CircleShape*)*manifold.b->collider->shapes;

		Vector2 pA = manifold.a->transform.TransformPoint(a->centrePoint), pB = manifold.b->transform.TransformPoint(b->centrePoint);
		Vector2 delta = pA - pB;
		float deltaMagSq = delta.x * delta.x + delta.y * delta.y;
		float radiusSum = (a->radius + b->radius);

		if (deltaMagSq < radiusSum * radiusSum)
		{
			deltaMagSq = sqrtf(deltaMagSq);
			manifold.penetration = radiusSum - deltaMagSq;
			manifold.collisionNormal = delta / deltaMagSq;
			return true;
		}

		return false;
	}
	case COLLISION_TYPE::CIRCLECAPSULE:
	{

		CircleShape* a = (CircleShape*)*manifold.a->collider->shapes;
		CapsuleShape* b = (CapsuleShape*)*manifold.b->collider->shapes;
		
		//transform to global
		Vector2 pA = manifold.b->transform.TransformPoint(b->pointA), pB = manifold.b->transform.TransformPoint(b->pointB);
		Vector2 circleCentre = manifold.a->transform.TransformPoint(a->centrePoint);
		
		//circle radius + capsule radius
		float radius = a->radius + b->radius;

		//this basically simplifies the problem to a circle-circle collision
		Vector2 pointOnCapsule = em::ClosestPointOnLine(pA, pB, circleCentre);
		
		Vector2 delta = circleCentre - pointOnCapsule;
		if (delta.x * delta.x + delta.y * delta.y < radius * radius)
		{
			//is colliding
			manifold.collisionNormal = glm::normalize(delta); //collision normal is always from b to a
			manifold.penetration = radius - glm::dot(delta, manifold.collisionNormal); //p = a->radius + b->radius - distancebetweencentreandclosestpointonLine
			manifold.collisionPoints[0] = pointOnCapsule + manifold.collisionNormal * b->radius;
			return true;
		}

		return false;
	}
	case COLLISION_TYPE::CIRCLEPOLYGON:
	{


		return false;
	}
	case COLLISION_TYPE::CIRCLEPLANE:
	{
		CircleShape* a = (CircleShape*)*manifold.a->collider->shapes;
		PlaneShape* b = (PlaneShape*)*manifold.b->collider->shapes;

		//transform to global
		Vector2 centre = manifold.a->transform.TransformPoint(a->centrePoint);
		Vector2 planeDirection = manifold.b->transform.TransformDirection(b->normal);
		float planeDistance = glm::dot(manifold.b->transform.TransformPoint(b->distance * b->normal), planeDirection);

		//calculate penetration
		float centreDot = glm::dot(centre, planeDirection);
		float penetration = centreDot - a->radius - planeDistance;
		
		//if p < 0, is colliding
		if (penetration < 0)
		{
			manifold.collisionNormal = planeDirection;
			manifold.penetration = -penetration;
			manifold.collisionPoints[0] = centre - planeDirection * (a->radius + penetration);
			return true;
		}

		return false;
	}
	case COLLISION_TYPE::CAPSULECAPSULE:
	{
		CapsuleShape* a = (CapsuleShape*)*manifold.a->collider->shapes;
		CapsuleShape* b = (CapsuleShape*)*manifold.b->collider->shapes;

		Vector2 aPointA = manifold.a->transform.TransformPoint(a->pointA),
			aPointB = manifold.a->transform.TransformPoint(a->pointB)
			,bPointA = manifold.b->transform.TransformPoint(b->pointA),
			bPointB = manifold.b->transform.TransformPoint(b->pointB);;

		//find the smallest distance between the two lines, check if less than a->radius + b->radius
		//in 2d this is 4 point-line distance checks, an intersection test, and 4 weird point tests
		

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

			if (p1* p1 > p2*p2)
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
			manifold.collisionNormal = sign  * -normal;
			manifold.penetration = sign * p1 + a->radius + b->radius;
			manifold.collisionPoints[0] = intersection + manifold.collisionNormal * manifold.penetration;
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
			}

			newCollision.collisionDelta = em::ClosestPointOnLine(aPointA, aPointB, bPointB) - bPointB;
			newCollision.collisionDistanceSq = em::SquareLength(newCollision.collisionDelta);
			if (collision.collisionDistanceSq > newCollision.collisionDistanceSq)
			{
				collision = newCollision;
				collisionPoint = bPointB;
			}

			float radius = a->radius + b->radius;

			//if the smallest collision distance is less than the sum of the radii, it is colliding
			if (collision.collisionDistanceSq < radius * radius)
			{
				float length = sqrtf(collision.collisionDistanceSq);
				manifold.penetration = radius - length;
				manifold.collisionNormal = collision.collisionDelta/length;
				manifold.collisionPoints[0] = collisionPoint;
				return true;
			}
		}

		return false;
	}
	case COLLISION_TYPE::CAPSULEPLANE:
	{
		CapsuleShape* a = (CapsuleShape*)*manifold.a->collider->shapes;
		PlaneShape* b = (PlaneShape*)*manifold.b->collider->shapes;
		
		Vector2 pointA = manifold.a->transform.TransformPoint(a->pointA), pointB = manifold.a->transform.TransformPoint(a->pointB);
		Vector2 planeDirection = manifold.b->transform.TransformDirection(b->normal);
		float planeDistance = glm::dot(manifold.b->transform.TransformPoint(b->distance * b->normal), planeDirection);

		float start = glm::dot(pointA, planeDirection);
		float end = glm::dot(pointB, planeDirection);
		if (start > end)
		{
			float temp = start;
			start = end;
			end = temp ;
		}

		start -= a->radius;
		end += a->radius;

		//if colliding
		if (planeDistance >= start)
		{
			manifold.collisionNormal = planeDirection;

			manifold.penetration = planeDistance - start;
			return true;
		}

		return false;
	}

	}

	return false;
}

void CollisionManager::GetCollisionType(CollisionManifold& manifold)
{
	//use a pairing function
	int a = (int)manifold.a->collider->shapes[0]->GetType();
	int b = (int)manifold.b->collider->shapes[0]->GetType();
	// a + 2b works like a cheap pairing function (only works for the inputs that SHAPE_TYPE are)
	// it only works with inputs that are powers of each other (e.g 4^0, 4^1, 4^2), 
	// and the number that is being put to the power of also needs to be greater than number of inputs - 1 otherwise there will be duplicates
	
	// a + 2b
	int pair = a + 2*b;
	
	PhysicsObject* t;
	//need to switch around the pairs so that the collision type gives the correct enum
	switch (pair) {
	case 3:		// Circle - Circle
		manifold.type = COLLISION_TYPE::CIRCLECIRCLE;
		break;

	case 9:		// Circle - Polygon
		manifold.type = COLLISION_TYPE::CIRCLEPOLYGON;
		break;
	
	case 33:	// Circle - Capsule
		manifold.type = COLLISION_TYPE::CIRCLECAPSULE;
		break;

	case 129:	// Circle - Plane
		manifold.type = COLLISION_TYPE::CIRCLEPLANE;
		break;

	case 6:		// Polygon - Circle
		t = manifold.a;
		manifold.a = manifold.b;
		manifold.b = t;
		manifold.type = COLLISION_TYPE::CIRCLEPOLYGON;
		break;
	
	case 12:	// Polygon - Polygon
		manifold.type = COLLISION_TYPE::POLYGONPOLYGON;
		break;
	
	case 36:	// Polygon - Capsule
		manifold.type = COLLISION_TYPE::POLYGONCAPSULE;
		break;

	case 132: // Polygon - Plane
		manifold.type = COLLISION_TYPE::POLYGONPLANE;
		break;

	case 18:	// Capsule - Circle
		t = manifold.a;
		manifold.a = manifold.b;
		manifold.b = t;
		manifold.type = COLLISION_TYPE::CIRCLECAPSULE;
		break;
	
	case 24:	// Capsule - Polygon
		 t = manifold.a;
		manifold.a = manifold.b;
		manifold.b = t;
		manifold.type = COLLISION_TYPE::POLYGONCAPSULE;
		break;
	
	case 48: //Capsule - Capsule
		manifold.type = COLLISION_TYPE::CAPSULECAPSULE;

		break;

	case 144: //Capsule - Plane
		manifold.type = COLLISION_TYPE::CAPSULEPLANE;

		break;

	case 66: // Plane - Circle
		t = manifold.a;
		manifold.a = manifold.b;
		manifold.b = t;
		manifold.type = COLLISION_TYPE::CIRCLEPLANE;
		break;

	case 72:	//Plane - Polygon
		t = manifold.a;
		manifold.a = manifold.b;
		manifold.b = t;
		manifold.type = COLLISION_TYPE::POLYGONPLANE;
		break;

	case 96:	//Plane - Capsule
		t = manifold.a;
		manifold.a = manifold.b;
		manifold.b = t;
		manifold.type = COLLISION_TYPE::CAPSULEPLANE;
		break;
	default:
	case 192:	// Plane - Plane
		manifold.type = COLLISION_TYPE::INVALID;
		break;
	
	}
}

bool CollisionManager::CheckAABBCollision(AABB& a, AABB& b)
{
	return (a.min.x < b.max.x&& a.min.y < b.max.y
		&& a.max.x > b.min.x&& a.max.y > b.min.y
		&& b.min.x < a.max.x&& b.min.y < a.max.y
		&& b.max.x > a.min.x&& b.max.y > a.min.y);
}