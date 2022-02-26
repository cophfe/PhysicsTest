#include "fzx.h"

namespace fzx
{
	const float sleepVelocityMag = 0.0001f; //(these are also squared)
	const float sleepAngularVelocityMag = 0.0001f;
	const float sleepTime = 0.2f;

	PhysicsObject::PhysicsObject(PhysicsData& data) : transform(Transform(data.position, data.rotation)), bounciness(data.bounciness), drag(data.drag), angularDrag(data.angularDrag)
		, staticFriction(data.staticFriction), dynamicFriction(data.dynamicFriction), isDynamic(data.isDynamic), isRotatable(data.isRotatable)
	{
		iMass = 0;
		iInertia = 0;

		colliders = nullptr;
		colliderCount = 0;

		pointer = nullptr;
	}

	void PhysicsObject::Update(float deltaTime)
	{
		transform.position += velocity * deltaTime;
		transform.rotation += angularVelocity * deltaTime;

		//update transform
		transform.UpdateData();

		velocity += force * iMass * deltaTime;
		angularVelocity += torque * iInertia * deltaTime;

		//based on box2d's drag method
		velocity /= (1.0f + drag * deltaTime);
		angularVelocity /= (1.0f + angularDrag * deltaTime);

		//clear force stuff
		force = Vector2(0, 0);
		torque = 0;
	}

	void PhysicsObject::GenerateAABB() {
		switch (colliderCount) {
		case 0:
			return;
		case 1:
			colliderAABB = colliders[0].CalculateAABB(transform);
			return;
		default:
		{
			AABB aabbs[2];
			aabbs[0] = colliders[0].CalculateAABB(transform);

			for (size_t i = 1; i < colliderCount; i++)
			{
				aabbs[1] = colliders[i].CalculateAABB(transform);

				aabbs[0].max.x = glm::max(aabbs[0].max.x, aabbs[1].max.x);
				aabbs[0].max.y = glm::max(aabbs[0].max.y, aabbs[1].max.y);
				aabbs[0].min.x = glm::min(aabbs[0].min.x, aabbs[1].min.x);
				aabbs[0].min.y = glm::min(aabbs[0].min.y, aabbs[1].min.y);
			}

			colliderAABB = aabbs[0];
		}
		}
	}

	void PhysicsObject::AddCollider(Shape* shape, float density, bool recalculateMass, bool isTrigger)
	{
		assert(colliderCount != UCHAR_MAX);
		
		if (colliders == nullptr)
		{
			colliders = new Collider[1]{ Collider(shape, density, isTrigger) };
			colliderCount = 1;

		}
		else
		{
			//move colliders
			Collider* newColliders = new Collider[colliderCount + 1];
			memcpy(newColliders, colliders, colliderCount * sizeof(Collider));
			newColliders[colliderCount] = Collider(shape, density, isTrigger);

			//clear colliders without calling Collider delete functions (since that will cause the shapes to be deleted)
			memset(colliders, 0, sizeof(Collider) * colliderCount);
			delete[] colliders;

			colliders = nullptr;
			colliders = newColliders;

			if (!colliders[colliderCount].CanBeDynamic())
				isDynamic = false;

			colliderCount++;
		}

		if (recalculateMass)
		{
			CalculateMass();
		}

		CentreShapesAboutZero();
	}

	void PhysicsObject::SetPointer(PhysicsObject** ptr)
	{
		if (ptr)
		{
			*ptr = this;
		}
		pointer = ptr;
	}

	void PhysicsObject::AddForceAtPosition(Vector2 force, Vector2 point)
	{
		this->force += force;
		//transform.position should actually be the center point of the collider
		this->torque += em::Cross(point - transform.position, force);
	}

	void PhysicsObject::AddImpulseAtPosition(Vector2 impulse, Vector2 point)
	{
		this->velocity += impulse * iMass;

		//transform.position should be the centre of mass
		this->angularVelocity += em::Cross(point - transform.position, impulse) * iInertia;
	}

	void PhysicsObject::AddVelocityAtPosition(Vector2 velocity, Vector2 point)
	{
		this->velocity += velocity;

		//transform.position should be the centre of mass
		this->angularVelocity += em::Cross(point - transform.position, velocity);
	}

	PhysicsObject::~PhysicsObject()
	{
		if (colliders)
			delete[] colliders;
		colliders = nullptr;
		if (pointer)
			*pointer = nullptr;
	}

	void PhysicsObject::CentreShapesAboutZero(bool translateBody)
	{
		//this is useful for rotation reasons only, so if not rotatable or dynamic, return early
		if (!isDynamic || !isRotatable)
			return;

		Vector2 centrePoint = Vector2(0,0);

		for (unsigned char i = 0; i < colliderCount; i++)
		{
			float mass = colliders[i].iMass == 0 ? 0 : 1 / colliders[i].iMass;
			centrePoint += colliders[i].GetShape()->GetCentrePoint() * mass;
		}

		//this should get the centrepoint (probably assuming uniform density, potential bug since uniform density isn't confirmed here)
		centrePoint *= iMass;
		std::cout << "Centrepoint: (" << centrePoint.x << ", " << centrePoint.y << ")\n";

		//if centrePoint is not zero then all shapes need to be translated until it is zero
		for (unsigned char i = 0; i < colliderCount; i++)
		{
			Shape* s = colliders[i].GetShape();
			switch (s->GetType())
			{
			case SHAPE_TYPE::CIRCLE:
				((CircleShape*)s)->centrePoint -= centrePoint;
				break;
			case SHAPE_TYPE::CAPSULE:
				((CapsuleShape*)s)->pointA -= centrePoint;
				((CapsuleShape*)s)->pointB -= centrePoint;
				break;
			case SHAPE_TYPE::POLYGON:
			{
				PolygonShape* pShape = (PolygonShape*)s;
				for (size_t i = 0; i < pShape->pointCount; i++)
				{
					pShape->points[i] -= centrePoint;
				}
			}
				break;
			default:
				return;
			}

			//now translate inertia by the length of the translation of the centrepoints
			if (iMass != 0)
				colliders[i].iInertia -= em::SquareLength(centrePoint) / colliders[i].iMass;
		}

		if (translateBody)
		{
			transform.position += centrePoint;
		}
	}

