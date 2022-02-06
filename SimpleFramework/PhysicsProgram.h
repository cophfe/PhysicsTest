#pragma once
#include "GameBase.h"
#include "PhysicsObject.h"
#include "CollisionManager.h"
#include "PlayerInput.h"
#include <forward_list>

#define FPS_OFFSET 1

class PhysicsProgram : public GameBase
{
public:
	PhysicsProgram();
	void Update();
	void Render();
	void OnMouseClick(int mouseButton);
	void OnMouseRelease(int mouseButton);
	
	PhysicsObject& AddPhysicsObject(PhysicsObject&& pObject);

	//get
	inline const float GetDeltaTime() { return deltaTime; }
	inline LineRenderer& GetLineRenderer() { return lines; }
	inline TriangleRenderer& GetTriangleRenderer() { return triangleRenderer; }
	inline TextRenderer& GetTextRenderer() { return textRenderer; }
	inline Vector2 GetCursorPos() { return cursorPos; }
	inline Vector2 GetScreenCursorPos() { return screenCursorPos; }
private:
	std::vector<PhysicsObject> pObjects;
	PlayerInput playerInput;
	CollisionManager collisionManager;
	double lastTime = 0;
	float lastFPSUpdateTime = - FPS_OFFSET;
	std::string fpsText;
};

