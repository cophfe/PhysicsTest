#include "CollisionManager.h"

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

		Vector2 delta = manifold.a->transform.TransformPoint(a->centrePoint) - manifold.b->transform.TransformPoint(b->centrePoint);
		float deltaMagSq = delta.x * delta.x + delta.y * delta.y;
		float radiusSum = (a->radius + b->radius);

		if (deltaMagSq < radiusSum * radiusSum)
		{
			deltaMagSq = sqrtf(deltaMagSq);
			manifold.penetration = radiusSum - deltaMagSq;
			manifold.collisionNormal = delta / deltaMagSq;
			return true;
		}
		else return false;
		break;
	}
	case COLLISION_TYPE::CIRCLECAPSULE:
	{

		CircleShape* a = (CircleShape*)*manifold.a->collider->shapes;
		CapsuleShape* b = (CapsuleShape*)*manifold.b->collider->shapes;
		
		Vector2 pA = manifold.b->transform.TransformPoint(b->pointA), pB = manifold.b->transform.TransformPoint(b->pointB);
		Vector2 circleCentre = manifold.a->transform.TransformPoint(a->centrePoint);
		
		//circle radius + line radius
		float radius = a->radius + b->radius;

		Vector2 delta = pB - pA;
		float lineLength = glm::length(delta);
		Vector2 lineNormal = Vector2(-delta.y, delta.x)/lineLength;

		float distanceToLinePlane = glm::dot(circleCentre, lineNormal) - glm::dot(lineNormal, pA);
		
		if (glm::abs(distanceToLinePlane) < radius)
		{
			Vector2 lineTangent = Vector2(lineNormal.y, -lineNormal.x);
			//point on line relative to the first
			float pointOnLine = glm::dot(lineTangent, circleCentre);
			float lineStart = glm::dot(lineTangent, pA), lineEnd = glm::dot(lineTangent, pB);

			if (pointOnLine > lineStart - radius && pointOnLine < lineEnd + radius)
			{
				//point a
				if (pointOnLine < lineStart)
				{
					Vector2 deltaPoint = circleCentre - pA;
					//now is guaranteed to be colliding
					if (glm::dot(deltaPoint, deltaPoint) < radius * radius)
					{
						//colliding with end point A, which is practically a circle
						//collisionPoint = pA + normalize(deltaPoint) * b->buffer
						manifold.collisionNormal = deltaPoint;
						manifold.penetration = glm::length(manifold.collisionNormal);
						manifold.collisionNormal /= manifold.penetration;
						//p = radius - distance to center + line buffer
						manifold.penetration = a->radius - manifold.penetration + b->radius;
						return true;
					}
					else return false;
				}
				else if (pointOnLine > lineEnd)
				{
					Vector2 deltaPoint = circleCentre - pB;
					//now is guaranteed to be colliding
					if (glm::dot(deltaPoint, deltaPoint) < radius * radius)
					{
						//colliding with end point B, which is practically a circle
						//collisionPoint = pA + normalize(deltaPoint) * b->buffer
						manifold.collisionNormal = deltaPoint;
						manifold.penetration = glm::length(manifold.collisionNormal);
						manifold.collisionNormal /= manifold.penetration;
						//p = radius - distance to center + line buffer
						manifold.penetration = a->radius - manifold.penetration + b->radius;
						return true;
					}
					else return false;
				}
				else {
					//colliding with line
					
					manifold.collisionNormal = glm::normalize(lineNormal * glm::sign(distanceToLinePlane));
					manifold.penetration = a->radius - glm::abs(distanceToLinePlane) + b->radius;
					//collisionPoint = circleCentre + manifold.collisionNormal * manifold.penetration; << accurate for line, this is now a capsule (so it is wrong)
					return true;
				}
			}
			else return false;
		}
		else return false;
		break;
	}
	case COLLISION_TYPE::CIRCLEPOLYGON:
	{


		return false;
		break;
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