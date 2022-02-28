#include "PhysicsProgram.h"
#include "PhysicsSystem.h"

PhysicsProgram::PhysicsProgram() : playerInput(PlayerInput(*this)), collisionManager(GetDeltaTime()), GameBase()
{
	//text.QueueText("The quick brown fox jumped over the lazy dog", Vector2(25.0f, 25.0f), 0.4f, Vector3(1.0f, 0.1f, 0.1f));
	//text.Build();

	//collisionManager.SetPhysicsDrawer(PhysicsDrawer(DrawCircle, DrawPolygon, DrawCapsule, DrawPlane, this));
	//multiple shapes in 1 physics object aren't supported JUST yet
	PhysicsData data = PhysicsData(Vector2(0, 0), 0, false);
	CreateGameObject(data, Vector3(1, 1, 1))
		->GetPhysicsObject()->AddCollider(new PlaneShape(Vector2(1, 0), -gridLimits));
	CreateGameObject(data, Vector3(1, 1, 1))
		->GetPhysicsObject()->AddCollider(new PlaneShape(Vector2(0, 1), -gridLimits));
	CreateGameObject(data, Vector3(1, 1, 1))
		->GetPhysicsObject()->AddCollider(new PlaneShape(Vector2(-1, 0), -gridLimits));
	CreateGameObject(data, Vector3(1, 1, 1))
		->GetPhysicsObject()->AddCollider(new PlaneShape(Vector2(0, -1), -gridLimits));

	collisionManager.SetCollisionCallback(OnCollision, nullptr);
}

void PhysicsProgram::Update()
{
	GameBase::Update();

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
	}

	playerInput.Update();
}

