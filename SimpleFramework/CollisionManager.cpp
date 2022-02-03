#include "CollisionManager.h"

void CollisionManager::ResolveCollisions(std::vector<PhysicsObject>& pObjects)
{
	if (pObjects.size() < 2)
		return;

	collisions.clear();

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

				if (CheckAABBCollision(pObjects[i].collider->aABB, pObjects[i].collider->aABB))
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
	if (EvaluateCollision(manifold))
	{
		
		//resolve collision
		Vector2 rV = manifold.b->GetVelocity() - manifold.a->GetVelocity();
		float projectedRV = glm::dot(manifold.collisionNormal, rV);
		
		//if velocities are seperating don't collide
		if (projectedRV > 0)
		{
			//calculate impulse magnitude
			//float impulseMagnitude = ?????;
			//divide by inverse masses add together to ratio by mass
			//impulseMagnitude /= (manifold.a->GetInverseMass() + manifold.b->GetInverseMass());
			//turn into vector
			//Vector2 impulse = manifold.collisionNormal * impulseMagnitude;

			//calculate impulse to add
			// (if it is for multiple positions idk, maybe just give up)
			//manifold.a->AddImpulseAtPosition(impulse * -1, manifold->hitPosition);
			//manifold.b->AddImpulseAtPosition(impulse, manifold->hitPosition);
		}

		//teleport shapes out of each other based on mass
		/*manifold.a->SetPosition(manifold.a->GetPosition() + manifold.collisionNormal * (manifold.penetration * manifold.a->GetInverseMass() / (manifold.a->GetInverseMass() + manifold.b->GetInverseMass())));
		manifold.b->SetPosition(manifold.b->GetPosition() - manifold.collisionNormal * (manifold.penetration * manifold.b->GetInverseMass() / (manifold.a->GetInverseMass() + manifold.b->GetInverseMass())));*/
	}
}

bool CollisionManager::EvaluateCollision(CollisionManifold& manifold)
{
	//set up manifold
	GetCollisionType(manifold);

	switch (manifold.type) 
	{
	case COLLISION_TYPE::CIRCLECIRCLE:
		auto a = manifold.a->collider;
		break;
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
	// and the number that is being put to the power of also needs to be greater than number of inputs - 1 otherwise at risk of duplicates
	
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
	
	case 129:	// Circle - Line
		manifold.type = COLLISION_TYPE::CIRCLELINE;
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
	
	case 132:	// Polygon - Line
		manifold.type = COLLISION_TYPE::POLYGONLINE;
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
	
	case 48:	// Capsule - Capsule
		manifold.type = COLLISION_TYPE::CAPSULECAPSULE;
		break;
	
	case 144:	// Capsule - Line
		manifold.type = COLLISION_TYPE::CAPSULELINE;
		break;

	case 66:	// Line - Circle
		t = manifold.a;
		manifold.a = manifold.b;
		manifold.b = t;
		manifold.type = COLLISION_TYPE::CIRCLELINE;
		break;
	
	case 72:	// Line - Polygon
		 t = manifold.a;
		manifold.a = manifold.b;
		manifold.b = t;
		manifold.type = COLLISION_TYPE::POLYGONLINE;
		break;
	
	case 96:	// Line - Capsule
		 t = manifold.a;
		manifold.a = manifold.b;
		manifold.b = t;
		manifold.type = COLLISION_TYPE::CAPSULELINE;
		break;
	
	default:
	case 192:	// Line - Line
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