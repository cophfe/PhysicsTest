#pragma once
#include "Maths.h"
#include "Collider.h"
#include "Transform.h"
class PhysicsProgram;
class CollisionManager;

struct PhysicsData 
{
	Vector2 position;
	Vector2 scale = Vector2(1.0f, 1.0f);
	float rotation;

	float bounciness;
	float drag;
	float angularDrag;
	bool isDynamic = false;
	bool isRotatable = true;
	//will automatically calculate mass and moment of inertia if equal to -1
	float mass = -1;

	float staticFriction;
	float dynamicFriction;
	PhysicsData() = default;
	PhysicsData(Vector2 position, float rotation, bool isDynamic = true, bool isRotatable = true, float bounciness = 0.2f, 
		float drag = 0, float angularDrag = 0, float mass = -1, float staticFriction = 0.9f, float dynamicFriction = 0.5f)
		: position(position), rotation(rotation), isDynamic(isDynamic), isRotatable(isRotatable), bounciness(bounciness), drag(drag), angularDrag(angularDrag), mass(mass), staticFriction(staticFriction), dynamicFriction(dynamicFriction)
	{}
};



class PhysicsObject
{
public:
	PhysicsObject(PhysicsData& data, Collider* collider);
	
	void Update(float deltaTime);
	void Render(PhysicsProgram& program);
	void GenerateAABB();

	//getters
	inline Collider*	GetCollider()				{ return collider; }
	inline Vector2		GetPosition()				{ return transform.position; }
	inline float		GetRotation()				{ return transform.rotation; }
	
	inline Vector2		GetVelocity()				{ return velocity; }
	inline float		GetAngularVelocity()		{ return angularVelocity; }
	inline Vector2		GetForce()					{ return force; }
	inline float		GetTorque()					{ return torque; }

	inline float		GetBounciness()				{ return bounciness; }
	inline float		GetDrag()					{ return drag; }
	inline float		GetAngularDrag()			{ return angularDrag; }
	inline float		GetMass()					{ return 1.0f / iMass; }
	inline float		GetInertia()				{ return 1.0f / iMomentOfInertia; }
	inline float		GetInverseMass()			{ return iMass; }
	inline float		GetInverseInertia()			{ return iMomentOfInertia; }
	inline Transform&	GetTransform()				{ return transform; }

	//setters
	inline void	SetPosition(Vector2 pos)			{ transform.position = pos; }
	inline void	SetRotation(float rot)				{ transform.rotation = rot; }

	inline void	SetVelocity(Vector2 vel)			{ velocity = vel; }
	inline void	SetAngularVelocity(float aVel)		{ angularVelocity = aVel; }
	inline void	SetForce(Vector2 force)				{ this->force = force;}
	inline void	SetTorque(float torque)				{ this->torque = torque; }
	
	inline void	SetBounciness(float bounce) 		{ bounciness = bounce;}
	inline void	SetDrag(float drag)					{ this->drag = drag; }
	inline void	SetAngularDrag(float aDrag)			{ this->angularDrag = aDrag;}
	inline void	SetMass(float mass)					{ this->iMass = 1.0f/mass; }
	inline void	SetMomentOfInertia(float mOI)		{ iMomentOfInertia = 1.0f/mOI; }

	//adders?
	inline void AddPosition(Vector2 position)		{ transform.position += position;  }
	inline void AddForce(Vector2 force)				{ this->force += force;  }
	inline void AddTorque(float torque)				{ this->torque += torque;  }
	inline void AddVelocity(Vector2 velocity)		{ this->velocity += velocity;  }
	inline void AddAngularVelocity(float velocity)	{ angularVelocity += velocity;  }
	inline void AddImpulse(Vector2 impulse)			{ velocity += impulse * iMass;  }
	inline void AddAngularImpulse(float impulse)	{ angularVelocity += impulse * iMomentOfInertia;  }
	void AddForceAtPosition(Vector2 force, Vector2 point);
	void AddImpulseAtPosition(Vector2 force, Vector2 point);
	void AddVelocityAtPosition(Vector2 impulse, Vector2 point);

	//rule o' 5
	~PhysicsObject(); //destructor
	PhysicsObject(const PhysicsObject& other); //copy constructor
	PhysicsObject(PhysicsObject&& other); //move constructor
	PhysicsObject& operator= (const PhysicsObject& other); //copy assignment
	PhysicsObject& operator= (PhysicsObject&& other); //move assignment


protected:
	friend CollisionManager;
	friend Collider;

	//is ptr so it can be null
	Collider* collider = nullptr;

	//position values
	//should be no problem with vec2 scale since there are no child objects
	Transform transform;

	//movement values
	Vector2 velocity = Vector2(0,0);
	float angularVelocity = 0;
	Vector2 force = Vector2(0,0);
	float torque = 0;

	//movement constants
	float bounciness;
	float drag;
	float angularDrag;
	float iMass;
	float staticFriction;
	float dynamicFriction;
	//the mass moment of inertia
	float iMomentOfInertia;


	//(just in case something is not moving, so no movement calculations have to be done)
	//bool 
	//float sleepTimer = 0;
};

