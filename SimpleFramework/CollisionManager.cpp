#include "CollisionManager.h"
#include "ExtraMath.hpp"
#include "PhysicsProgram.h"

#ifndef COLLISIONROTATION
#define COLLISIONROTATION
#endif // !COLLISIONROTATION
#ifndef FRICTION
//#define FRICTION
#endif // !FRICTION


CollideFunction CollisionManager::collisionFunctions[4][4] =
{
	{ CollideCircleCircle,	CollideCirclePolygon,	CollideCircleCapsule,	CollideCirclePlane	},
	{ CollidePolygonCircle,	CollidePolygonPolygon,	CollidePolygonCapsule,	CollidePolygonPlane	},
	{ CollideCapsuleCircle,	CollideCapsulePolygon,	CollideCapsuleCapsule,	CollideCapsulePlane },
	{ CollidePlaneCircle,	CollidePlanePolygon,	CollidePlaneCapsule,	CollideInvalid		}
};

void CollisionManager::ResolveCollisions()
{
	if (bodies.size() < 2)
		return;

	collisions.clear();

	for (size_t i = 0; i < bodies.size(); i++)
	{
		bodies[i].GenerateAABB();
	}

	for (int i = 0; i < bodies.size() - 1; i++)
	{
		//skip over if is not active
		if (bodies[i].GetCollider() == nullptr)
		{
			continue;
		}

		for (int j = i + 1; j < bodies.size(); j++)
		{
			//check if the objects are compatible layer wise, if collider exists, and 
			if (bodies[i].GetCollider() != nullptr)
			{
				//broad phase
				//this checks if the AABBs are colliding

				if (CheckAABBCollision(bodies[i].collider->aABB, bodies[j].collider->aABB))
				{
					//in this case we need to check if collision is valid, and if so, resolve it
					//we add it to collisions for this frame
					collisions.emplace_back(CollisionData(&bodies[i], &bodies[j]));
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

PhysicsObject* CollisionManager::PointCast(Vector2 point, bool includeStatic)
{
	for (size_t i = 0; i < bodies.size(); i++)
	{
		if (bodies[i].collider)
		{
			for (size_t j = 0; j < bodies[i].collider->shapeCount; j++)
			{
				if ((includeStatic || bodies[i].iMass != 0) && bodies[i].collider->shapes[j]->PointCast(point, bodies[i].transform))
				{
					std::cout << "Grabbed object with mass " << bodies[i].iMass << std::endl;
					return &bodies[i];
				}
			}
			
		}
	}
	return nullptr;
}

void CollisionManager::Update()
{
	UpdatePhysics();
	ResolveCollisions();
}

void CollisionManager::DrawShapes()
{
	for (auto& body : bodies)
	{
		body.Render(*program);
	}
}

void CollisionManager::UpdatePhysics()
{
	//do physics
	for (auto& body : bodies)
	{
		body.Update(deltaTime);

		if (body.GetInverseMass() != 0)
		{
			body.AddVelocity(gravity * deltaTime);
		}
	}
}

PhysicsObject& CollisionManager::AddPhysicsObject(PhysicsObject&& body)
{
	bodies.emplace_back(body);
	return bodies[bodies.size() - 1];
}

PhysicsObject& CollisionManager::AddPhysicsObject(PhysicsObject body)
{
	bodies.push_back(body);
	return bodies[bodies.size() - 1];
}

void CollisionManager::ClearPhysicsBodies()
{
	bodies.clear();
}

static Vector2 GetVelocityAtPoint(Vector2 centre, Vector2 point, float angularVelocity, Vector2 velocity) 
{
	//radius vector works as both a distance from centre multiplier and an object normal
	Vector2 radiusVector = point - centre;
	Vector2 angularVelocityVector = angularVelocity * Vector2{ -radiusVector.y, radiusVector.x };
	//this ^ is 2D equivelant of Cross(angularVelocity, radiusVector)
	//this is different from the current em::Cross function because is simplified from when cross takes two vector3 values like this: Vector3(X,X,0), Vector3(X,X,0). returns a float because only the z component of the return value is nonzero
	//This 'cross' simplifies from when cross takes two vector3 values like this: vector3(0,0,X), Vector3(X,X,0). it returns a vector2 because in that case, the x and y values of the return value are nonzero
	return angularVelocityVector + velocity;
}


void CollisionManager::ResolveCollision(CollisionData& manifold)
{
	//if collision happened (data is added into manifold about collision)
	if (EvaluateCollision(manifold) && (manifold.a->iMass + manifold.b->iMass != 0))
	{
#ifdef COLLISIONROTATION
		Vector2 radiusA = manifold.collisionPoints[0] - manifold.a->transform.position,
			radiusB = manifold.collisionPoints[0] - manifold.b->transform.position;
		//explanation for this \/ in GetVelocityAtPoint
		Vector2 angularVelocityA = manifold.a->GetAngularVelocity() * Vector2 { -radiusA.y, radiusA.x };
		Vector2 angularVelocityB = manifold.b->GetAngularVelocity() * Vector2 { -radiusB.y, radiusB.x };
		Vector2 rV = (manifold.b->GetVelocity() + angularVelocityB)
			- (manifold.a->GetVelocity() + angularVelocityA);

		float projectedRV = glm::dot(manifold.collisionNormal, rV);
		//bounciness is average of the two
		float e = 0.5f * (manifold.a->bounciness + manifold.b->bounciness);

		float rACrossN = em::Cross(radiusA, manifold.collisionNormal);
		float rBCrossN = em::Cross(radiusB, manifold.collisionNormal);

		float impulseMagnitude = (-(1 + e) * projectedRV)
			/ (manifold.a->iMass + manifold.b->iMass
				+ (rACrossN * rACrossN * manifold.a->iMomentOfInertia) + (rBCrossN * rBCrossN * manifold.b->iMomentOfInertia));

		//turn into vector
		Vector2 impulse = manifold.collisionNormal * impulseMagnitude;

		//calculate impulse to add
		manifold.a->AddImpulseAtPosition(-impulse, manifold.collisionPoints[0]);
		manifold.b->AddImpulseAtPosition(impulse, manifold.collisionPoints[0]);

#ifdef FRICTION
		//FRICTION
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		//a lot of this requires object-specific info, which doesn't make sense because impulse based collision model should have equal and opposite forces

		//function that box2D uses to get friction (so that low object frictions seriously lower overall friction)
		float staticFriction = sqrtf(manifold.a->staticFriction * manifold.b->staticFriction);
		float dynamicFriction = sqrtf(manifold.a->dynamicFriction * manifold.b->dynamicFriction);

		Vector2 tangent;
		if (projectedRV == 0)
		{
			//component of forces that that is parallel to the collision normal
			tangent = glm::normalize(Vector2(0, -1) - glm::dot(Vector2(0, -1), manifold.collisionNormal) * manifold.collisionNormal);
		}
		else
		{
			//component of rV that is parallel to the collision normal
			Vector2 t = rV - projectedRV * manifold.collisionNormal;
			if (t == Vector2(0, 0))
				tangent = Vector2(0, 0);
			else
				tangent = glm::normalize(t); // t can be 0,0
	}
		float frictionMagnitude;

		frictionMagnitude = glm::dot(rV, tangent) * dynamicFriction; //<-- what actually works (mostly
		//frictionMagnitude = -impulseMagnitude * dynamicFriction; //<-- what it says to do 

		//if friction magnitude would be so big as to flip the direction of velocity, just make it zero the velocity
		//float velocityInTangentDirection = glm::dot(manifold.a->velocity, tangent);
		//if (glm::abs(frictionMagnitude * manifold.a->iMass) > glm::abs(velocityInTangentDirection))
			//manifold.a->AddVelocityAtPosition(-velocityInTangentDirection * tangent, manifold.collisionPoints[0]);
		//else
		manifold.a->AddImpulseAtPosition(frictionMagnitude * tangent, manifold.collisionPoints[0]);
		manifold.b->AddImpulseAtPosition(-frictionMagnitude * tangent, manifold.collisionPoints[0]);
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#endif
#else
		//resolve collision
		Vector2 rV = manifold.b->GetVelocity() - manifold.a->GetVelocity();
		
		float projectedRV = glm::dot(manifold.collisionNormal, rV);
		//bounciness is average of the two
		float e = 0.5f * (manifold.a->bounciness + manifold.b->bounciness);

		//calculate impulse magnitude
		float impulseMagnitude = -(1 + e) * projectedRV;
		impulseMagnitude /= (manifold.a->GetInverseMass() + manifold.b->GetInverseMass());

		//turn into vector
		Vector2 impulse = manifold.collisionNormal * impulseMagnitude;

		manifold.a->AddImpulse(-impulse);
		manifold.b->AddImpulse(impulse);
#endif

		
		
		//teleport shapes out of each other based on mass
		manifold.a->SetPosition(manifold.a->GetPosition() + manifold.collisionNormal * (manifold.penetration * manifold.a->GetInverseMass() / (manifold.a->GetInverseMass() + manifold.b->GetInverseMass())));
		manifold.b->SetPosition(manifold.b->GetPosition() - manifold.collisionNormal * (manifold.penetration * manifold.b->GetInverseMass() / (manifold.a->GetInverseMass() + manifold.b->GetInverseMass())));

		//[debug] add collision point for rendering
		program->collisionPoints.push_back(manifold.collisionPoints[0]);
	}
}

bool CollisionManager::EvaluateCollision(CollisionData& data)
{
	int x = (int)data.a->GetCollider()->GetShape(data.shapeIndexA)->GetType();
	int y = (int)data.b->GetCollider()->GetShape(data.shapeIndexB)->GetType();
	return (collisionFunctions[x][y])(data);
}

bool CollisionManager::CheckAABBCollision(AABB& a, AABB& b)
{
	return (a.min.x < b.max.x&& a.min.y < b.max.y
		&& a.max.x > b.min.x&& a.max.y > b.min.y
		&& b.min.x < a.max.x&& b.min.y < a.max.y
		&& b.max.x > a.min.x&& b.max.y > a.min.y);
}