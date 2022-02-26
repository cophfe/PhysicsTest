#include "fzx.h"

#ifndef COLLISIONROTATION
#define COLLISIONROTATION
#endif // !COLLISIONROTATION
#ifndef FRICTION
#define FRICTION
#endif // !FRICTION

namespace fzx
{
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
			bodies[i]->GenerateAABB();
		}

		for (int i = 0; i < bodies.size() - 1; i++)
		{
			//int cc1 = bodies[i]->GetColliderCount();
			//skip over if is not active
			if (bodies[i]->GetColliderCount() == 0)
			{
				continue;
			}

			for (int j = i + 1; j < bodies.size(); j++)
			{
				//int cc2 = bodies[i]->GetColliderCount();
				if (bodies[i]->GetColliderCount() == 0)
					continue;

				//broad phase
				//this checks if the AABBs are colliding

				if (CheckAABBCollision(bodies[i]->GetAABB(), bodies[j]->GetAABB()))
				{
					//in this case we need to check if collision is valid, and if so, resolve it
					//we add it to collisions for this frame
					//collisions.emplace_back(CollisionData(bodies[i], bodies[j]));

					for (char u = 0; u < bodies[i]->GetColliderCount(); u++)
					{
						for (char v = 0; v < bodies[j]->GetColliderCount(); v++)
						{
							Collider& c1 = bodies[i]->GetCollider(u);
							Collider& c2 = bodies[j]->GetCollider(v);

							//if collision layers correctly match up and aabbs are intersecting
							if ((c1.collisionLayer & c2.collisionMask) && (c2.collisionLayer & c1.collisionMask) && CheckAABBCollision(c1.aABB, c2.aABB))
							{
								collisions.emplace_back(CollisionData(bodies[i], bodies[j], u, v));
							}
						}
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

	PhysicsObject* CollisionManager::PointCast(Vector2 point, bool includeStatic, bool includeTriggers)
	{
		for (int i = 0; i < bodies.size(); i++)
		{
			for (int j = 0; j < bodies[i]->GetColliderCount(); j++)
			{
				if ((includeTriggers || !bodies[i]->GetCollider(j).isTrigger) && (includeStatic || bodies[i]->isDynamic) && bodies[i]->GetCollider(j).GetShape()->PointCast(point, bodies[i]->transform))
				{
					return bodies[i];
				}
			}
		}
		return nullptr;
	}

	void CollisionManager::Update()
	{
		UpdatePhysics();
		for (size_t i = 0; i < COLLISION_ITERATIONS; i++)
		{
			ResolveCollisions();
		}
	}

	void CollisionManager::UpdatePhysics()
	{
		//do physics
		for (auto* body : bodies)
		{
			body->Update(deltaTime);

			if (body->GetInverseMass() != 0)
			{
				body->AddVelocity(gravity * deltaTime);
			}
		}
	}

	PhysicsObject* CollisionManager::CreatePhysicsObject(PhysicsData& data)
	{
		PhysicsObject* body = new PhysicsObject(data);
		bodies.push_back(body);
		return bodies[bodies.size() - 1];
	}

	void CollisionManager::DeletePhysicsBody(PhysicsObject* body)
	{
		if (!body) return;

		bodies.erase(std::remove(bodies.begin(), bodies.end(), body));
		delete body;
	}

	void CollisionManager::ClearPhysicsBodies()
	{
		for (size_t i = 0; i < bodies.size(); i++)
		{
			delete bodies[i];
		}
		bodies.clear();
	}

	static Vector2 GetVelocityAtPoint(Vector2 centre, Vector2 point, float angularVelocity, Vector2 velocity)
	{
		//radius vector works as both a distance from centre multiplier and an object normal
		Vector2 radiusVector = point - centre;
		Vector2 angularVelocityVector = angularVelocity * Vector2{ -radiusVector.y, radiusVector.x };
		//this^ is the perpendicular vector multiplied by  angular velocity, but it is also the 2D equivelant of Cross(angularVelocity, radiusVector)
		//this is different from the current em::Cross function because is simplified from when cross takes two vector3 values like this: Vector3(X,X,0), Vector3(X,X,0). returns a float because only the z component of the return value is nonzero
		//This 'cross' simplifies from when cross takes two vector3 values like this: vector3(0,0,X), Vector3(X,X,0). it returns a vector2 because in that case, the x and y values of the return value are nonzero
		return angularVelocityVector + velocity;
	}


	void CollisionManager::ResolveCollision(CollisionData& data)
	{
		//if collision happened (data is added into manifold about collision)
		if ((data.a->iMass + data.b->iMass != 0) &&
			EvaluateCollision(data))
		{
			if ((cCallback && !cCallback(data, cCallbackPtr)) || data.a->GetCollider(data.colliderIndexA).GetIsTrigger() || data.b->GetCollider(data.colliderIndexB).GetIsTrigger())
			{
				//if the callback returns false, or one of the colliders is a trigger, the collision isn't evaluated
				return;
			}

			Vector2 collisionPoint;
			if (data.pointCount == 2)
				collisionPoint = 0.5f * (data.collisionPoints[0] + data.collisionPoints[1]);
			else
				collisionPoint = data.collisionPoints[0];

#ifdef COLLISIONROTATION
			Vector2 radiusA = collisionPoint - data.a->transform.position,
				radiusB = collisionPoint - data.b->transform.position;
			//explanation for this \/ in GetVelocityAtPoint
			Vector2 angularVelocityA = data.a->GetAngularVelocity() * Vector2 { -radiusA.y, radiusA.x };
			Vector2 angularVelocityB = data.b->GetAngularVelocity() * Vector2 { -radiusB.y, radiusB.x };
			Vector2 rV = (data.b->GetVelocity() + angularVelocityB)
				- (data.a->GetVelocity() + angularVelocityA);

			float projectedRV = glm::dot(data.collisionNormal, rV);

			if (projectedRV > 0) // this check stops objects from 'sticking' together
			{
				//bounciness is average of the two
				float e = 0.5f * (data.a->bounciness + data.b->bounciness);

				float rACrossN = em::Cross(radiusA, data.collisionNormal);
				float rBCrossN = em::Cross(radiusB, data.collisionNormal);

				float massDistribution = 1.0f / (data.a->iMass + data.b->iMass
					+ (rACrossN * rACrossN * data.a->iInertia) + (rBCrossN * rBCrossN * data.b->iInertia));
				float impulseMagnitude = (-(1 + e) * projectedRV) * massDistribution;

				//turn into vector
				Vector2 impulse = data.collisionNormal * impulseMagnitude;

				//calculate impulse to add
				data.a->AddImpulseAtPosition(-impulse, collisionPoint);
				data.b->AddImpulseAtPosition(impulse, collisionPoint);

#ifdef FRICTION
				//FRICTION
				//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

				//this is how box2D calculates friction coefficients (so that low coefficients seriously lower overall friction)
				float staticFriction = sqrtf(data.a->staticFriction * data.b->staticFriction);
				float dynamicFriction = sqrtf(data.a->dynamicFriction * data.b->dynamicFriction);


				Vector2 tangent;
				//calculate the tangent here. if there is no relative velocity than there is zero friction too (or at least zero friction that can be modelled using the coloumb model)
				Vector2 t = rV - projectedRV * data.collisionNormal;
				if (t == Vector2(0, 0))
					tangent = Vector2(0, 0);
				else
					tangent = glm::normalize(t);
				float tangentRV = glm::dot(tangent, rV);

				float rACrossT = em::Cross(radiusA, tangent);
				float rBCrossT = em::Cross(radiusB, tangent);
				massDistribution = 1.0f / (data.a->iMass + data.b->iMass
					+ (rACrossT * rACrossT * data.a->iInertia) + (rBCrossT * rBCrossT * data.b->iInertia));

				//the magnitude of friction in a static friction situation
				float frictionMagnitude = dynamicFriction * -tangentRV * massDistribution;
				//if overcomes static friction, set to dynamic friction
				// impulseMag is always negative, so the comparer is flipped into less than from more than 
				if (frictionMagnitude <= staticFriction * impulseMagnitude)
					frictionMagnitude = dynamicFriction * impulseMagnitude;

				data.a->AddImpulseAtPosition(-frictionMagnitude * tangent, collisionPoint);
				data.b->AddImpulseAtPosition(frictionMagnitude * tangent, collisionPoint);
				//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#endif
#else
			//resolve collision
			Vector2 rV = manifold.b->GetVelocity() - manifold.a->GetVelocity();

			float projectedRV = glm::dot(manifold.collisionNormal, rV);

			if (projectedRV > 0)
			{
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
			}

			//teleport shapes out of each other based on mass
			Vector2 offsetA = data.collisionNormal * (data.penetration * data.a->GetInverseMass() / (data.a->GetInverseMass() + data.b->GetInverseMass()));
			data.a->SetPosition(data.a->GetPosition() + offsetA);
			Vector2 offsetB = -data.collisionNormal * (data.penetration * data.b->GetInverseMass() / (data.a->GetInverseMass() + data.b->GetInverseMass()));
			data.b->SetPosition(data.b->GetPosition() + offsetB);

			//[debug] add collision point for rendering
			//program->collisionPoints.push_back(collisionPoint);
		}
	}

	bool CollisionManager::EvaluateCollision(CollisionData & data)
	{
		int x = (int)data.a->GetCollider(data.colliderIndexA).GetShape()->GetType();
		int y = (int)data.b->GetCollider(data.colliderIndexB).GetShape()->GetType();
		return (collisionFunctions[x][y])(data);
	}

	void CollisionManager::Destroy()
	{
		for (size_t i = 0; i < bodies.size(); i++)
		{
			delete bodies[i];
			bodies[i] = nullptr;
		}
	}

	CollisionManager::~CollisionManager()
	{
	}

	bool CollisionManager::CheckAABBCollision(AABB & a, AABB & b)
	{
		return (a.min.x < b.max.x&& a.min.y < b.max.y
			&& a.max.x > b.min.x&& a.max.y > b.min.y
			&& b.min.x < a.max.x&& b.min.y < a.max.y
			&& b.max.x > a.min.x&& b.max.y > a.min.y);
	}
}