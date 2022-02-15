#include "PhysicsProgram.h"
#include "CollisionManager.h"

std::vector<Vector2> PhysicsProgram::collisionPoints;

PhysicsProgram::PhysicsProgram() : playerInput(PlayerInput(*this)), collisionManager(this), GameBase()
{
	//text.QueueText("The quick brown fox jumped over the lazy dog", Vector2(25.0f, 25.0f), 0.4f, Vector3(1.0f, 0.1f, 0.1f));
	//text.Build();

	//multiple shapes in 1 physics object aren't supported JUST yet
	PhysicsData data = PhysicsData(Vector2(0, 0), 0, false);
	AddPhysicsObject(PhysicsObject(data, new Collider(new PlaneShape(Vector2(1, 0), -gridLimits))));
	AddPhysicsObject(PhysicsObject(data, new Collider(new PlaneShape(Vector2(0, 1), -gridLimits))));
	AddPhysicsObject(PhysicsObject(data, new Collider(new PlaneShape(Vector2(-1, 0), -gridLimits))));
	AddPhysicsObject(PhysicsObject(data, new Collider(new PlaneShape(Vector2(0, -1), -gridLimits))));

}

void PhysicsProgram::Update()
{
	GameBase::Update();

	playerInput.Update();
	uiHeldDown = false;
	if (uiEnabled) {
		for (size_t i = 0; i < uiObjects.size(); i++)
		{
			uiObjects[i]->Update(*this);
		}
	}

	if (!paused)
	{
		UpdatePhysics();
		collisionManager.ResolveCollisions(pObjects);
	}
}

void PhysicsProgram::UpdatePhysics()
{
	collisionPointUpdateTime -= deltaTime;
	if (collisionPointUpdateTime < 0)
	{
		collisionPointUpdateTime = COLLISION_POINT_OFFSET;
		collisionPoints.clear(); 
	}

	//do physics
	for (auto& pObject : pObjects)
	{
		pObject.Update(*this);
	}
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

	for (auto& pObject : pObjects)
	{
		pObject.Render(*this);
	}

	//wow pretty disgusting that this function is doesn't take in *this when the others do
	playerInput.Render();

	for (size_t i = 0; i < uiObjects.size(); i++)
	{
		//wow pretty disgusting that this function is called Draw when the others are called render
		uiObjects[i]->Draw(*this);
	}

	//draw collision crosses
	for (size_t i = 0; i < collisionPoints.size(); i++)
	{
		lines.DrawCross(collisionPoints[i], 0.1f, { 0.9f, 0.1f, 0.1f });
	}

	//This call puts all the lines you've set up on screen - don't delete it or things won't work.
	//uses simple shader to draw lines and grid
	GameBase::Render();
}

void PhysicsProgram::OnMouseClick(int mouseButton)
{
	if (uiEnabled) {
		uiHeldDown = false;
		for (size_t i = 0; i < uiObjects.size(); i++)
		{
			uiObjects[i]->OnMouseClick(*this);
		}
	}
	playerInput.OnMouseClick(mouseButton);
}

void PhysicsProgram::OnMouseRelease(int mouseButton)
{
	if (uiEnabled) {
		for (size_t i = 0; i < uiObjects.size(); i++)
		{
			uiObjects[i]->OnMouseRelease(*this);
		}
	}
	playerInput.OnMouseRelease(mouseButton);
}

PhysicsObject& PhysicsProgram::AddPhysicsObject(PhysicsObject&& pObject)
{
	pObjects.emplace_back(pObject);
	return pObjects[pObjects.size() - 1];
}

UIObject* PhysicsProgram::AddUIObject(UIObject* uiObject)
{
	uiObjects.emplace_back(uiObject);
	return uiObjects[uiObjects.size() - 1];
}

void PhysicsProgram::OnWindowResize(int width, int height)
{
	GameBase::OnWindowResize(width, height);
	for (size_t i = 0; i < uiObjects.size(); i++)
	{
		uiObjects[i]->OnWindowChange(oldWindowSize, windowSize);
	}
}

void PhysicsProgram::ResetPhysics()
{
	pObjects.clear();
	collisionPoints.clear();

	PhysicsData data = PhysicsData(Vector2(0, 0), 0, false);
	AddPhysicsObject(PhysicsObject(data, new Collider(new PlaneShape(Vector2(1, 0), -gridLimits))));
	AddPhysicsObject(PhysicsObject(data, new Collider(new PlaneShape(Vector2(0, 1), -gridLimits))));
	AddPhysicsObject(PhysicsObject(data, new Collider(new PlaneShape(Vector2(-1, 0), -gridLimits))));
	AddPhysicsObject(PhysicsObject(data, new Collider(new PlaneShape(Vector2(0, -1), -gridLimits))));
}

PhysicsObject* PhysicsProgram::GetObjectUnderPoint(Vector2 point, bool includeStatic)
{
	return collisionManager.PointCast(point, pObjects, includeStatic);
}

void PhysicsProgram::ResolveCollisions()
{
	collisionManager.ResolveCollisions(pObjects);

}
