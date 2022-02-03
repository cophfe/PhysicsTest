#pragma once
#include "GameBase.h"
#include "PhysicsObject.h"
#include "CollisionManager.h"
#include "PlayerInput.h"
#include <forward_list>

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
	inline Vector2 GetCursorPos() { return cursorPos; }
private:
	std::vector<PhysicsObject> pObjects;
	PlayerInput playerInput;
	CollisionManager collisionManager;
};

