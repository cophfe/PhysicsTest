#include "PhysicsObject.h"
#include "PhysicsProgram.h"

void PhysicsObject::Update(PhysicsProgram& program)
{
	position += velocity * program.GetDeltaTime();
	//(apply drag)
	velocity += force / iMass * program.GetDeltaTime();

	rotation += angularVelocity * program.GetDeltaTime();
	//(apply angular drag)
	angularVelocity += torque * iMomentOfInertia * program.GetDeltaTime();

	//clear force stuff
	force = Vector2(0, 0);
	torque = 0;
}

void PhysicsObject::Render(PhysicsProgram& program)
{
	if (collider)
		collider->RenderShape(program);
}

void PhysicsObject::AddForceAtPosition(Vector2 force, Vector2 point)
{
	this->force += force;
	this->torque += Cross(point - collider->GetGlobalCentrePoint(), force);
}

//z value of cross product in 3D with a and b (x and y values equal 0)
float Cross(Vector2 a, Vector2 b) 
{
	return a.x * b.y - a.y * b.x;
}