void PhysicsProgram::UpdatePhysics()
{
	collisionManager.Update();
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

	for (auto* gO : gameObjects)
	{
		gO->Render(this);
	}
	playerInput.Render();
	for (size_t i = 0; i < uiObjects.size(); i++)
	{
		//no I will not make these render functions consistant with each other, i will keep them as the messes they are
		uiObjects[i]->Draw(*this);
	}

	//draw collision points and normals
	for (size_t i = 0; i < collisionPoints.size(); i++)
	{
		lines.DrawCross(collisionPoints[i], 0.05f, { 0.9f, 0.1f, 0.1f });
		lines.DrawLineSegment(collisionPoints[i], collisionPoints[i] + collisionNormals[i] * 0.3f, {0.1f, 0.5f, 0.9f});
	}

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

void PhysicsProgram::OnKeyPressed(int key)
{
	playerInput.OnKeyPressed(key);
}

void PhysicsProgram::OnKeyReleased(int key)
{
	playerInput.OnKeyReleased(key);
}

void PhysicsProgram::DeleteGameObject(GameObject* object)
{
	gameObjects.erase(std::remove(gameObjects.begin(), gameObjects.end(), object));
	collisionManager.DeletePhysicsBody(object->body);
	delete object;
}

GameObject* PhysicsProgram::CreateGameObject(PhysicsData data, Vector3 colour)
{
	gameObjects.push_back(new GameObject(data, &collisionManager, colour));
	return gameObjects[gameObjects.size() - 1];
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
	//resets stuff
	for (size_t i = 0; i < gameObjects.size(); i++)
	{
		delete gameObjects[i];
	}
	gameObjects.clear();
	collisionManager.ClearPhysicsBodies();
	collisionPoints.clear();

	PhysicsData data = PhysicsData(Vector2(0, 0), 0, false);
	CreateGameObject(data, Vector3(1, 1, 1))->GetPhysicsObject()->AddCollider(new PlaneShape(Vector2(1, 0), -gridLimits));
	CreateGameObject(data, Vector3(1, 1, 1))->GetPhysicsObject()->AddCollider(new PlaneShape(Vector2(0, 1), -gridLimits));
	CreateGameObject(data, Vector3(1, 1, 1))->GetPhysicsObject()->AddCollider(new PlaneShape(Vector2(-1, 0), -gridLimits));
	CreateGameObject(data, Vector3(1, 1, 1))->GetPhysicsObject()->AddCollider(new PlaneShape(Vector2(0, -1), -gridLimits));

	//testing for physics problems
	//data = PhysicsData(Vector2(5, -13), glm::radians(45.0f));
	//CreateGameObject(data, Vector3(1, 0, 0))->GetPhysicsObject()->AddCollider(PolygonShape::GetRegularPolygonCollider(1, 4));
	//data.position.y += 2.1f;
	//CreateGameObject(data, Vector3(1, 0, 0))->GetPhysicsObject()->AddCollider(PolygonShape::GetRegularPolygonCollider(1, 4));

}

PhysicsObject* PhysicsProgram::GetObjectUnderPoint(Vector2 point, bool includeStatic, bool includeTriggers)
{
	return collisionManager.PointCast(point, includeStatic, includeTriggers);
}

GameObject* PhysicsProgram::GetGameObjectUnderPoint(Vector2 point, bool includeStatic, bool includeTriggers)
{
	for (auto* gameObject : gameObjects)
	{
		for (size_t j = 0; j < gameObject->GetPhysicsObject()->GetColliderCount(); j++)
		{
			Collider&  c = gameObject->GetPhysicsObject()->GetCollider(j);

			if ((includeTriggers || !c.GetIsTrigger()) && (includeStatic || gameObject->GetPhysicsObject()->GetInverseMass() != 0) && c.GetShape()->PointCast(point, gameObject->GetPhysicsObject()->GetTransform()))
			{
				return gameObject;
			}
		}
	}
	return nullptr;
}

void PhysicsProgram::DrawShape(Shape* shape, Transform shapeTransform, Vector3 shapeColour, void* physicsProgram)
{
	switch (shape->GetType())
	{
	case SHAPE_TYPE::CIRCLE:
		DrawCircle(shape, shapeTransform, shapeColour, physicsProgram);
		break;
	case SHAPE_TYPE::POLYGON:
		DrawPolygon(shape, shapeTransform, shapeColour, physicsProgram);
		break;
	case SHAPE_TYPE::CAPSULE:
		DrawCapsule(shape, shapeTransform, shapeColour, physicsProgram);
		break;
	case SHAPE_TYPE::PLANE:
		DrawPlane(shape, shapeTransform, shapeColour, physicsProgram);
		break;
	}
}

void PhysicsProgram::DrawCircle(Shape* shape, Transform& shapeTransform, Vector3 shapeColour, void* physicsProgram)
{
	PhysicsProgram* program = (PhysicsProgram*)physicsProgram;
	CircleShape* circle = (CircleShape*)shape;

	Vector2 cP = shapeTransform.TransformPoint(circle->centrePoint);
	program->GetLineRenderer().DrawCircle(cP, circle->radius, shapeColour);
	//program->GetLineRenderer().DrawLineSegment(shapeTransform.TransformPoint(circle->centrePoint + Vector2(0, circle->radius)), shapeTransform.TransformPoint(circle->centrePoint + Vector2(0, circle->radius * 0.5f)), shapeColour);
}

void PhysicsProgram::DrawPolygon(Shape* shape, Transform& shapeTransform, Vector3 shapeColour, void* physicsProgram)
{
	PhysicsProgram* program = (PhysicsProgram*)physicsProgram;
	PolygonShape* polygon = (PolygonShape*)shape;

	auto& lines = program->GetLineRenderer();
	for (size_t i = 0; i < polygon->pointCount; i++)
	{
		lines.AddPointToLine(shapeTransform.TransformPoint(polygon->points[i]), shapeColour);
	}
	lines.FinishLineLoop();
}

void PhysicsProgram::DrawCapsule(Shape* shape, Transform& shapeTransform, Vector3 shapeColour, void* physicsProgram)
{
	PhysicsProgram* program = (PhysicsProgram*)physicsProgram;
	CapsuleShape* capsule = (CapsuleShape*)shape;

	//if the radius is big enough, this is a capsule, otherwise treat it as a line
	if (capsule->radius > 0.01f)
	{
		Vector2 a = shapeTransform.TransformPoint(capsule->pointA);
		Vector2 b = shapeTransform.TransformPoint(capsule->pointB);
		auto& lR = program->GetLineRenderer();
		lR.DrawCircle(a, capsule->radius, shapeColour, 32);
		lR.DrawCircle(b, capsule->radius, shapeColour, 32);

		Vector2 radiusAddition = glm::normalize(a - b) * capsule->radius;
		radiusAddition = { radiusAddition.y, -radiusAddition.x };

		lR.DrawLineSegment(a + radiusAddition, b + radiusAddition, shapeColour);
		lR.DrawLineSegment(a - radiusAddition, b - radiusAddition, shapeColour);
	}
	else {
		LineRenderer& lR = program->GetLineRenderer();
		lR.DrawLineSegment(shapeTransform.TransformPoint(capsule->pointA),
			shapeTransform.TransformPoint(capsule->pointB), shapeColour);
	}
}

void PhysicsProgram::DrawPlane(Shape* shape, Transform& shapeTransform, Vector3 shapeColour, void* physicsProgram)
{
	PhysicsProgram* program = (PhysicsProgram*)physicsProgram;
	PlaneShape* plane = (PlaneShape*)shape;

	auto& lR = program->GetLineRenderer();

	Vector2 tNormal = shapeTransform.TransformDirection(plane->normal);

	Vector2 tPlanePoint = shapeTransform.TransformPoint(plane->normal * plane->distance);
	float tDist = glm::dot(plane->normal, tPlanePoint);

	Vector2 tangent = 1000.0f * Vector2{ tNormal.y, -tNormal.x };
	lR.DrawLineSegment(tPlanePoint + tangent, tPlanePoint - tangent, shapeColour);
}

std::deque<Vector2> PhysicsProgram::collisionPoints;
std::deque<Vector2> PhysicsProgram::collisionNormals;

bool PhysicsProgram::OnCollision(CollisionData& data, void* infoPtr)
{
	if (data.pointCount == 2)
		collisionPoints.push_back(0.5f * (data.collisionPoints[0] + data.collisionPoints[1]));
	else
		collisionPoints.push_back(data.collisionPoints[0]);
	collisionNormals.push_back(data.collisionNormal);

	if (collisionPoints.size() > 300)
	{
		collisionPoints.pop_front();
		collisionNormals.pop_front();
	}

	return true;
}

PhysicsProgram::~PhysicsProgram()
{
	for (size_t i = 0; i < uiObjects.size(); i++)
	{
		delete uiObjects[i];
		uiObjects[i] = nullptr;
	}

	for (size_t i = 0; i < gameObjects.size(); i++)
	{
		delete gameObjects[i];
		gameObjects[i] = nullptr;
	}
}
