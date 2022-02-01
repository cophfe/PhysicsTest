#include "CollisionManager.h"

void CollisionManager::ResolveCollisions(std::vector<PhysicsObject>& pObjects)
{
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

				if (CheckAABBCollision(pObjects[i].GetCollider()->GetAABB(), pObjects[i].GetCollider()->GetAABB()))
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
	manifold.type = (COLLISION_TYPE)((char)manifold.a->GetCollider()->GetType() + (char)manifold.b->GetCollider()->GetType());

	if (manifold.type == COLLISION_TYPE::POLYGONCIRCLE)
	{
		//make sure polygon is first
		if (manifold.a->GetCollider()->GetType() == COLLIDER_TYPE::CIRCLE)
		{
			auto circle = manifold.a;
			manifold.a = manifold.b;
			manifold.b = circle;
		}
	}

	switch (manifold.type) 
	{
	case COLLISION_TYPE::CIRCLECIRCLE:
		break;
	case COLLISION_TYPE::POLYGONCIRCLE:
		break;
	case COLLISION_TYPE::POLYGONPOLYGON:
		break;
	}

	return false;
}

bool CollisionManager::CheckAABBCollision(AABB& a, AABB& b)
{
	return (a.min.x < b.max.x&& a.min.y < b.max.y
		&& a.max.x > b.min.x&& a.max.y > b.min.y
		&& b.min.x < a.max.x&& b.min.y < a.max.y
		&& b.max.x > a.min.x&& b.max.y > a.min.y);
}