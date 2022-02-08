#include "Slider.h"
#include "PhysicsProgram.h"

Slider::Slider(Vector2 size, ANCHOR_POINT anchor, Vector2 anchoredPosition, float minFill, float maxFill,
	float currentFill, Vector3 fillColour, Vector3 backgroundColour, PhysicsProgram& program, float fillPadding,
	bool fillDirectionIsLeft, bool useText, std::string label, Vector3 textColour, float textScale, float textPadding)
	: fillPadding(Vector2(fillPadding, fillPadding)), textScale(textScale), textPadding(Vector2(textPadding, textPadding))
	, fillMin(minFill), backgroundColour(backgroundColour), fillColour(fillColour), useText(useText), label(label)
{
	fillOffset = maxFill - minFill;
	currentFill = glm::clamp(currentFill, minFill, maxFill);
	fillAmount = (currentFill - minFill)/ fillOffset;
	this->anchor = anchor;
	Vector2 position = GetPositionFromAnchoredPosition(anchoredPosition, anchor, program.GetWindowSize());
	fillAABB.max = position + size * 0.5f - this->fillPadding;
	fillAABB.min = position - size * 0.5f + this->fillPadding;


	fillColourOnHover = fillColour * 0.73f;
	fillColourOnClick = fillColour * 0.5f;
	currentFillColour = fillColour;
}

void Slider::Update(PhysicsProgram& program)
{
	if (enabled && !heldDown)
	{
		if (fillAABB.PointCast(program.GetScreenCursorPos()))
		{
			currentFillColour = fillColourOnHover;
		}
		else
		{
			currentFillColour = fillColour;
		}
	}
	else if (heldDown)
	{
		program.SetUIHeldDown(true);
		fillAmount = glm::clamp((program.GetScreenCursorPos().x - fillAABB.min.x) / (fillAABB.max.x - fillAABB.min.x), 0.0f, 1.0f);
	}
}

void Slider::OnMouseClick(PhysicsProgram& program)
{
	if (enabled && fillAABB.PointCast(program.GetScreenCursorPos()))
	{
		heldDown = true;
		currentFillColour = fillColourOnClick;
		program.SetUIHeldDown(true);
	}
}

void Slider::OnMouseRelease(PhysicsProgram& program)
{
	if (heldDown && onValueChanged)
	{
		onValueChanged(*this, OnValueChangedPtr, fillAmount);
		heldDown = false;
	}
}

void Slider::Draw(PhysicsProgram& program)
{
	auto& triR = program.GetTriangleRenderer();
	
	//bg
	triR.QueueBox(fillAABB.min - fillPadding, fillAABB.max + fillPadding, backgroundColour);
	//fill
	triR.QueueBox(fillAABB.min, Vector2(fillAABB.min.x + fillAmount * (fillAABB.max.x - fillAABB.min.x), fillAABB.max.y), currentFillColour);

	auto& texR = program.GetTextRenderer();
	texR.QueueText(label + std::to_string((int)(fillAmount * fillOffset)), fillAABB.min + fillPadding + textPadding, textScale, textColour);
}

void Slider::SetPosition(Vector2 newPosition)
{
	Vector2 size = fillAABB.max - fillAABB.min;
	fillAABB.max = newPosition + size * 0.5f;
	fillAABB.min = newPosition - size * 0.5f;
}

Vector2 Slider::GetPosition()
{
	Vector2 size = fillAABB.max - fillAABB.min;
	return fillAABB.min + 0.5f * size;
}

void Slider::SetOnValueChangedCallback(void(*function)(Slider& slider, void* infoPtr, float value), void* infoPtr)
{
	onValueChanged = function;
	OnValueChangedPtr = infoPtr;
}
