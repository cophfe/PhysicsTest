#include "PhysicsObject.h"
#include "PhysicsProgram.h"
#include "CollisionManager.h"

float PhysicsObject::gravity = 2;

PhysicsObject::PhysicsObject(PhysicsData& data, Collider* collider) : transform(Transform(data.position, data.rotation)), bounciness(data.bounciness), drag(data.drag), angularDrag(data.angularDrag)
{
	if (!data.isDynamic)
	{
		iMass = 0;
		iMomentOfInertia = 0;
	}
	else if (!data.isRotatable)
	{
		iMomentOfInertia = 0;
	}
	else if (data.mass == -1) 
	{
		if (collider != nullptr) {
			float mass = collider->CalculateMass();
			if (mass == 0)
				iMass = 0;
			else
				iMass = 1 / mass;

			float inertia = collider->CalculateInertia();
			if (inertia == 0)
				iMomentOfInertia = 0;
			else
				iMomentOfInertia = 1 / inertia;
		}
		else {
			iMass = 0;
			iMomentOfInertia = 0;
		}
	}
	
	this->collider = collider;
	if (collider != nullptr)
	{
		collider->SetAttached(this);
		if (!collider->CanBeDynamic()) 
		{
			iMass = 0;
			iMomentOfInertia = 0;
		}
	}
	


	
}

void PhysicsObject::Update(PhysicsProgram& program)
{
	transform.position += velocity * program.GetDeltaTime();
	transform.rotation += angularVelocity * program.GetDeltaTime();
	if (iMass != 0) {
		//(apply drag)
		velocity += force * iMass * program.GetDeltaTime();
		velocity.y -= gravity * program.GetDeltaTime();

		//(apply angular drag)
		angularVelocity += torque * iMomentOfInertia * program.GetDeltaTime();

	}
	
	//clear force stuff
	force = Vector2(0, 0);
	torque = 0;

	//update transform
	transform.UpdateData();
}

void PhysicsObject::Render(PhysicsProgram& program)
{
	if (collider) {
		collider->RenderShape(program);

		/*auto& aABB = collider->aABB;
		program.GetLineRenderer().DrawLineSegment(aABB.max, Vector2(aABB.max.x, aABB.min.y));
		program.GetLineRenderer().DrawLineSegment(Vector2(aABB.max.x, aABB.min.y), aABB.min);
		program.GetLineRenderer().DrawLineSegment(aABB.min, Vector2(aABB.min.x, aABB.max.y));
		program.GetLineRenderer().DrawLineSegment(Vector2(aABB.min.x, aABB.max.y), aABB.max);*/

	}
}

void PhysicsObject::GenerateAABB() {
	if (collider)
		collider->CalculateAABB(transform);
}

//z value of cross product in 3D with a and b (x and y values equal 0)
float Cross(Vector2 a, Vector2 b)
{
	return a.x * b.y - a.y * b.x;
}

void PhysicsObject::AddForceAtPosition(Vector2 force, Vector2 point)
{
	this->force += force;
	//transform.position should actually be the center point of the collider
	this->torque += Cross(point - transform.position, force);
}

void PhysicsObject::AddImpulseAtPosition(Vector2 force, Vector2 point)
{
}

PhysicsObject::~PhysicsObject()
{
	delete collider;
	collider = nullptr;
}

PhysicsObject::PhysicsObject(const PhysicsObject& other)
{
	collider = new Collider(*other.collider);
	collider->SetAttached(this);

	transform = other.transform;
	velocity = other.velocity;
	angularVelocity = other.angularVelocity;
	force = other.force;
	torque = other.torque;
	bounciness = other.bounciness;
	drag = other.drag;
	angularDrag = other.angularDrag;
	iMass = other.iMass;
	iMomentOfInertia = other.iMomentOfInertia;
	sleeping = other.sleeping;
}

PhysicsObject::PhysicsObject(PhysicsObject&& other)
{
	collider = other.collider;
	collider->SetAttached(this);

	other.collider = nullptr;

	transform = other.transform;
	velocity = other.velocity;
	angularVelocity = other.angularVelocity;
	force = other.force;
	torque = other.torque;
	bounciness = other.bounciness;
	drag = other.drag;
	angularDrag = other.angularDrag;
	iMass = other.iMass;
	iMomentOfInertia = other.iMomentOfInertia;
	sleeping = other.sleeping;
}

PhysicsObject& PhysicsObject::operator=(const PhysicsObject& other)
{
	delete collider;
	collider = new Collider(*other.collider);
	collider->SetAttached(this);

	transform = other.transform;
	velocity = other.velocity;
	angularVelocity = other.angularVelocity;
	force = other.force;
	torque = other.torque;
	bounciness = other.bounciness;
	drag = other.drag;
	angularDrag = other.angularDrag;
	iMass = other.iMass;
	iMomentOfInertia = other.iMomentOfInertia;
	sleeping = other.sleeping;

	return *this;
}

PhysicsObject& PhysicsObject::operator=(PhysicsObject&& other) 
{
	delete collider;
	collider = other.collider;
	collider->SetAttached(this);

	other.collider = nullptr;

	transform = other.transform;
	velocity = other.velocity;
	angularVelocity = other.angularVelocity;
	force = other.force;
	torque = other.torque;
	bounciness = other.bounciness;
	drag = other.drag;
	angularDrag = other.angularDrag;
	iMass = other.iMass;
	iMomentOfInertia = other.iMomentOfInertia;
	sleeping = other.sleeping;

	return *this;
}


