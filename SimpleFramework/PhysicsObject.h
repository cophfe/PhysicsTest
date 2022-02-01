#pragma once
#include "Maths.h"
#include "Collider.h"
class PhysicsProgram;

struct PhysicsData 
{
	glm::vec2 initialPosition;
	glm::vec2 initialScale = glm::vec2(1.0f, 1.0f);
	float rotation;
	glm::vec2 initialVelocity;
	float initialAngularVelocity;

	float bounciness = 0.4f;
	float drag;
	float angularDrag;
	bool isKinematic = true;
	bool isRotatable = true;
	//will automatically calculate mass and moment of inertia if equal to -1
	float mass = -1;
	float momentOfInertia = -1;
};

class PhysicsObject
{
public:

	PhysicsObject(PhysicsData& data, Collider* collider);
	
	void Update(PhysicsProgram& program);
	void Render(PhysicsProgram& program);

	//getters
	inline Collider*	GetCollider()				{ return collider; }
	inline Vector2		GetPosition()				{ return position; }
	inline float		GetRotation()				{ return rotation; }
	inline float		GetScale()					{ return scale; }
	
	inline Vector2		GetVelocity()				{ return velocity; }
	inline float		GetAngularVelocity()		{ return angularVelocity; }
	inline Vector2		GetForce()					{ return force; }
	inline float		GetTorque()					{ return torque; }

	inline float		GetBounciness()				{ return bounciness; }
	inline float		GetDrag()					{ return drag; }
	inline float		GetAngularDrag()			{ return angularDrag; }
	inline float		GetMass()					{ return 1.0f / iMass; }
	inline float		GetMomentOfInertia()		{ return 1.0f / iMomentOfInertia; }
	inline float		GetInverseMass()			{ return iMass; }
	inline float		GetInverseMomentOfInertia()	{ return iMomentOfInertia; }

	inline Matrix2x2&	GetTransform()				{ return transform;}
	//setters
	inline void	SetPosition(Vector2 pos)			{ position = pos; }
	inline void	SetRotation(float rot)				{ rotation = rot;}
	inline void	SetScale(float scale)				{ this->scale = scale;}

	inline void	SetVelocity(Vector2 vel)			{ velocity = vel;}
	inline void	SetAngularVelocity(float aVel)		{ angularVelocity = aVel; }
	inline void	SetForce(Vector2 force)				{ this->force = force;}
	inline void	SetTorque(float torque)				{ this->torque = torque; }
	
	inline void	SetBounciness(float bounce) 		{ bounciness = bounce;}
	inline void	SetDrag(float drag)					{ this->drag = drag; }
	inline void	SetAngularDrag(float aDrag)			{ this->angularDrag = aDrag;}
	inline void	SetMass(float mass)					{ this->iMass = 1.0f/mass; }
	inline void	SetMomentOfInertia(float mOI)		{ iMomentOfInertia = 1.0f/mOI; }

	//adders?
	inline void AddForce(Vector2 force)				{ this->force += force; }
	inline void AddVelocity(Vector2 velocity)		{ this->velocity += velocity; }
	inline void AddImpulse(Vector2 impulse)			{ velocity += impulse * iMass; }
	void AddForceAtPosition(Vector2 force, Vector2 point);

protected:
	//collider connected to physicsObject
	//is ptr so it can be null
	Collider* collider = nullptr;

	//position values
	//should be no problem with vec2 scale since there are no child objects
	Vector2 position;
	float scale;
	float rotation;

	Matrix2x2 transform;

	//movement values
	Vector2 velocity;
	float angularVelocity;
	Vector2 force;
	float torque;

	//movement constants
	float bounciness;
	float drag;
	float angularDrag;
	float iMass;
	float iMomentOfInertia;

};

