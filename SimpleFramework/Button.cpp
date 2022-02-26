#include "Button.h"
#include "PhysicsProgram.h"

Button::Button(Vector2 size, Vector2 anchoredPosition, ANCHOR_POINT anchor, std::string text, Vector3 textColour, Vector3 colour, PhysicsProgram& program, float textScale, float padding, Vector3 edgeColour)
	: text(text), textColour(textColour), colour(colour), edgeColour(edgeColour)
{
	//if textScale == 0 calculate it automatically
	if (textScale == 0)
	{
		AutoSetScale(program, padding, size);
	}
	else {
		float width, height;
		program.GetTextRenderer().GetTextWidthHeight(text, width, height);
		textOffset.x = 0.5f * (size.x - textScale * width);
		textOffset.y = 0.5f * (size.y - textScale * height);
		this->textScale = textScale;
	}

	this->anchor = anchor;
	Vector2 position = GetPositionFromAnchoredPosition(anchoredPosition, anchor, program.GetWindowSize());
	buttonAABB.max = position + size * 0.5f;
	buttonAABB.min = position - size * 0.5f;

	currentColour = colour;
	if (colour.x * colour.x + colour.y * colour.y + colour.z * colour.z > 0.5f * 0.5f)
	{
		colourOnHover = colour * 0.75f;
		colourOnClick = colour * 0.5f;
	}
	else
	{
		colourOnHover = colour + Vector3(0.2f,0.2f,0.2f);
		colourOnClick = colour + Vector3(0.5f, 0.5f, 0.5f);
	}
	
}

void Button::Update(PhysicsProgram& program)
{
	if (buttonActive && enabled && !heldDown)
	{
		if (buttonAABB.PointCast(program.GetScreenCursorPos()))
		{
			currentColour = colourOnHover;
		}
		else
		{
			currentColour = colour;
		}
	}
	else if (heldDown)
	{
		program.SetUIHeldDown(true);
	}

	
	
}

void Button::OnMouseClick(PhysicsProgram& program)
{
	if (buttonActive && enabled && buttonAABB.PointCast(program.GetScreenCursorPos()))
	{
		heldDown = true;
		currentColour = colourOnClick;
		program.SetUIHeldDown(true);
	}
}

void Button::OnMouseRelease(PhysicsProgram& program)
{
	if (buttonActive && enabled && heldDown)
	{
		heldDown = false;
		if (buttonAABB.PointCast(program.GetScreenCursorPos()) && onClick != nullptr)
		{
			currentColour = colourOnHover;
			onClick(*this, onClickPtr);
		}
		else
			currentColour = colour;
	}
}

void Button::Draw(PhysicsProgram& program)
{
	if (!enabled) return;

	TriangleRenderer& triR = program.GetTriangleRenderer();
	triR.QueueBox(buttonAABB.min, buttonAABB.max, currentColour);

	TextRenderer& tR = program.GetTextRenderer();
	tR.QueueText(text, buttonAABB.min + textOffset, textScale, textColour);

	LineRenderer& lR = program.GetUILineRenderer();
	lR.DrawLineSegment(buttonAABB.max, Vector2(buttonAABB.max.x, buttonAABB.min.y), edgeColour);
	lR.DrawLineSegment(Vector2(buttonAABB.max.x, buttonAABB.min.y), buttonAABB.min, edgeColour);
	lR.DrawLineSegment(buttonAABB.min, Vector2(buttonAABB.min.x, buttonAABB.max.y), edgeColour);
	lR.DrawLineSegment(Vector2(buttonAABB.min.x, buttonAABB.max.y), buttonAABB.max, edgeColour);
}

void Button::SetOnClick(void(*function)(Button& button, void* infoPointer), void* infoPointer)
{
	onClick = function;
	onClickPtr = infoPointer;
}

void Button::DisableButton(Vector3 disableColour)
{
	if (!enabled) return;
	currentColour = disableColour;
	buttonActive = false;
	heldDown = false;
}

void Button::EnableButton()
{
	if (!enabled) return;
	if (!buttonActive) currentColour = colour;
	buttonActive = true;
}

void Button::ChangeText(PhysicsProgram& program, std::string text, bool autoSetScale, float padding, float scale)
{
	Vector2 size = buttonAABB.max - buttonAABB.min;
	if (autoSetScale)
	{
		AutoSetScale(program, padding, size);
	}
	else {
		float width, height;
		program.GetTextRenderer().GetTextWidthHeight(text, width, height);
		textOffset.x = 0.5f * (size.x - scale * width);
		textOffset.y = 0.5f * (size.y - scale * height);
		textScale = scale;
	}
}

void Button::SetPosition(Vector2 pos)
{
	Vector2 size = buttonAABB.max - buttonAABB.min;
	buttonAABB.max = pos + size * 0.5f;
	buttonAABB.min = pos - size * 0.5f;
}

Vector2 Button::GetPosition()
{
	Vector2 size = buttonAABB.max - buttonAABB.min;
	return buttonAABB.min + 0.5f * size;
}

void Button::AutoSetScale(PhysicsProgram& program, float padding, Vector2 boxSize)
{
	float width, height;
	program.GetTextRenderer().GetTextWidthHeight(text, width, height);
	
	//first calculate as if height is the constraining factor
	float newHeight = boxSize.y - 2 * padding;
	textScale = newHeight / height;

	//then check if the width is actually the constraining factor, if so recalculate text scale
	if (width * textScale > boxSize.x - 2 * padding)
	{
		textScale = (boxSize.x - 2 * padding) / width;
	}

	textOffset.x = 0.5f * (boxSize.x - textScale * width);
	textOffset.y = 0.5f * (boxSize.y - textScale * height);
}
//
//void Button::SetOnHover(void(*function)(Button& button, void* infoPointer), void* infoPointer)
//{
//	onHover = function;
//	onHoverPtr = infoPointer;
//}