	void PhysicsObject::CalculateMass()
	{
		float mass = 0;
		float inertia = 0;

		if (!isDynamic || !CanBeDynamic())
		{
			iMass = 0;
			iInertia = 0;

			for (size_t i = 0; i < colliderCount; i++)
			{
				colliders[i].iMass = 0;
				colliders[i].iInertia = 0;
			}
			return;
		}

		for (size_t i = 0; i < colliderCount; i++)
		{
			float colliderMass = 0;
			//inertia of composite shape = sum of individual inertias
			float colliderInertia = 0;

			colliders[i].CalculateMass(colliderMass, colliderInertia);

			//if this collider is a trigger and the collider count is more than one, this collider's mass and inertia will not contribute toward the overall mass and inertia
			if (colliderCount != 1 && colliders[i].GetIsTrigger())
				continue;

			mass += colliderMass;
			inertia += colliderInertia;
		}
		iInertia = inertia == 0 ? 0 : 1.0f / inertia;
		iMass = mass == 0 ? 0 : 1.0f / mass;

		if (!isRotatable)
		{
			iInertia = 0;
			for (size_t i = 0; i < colliderCount; i++)
			{
				colliders[i].iInertia = 0;
			}
		}
	}

	bool PhysicsObject::CanBeDynamic()
	{
		for (size_t i = 0; i < colliderCount; i++)
		{
			if (!colliders[i].CanBeDynamic())
				return false;
		}
		return true;
	}

	PhysicsObject::PhysicsObject(const PhysicsObject& other) : staticFriction(other.staticFriction), dynamicFriction(other.dynamicFriction)
	{
		colliders = (Collider*)(new char[sizeof(Collider) * other.colliderCount]);
		memcpy(colliders, other.colliders, sizeof(Collider) * other.colliderCount);
		colliderCount = other.colliderCount;
		/*for (size_t i = 0; i < colliderCount; i++)
		{
			colliders[i].SetAttached(this);
		}*/

		transform = other.transform;
		velocity = other.velocity;
		angularVelocity = other.angularVelocity;
		force = other.force;
		torque = other.torque;
		bounciness = other.bounciness;
		drag = other.drag;
		angularDrag = other.angularDrag;
		iMass = other.iMass;
		iInertia = other.iInertia;
		isDynamic = other.isDynamic;
		isRotatable = other.isRotatable;
		pointer = other.pointer;
	}

	PhysicsObject::PhysicsObject(PhysicsObject&& other) : staticFriction(other.staticFriction), dynamicFriction(other.dynamicFriction)
	{
		colliders = other.colliders;
		other.colliders = nullptr;
		colliderCount = other.colliderCount;

		transform = other.transform;
		velocity = other.velocity;
		angularVelocity = other.angularVelocity;
		force = other.force;
		torque = other.torque;
		bounciness = other.bounciness;
		drag = other.drag;
		angularDrag = other.angularDrag;
		iMass = other.iMass;
		iInertia = other.iInertia;
		isDynamic = other.isDynamic;
		isRotatable = other.isRotatable;
		pointer = other.pointer;
	}

	PhysicsObject& PhysicsObject::operator=(const PhysicsObject& other)
	{
		if (colliders)
			delete[] colliders;
		colliders = (Collider*)(new char[sizeof(Collider) * other.colliderCount]);
		memcpy(colliders, other.colliders, sizeof(Collider) * other.colliderCount);
		colliderCount = other.colliderCount;


		staticFriction = other.staticFriction;
		dynamicFriction = other.dynamicFriction;
		transform = other.transform;
		velocity = other.velocity;
		angularVelocity = other.angularVelocity;
		force = other.force;
		torque = other.torque;
		bounciness = other.bounciness;
		drag = other.drag;
		angularDrag = other.angularDrag;
		iMass = other.iMass;
		iInertia = other.iInertia;

		isDynamic = other.isDynamic;
		isRotatable = other.isRotatable;
		pointer = other.pointer;

		return *this;
	}

	PhysicsObject& PhysicsObject::operator=(PhysicsObject&& other)
	{
		if (colliders)
			delete[] colliders;
		colliders = other.colliders;
		/*for (size_t i = 0; i < colliderCount; i++)
		{
			colliders[i].SetAttached(this);

		}*/
		colliderCount = other.colliderCount;
		other.colliders = nullptr;

		staticFriction = other.staticFriction;
		dynamicFriction = other.dynamicFriction;
		transform = other.transform;
		velocity = other.velocity;
		angularVelocity = other.angularVelocity;
		force = other.force;
		torque = other.torque;
		bounciness = other.bounciness;
		drag = other.drag;
		angularDrag = other.angularDrag;
		iMass = other.iMass;
		iInertia = other.iInertia;

		isDynamic = other.isDynamic;
		isRotatable = other.isRotatable;
		pointer = other.pointer;

		return *this;
	}
}

