#include "PhysicsProgram.h"
#include "CollisionManager.h"

PhysicsProgram::PhysicsProgram() : playerInput(PlayerInput(*this)), GameBase()
{
	//text.QueueText("The quick brown fox jumped over the lazy dog", Vector2(25.0f, 25.0f), 0.4f, Vector3(1.0f, 0.1f, 0.1f));
	//text.Build();

	//multiple shapes in 1 physics object aren't supported JUST yet
	PhysicsData data = PhysicsData(Vector2(0, 0), 0, false);
	AddPhysicsObject(PhysicsObject(data, new Collider(new LineShape(Vector2(-gridLimits, -gridLimits), Vector2(-gridLimits, gridLimits)))));
	AddPhysicsObject(PhysicsObject(data, new Collider(new LineShape(Vector2(-gridLimits, gridLimits), Vector2(gridLimits, gridLimits)))));
	AddPhysicsObject(PhysicsObject(data, new Collider(new LineShape(Vector2(gridLimits, gridLimits), Vector2(gridLimits, -gridLimits)))));
	AddPhysicsObject(PhysicsObject(data, new Collider(new LineShape(Vector2(gridLimits, -gridLimits), Vector2(-gridLimits, -gridLimits)))));

}

void PhysicsProgram::Update()
{
	GameBase::Update();

	//do physics
	for (auto& pObject : pObjects)
	{
		pObject.Update(*this);
	}

	collisionManager.ResolveCollisions(pObjects);

	//player input also
	playerInput.Update();

	
}

void PhysicsProgram::Render()
{
	//update text
	double time = glfwGetTime();
	//if FPS_OFFSET seconds has elapsed since last FPS update
	if (time - lastFPSUpdateTime> FPS_OFFSET)
	{
		lastFPSUpdateTime = time;
		fpsText = std::string("FPS: ") + std::to_string((int)(1 / (time - lastTime)));
	}
	textRenderer.QueueText(fpsText, Vector2(25.0f, 25.0f), 0.4f, Vector3(1.0f, 0.1f, 0.1f));
	
	lastTime = time;

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

	for (auto& pObject : pObjects)
	{
		pObject.Render(*this);
	}

	playerInput.Render();

	//This call puts all the lines you've set up on screen - don't delete it or things won't work.
	//uses simple shader to draw lines and grid
	GameBase::Render();
}

void PhysicsProgram::OnMouseClick(int mouseButton)
{
	playerInput.OnMouseClick(mouseButton);
}

void PhysicsProgram::OnMouseRelease(int mouseButton)
{
	playerInput.OnMouseRelease(mouseButton);
}

PhysicsObject& PhysicsProgram::AddPhysicsObject(PhysicsObject&& pObject)
{
	pObjects.emplace_back(pObject);
	return pObjects[pObjects.size() - 1];
}
