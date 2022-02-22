#pragma once
#include "UIObject.h"

class PhysicsProgram;

class Button : public UIObject
{
public:
	Button(Vector2 size, Vector2 anchoredPosition, ANCHOR_POINT anchor, std::string text, Vector3 textColour, Vector3 colour, PhysicsProgram& program, float textScale = 0.0f, float padding = 2.0f, Vector3 edgeColour = Vector3(0,0,0));
	
	void Update(PhysicsProgram& program);
	void OnMouseClick(PhysicsProgram& program);
	void OnMouseRelease(PhysicsProgram& program);
	bool IsHeldDown() { return heldDown; }
	
	void Draw(PhysicsProgram& program);

	void SetOnClick(void(*function)(Button& button, void* infoPointer), void* infoPointer);
	//void SetOnHover(void(*function)(Button& button, void* infoPointer), void* infoPointer);

	void DisableButton(Vector3 disableColour);
	void EnableButton();

	void ChangeText(PhysicsProgram& program, std::string text, bool autoSetScale, float padding = 8, float scale = 1);
	const std::string& GetText() { return text; }
	void SetPosition(Vector2 pos);
	Vector2 GetPosition();
	TYPE GetType() { return TYPE::BUTTON; }
	~Button() = default;

	Vector3 textColour; 
	Vector3 colour;
	Vector3 colourOnHover;
	Vector3 colourOnClick;
	Vector3 edgeColour;
private:
	void AutoSetScale(PhysicsProgram& program, float padding, Vector2 boxSize);
	
	bool buttonActive = true;
	AABB buttonAABB;
	std::string text;

	Vector3 currentColour;

	float textScale;
	Vector2 textOffset;

	void(*onClick)(Button& button, void* infoPointer) = nullptr;
	//void(*onHover)(Button& button, void* infoPointer) = nullptr;

	void* onClickPtr = nullptr;
	//void* onHoverPtr = nullptr;
};

