#pragma once
#include "Graphics.h"
#include "TextRenderer.h"
#include "LineRenderer.h"
#include "Shape.h"

class PhysicsProgram;

class Button
{
public:
	Button(Vector2 size, Vector2 position, std::string text, Vector3 textColour, Vector3 edgeColour, PhysicsProgram& program, float textScale = 1.0f);
	
	void Update(PhysicsProgram& program);
	void Draw(PhysicsProgram& program);

	void SetOnClick(void(*function));
	void SetOnHover(void(*function));
	
private:
	AABB buttonAABB;
	std::string text;
	Vector3 textColour; 
	Vector3 edgeColour;
	float textScale;
	Vector2 textOffset;
	void(*onClick) = nullptr;
	void(*onHover) = nullptr;
};

