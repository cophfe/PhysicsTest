#pragma once
class PhysicsProgram;
#include "Maths.h"
#include "Graphics.h"
#include "TextRenderer.h"
#include "LineRenderer.h"
#include "Shape.h"

class UIObject
{
public:
	enum class ANCHOR_POINT {
		BOTTOM_LEFT,
		BOTTOM_RIGHT,
		TOP_LEFT,
		TOP_RIGHT,
		CENTRE,
		COUNT
	};


	virtual void Update(PhysicsProgram& program) = 0;
	virtual void OnMouseClick(PhysicsProgram& program) = 0;
	virtual void OnMouseRelease(PhysicsProgram& program) = 0;
	virtual void Draw(PhysicsProgram& program) = 0;
	bool IsHeldDown() { return heldDown; }
	virtual void SetPosition(Vector2 newPosition) = 0;
	virtual Vector2 GetPosition() = 0;
	virtual void OnWindowChange(Vector2Int oldSize, Vector2Int newSize);

	virtual void SetEnabled(bool val) { enabled = val; }
	virtual bool GetEnabled() { return enabled; }

	static Vector2 GetPositionFromAnchoredPosition(Vector2 anchoredPosition, ANCHOR_POINT anchor, Vector2Int windowSize);
	static Vector2 GetAnchoredPositionFromPosition(Vector2 position, ANCHOR_POINT anchor, Vector2Int windowSize);
protected:
	bool heldDown = false;
	bool enabled = true;

	ANCHOR_POINT anchor;
};

