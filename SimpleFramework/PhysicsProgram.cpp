#include "PhysicsProgram.h"
#include "CollisionManager.h"

PhysicsProgram::PhysicsProgram() : GameBase()
{
}

void PhysicsProgram::Update()
{
	GameBase::Update();

	//do physics
	for (auto pObject : pObjects)
	{
		pObject.Update(*this);
	}
}

void PhysicsProgram::Render()
{

	if (leftButtonDown)
	{
		lines.DrawCircle(cursorPos, 0.2f, { 1, 0, 0 });
		OnMouseClick(0);

	}
	else if (rightButtonDown)
	{
		lines.DrawCircle(cursorPos, 0.2f, { 0, 1, 0 });
		OnMouseClick(1);
	}
	else
	{
		lines.DrawCircle(cursorPos, 0.2f, { 0, 0, 1 });
	}

	//Your drawing code goes here!

	for (auto pObject : pObjects)
	{
		pObject.Render(*this);
	}

	//This call puts all the lines you've set up on screen - don't delete it or things won't work.
	GameBase::Render();
}

void PhysicsProgram::OnMouseClick(int mouseButton)
{

}

void PhysicsProgram::AddPhysicsObject(PhysicsObject&& pObject)
{
	pObjects.emplace_front(pObject);
}
