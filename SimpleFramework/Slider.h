#pragma once
#include "UIObject.h"
class PhysicsProgram;

class Slider :
    public UIObject
{
public:
	Slider(Vector2 size, ANCHOR_POINT anchor, Vector2 anchoredPosition, float minFill, float maxFill,
		float currentFill, Vector3 fillColour, Vector3 backgroundColour, Vector3 edgeColour, PhysicsProgram& program, float fillPadding = 0.0f,
		bool fillDirectionIsLeft = true, bool useText = true, std::string label = "", Vector3 textColour = {0.0f,0.0f,0.0f}, float textScale = 0.4f, float textPadding = 1.0f);

	void Update(PhysicsProgram& program);
	void OnMouseClick(PhysicsProgram& program) ;
	void OnMouseRelease(PhysicsProgram& program);
	void Draw(PhysicsProgram& program);
	void SetPosition(Vector2 newPosition);
	Vector2 GetPosition();
	void SetOnValueChangedCallback(void(*function)(Slider& slider, void* infoPtr, float value), void* infoPtr);
	TYPE GetType() { return TYPE::SLIDER; }

	Vector3 fillColour;
	Vector3 fillColourOnHover;
	Vector3 fillColourOnClick;
	Vector3 edgeColour;
	Vector3 backgroundColour;

	//fill amount is between 0 and 1
	float GetFillAmount() { return fillAmount * fillOffset + fillMin; }
	virtual ~Slider() = default;

private:
	AABB fillAABB;
	std::string label;
	Vector3 currentFillColour;
	Vector3 textColour;
	Vector2 fillPadding;
	Vector2 textPadding;
	void(*onValueChanged)(Slider& slider, void* infoPtr, float value) = nullptr;
	void* OnValueChangedPtr = nullptr;
	float fillAmount, fillMin, fillOffset;
	float textScale;
	bool useText;
};

