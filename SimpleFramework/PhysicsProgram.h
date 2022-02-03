#pragma once
#include "GameBase.h"
#include "PhysicsObject.h"
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
	
	void AddPhysicsObject(PhysicsObject&& pObject);

	//get
	inline const float GetDeltaTime() { return deltaTime; }
	inline LineRenderer& GetLineRenderer() { return lines; }
private:
	std::forward_list<PhysicsObject> pObjects;
	PlayerInput playerInput;
};

