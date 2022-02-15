#pragma once
#include "GameBase.h"
#include "PhysicsObject.h"
#include "CollisionManager.h"
#include "PlayerInput.h"
#include <forward_list>
#include "UIObject.h"

#define FPS_OFFSET 1
#define COLLISION_POINT_OFFSET 1

class PhysicsProgram : public GameBase
{
public:
	PhysicsProgram();
	void Update();
	void UpdatePhysics();
	void Render();
	void OnMouseClick(int mouseButton);
	void OnMouseRelease(int mouseButton);
	
	PhysicsObject& AddPhysicsObject(PhysicsObject&& pObject);
	UIObject* AddUIObject(UIObject* uiObject);

	//set
	void SetUIInputEnabled(bool enabled) { uiEnabled = enabled; }
	void SetPauseState(bool state) { paused = state; }
	//get
	PlayerInput GetPlayerInput() { return playerInput; }
	bool GetPauseState() { return paused; }
	inline const float GetDeltaTime() { return deltaTime; }
	inline LineRenderer& GetLineRenderer() { return lines; }
	inline LineRenderer& GetUILineRenderer() { return linesUI; }
	inline TriangleRenderer& GetTriangleRenderer() { return triangleRenderer; }
	inline TextRenderer& GetTextRenderer() { return textRenderer; }
	inline Vector2 GetCursorPos() { return cursorPos; }
	inline Vector2 GetScreenCursorPos() { return screenCursorPos; }
	inline bool GetLeftMouseDown() { return leftButtonDown; }
	inline bool GetRightMouseDown() { return rightButtonDown; }
	inline bool GetUIHeldDown() { return uiHeldDown; }
	inline void SetUIHeldDown(bool value) { uiHeldDown = value; }
	inline Vector2Int GetWindowSize() { return windowSize; }
	void OnWindowResize(int width, int height);
	void ResetPhysics();
	PhysicsObject* GetObjectUnderPoint(Vector2 point, bool includeStatic = false);

	void ResolveCollisions();
private:
	friend CollisionManager;
	static std::vector<Vector2> collisionPoints;

	std::vector<UIObject*> uiObjects;
	bool uiHeldDown;
	bool uiEnabled = true;

	std::vector<PhysicsObject> pObjects;
	PlayerInput playerInput;
	CollisionManager collisionManager;
	double lastTime = 0;
	float lastFPSUpdateTime = - FPS_OFFSET;
	float collisionPointUpdateTime = 0;
	std::string fpsText;
	bool paused = false;
};

