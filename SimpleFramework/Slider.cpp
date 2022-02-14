#include "Slider.h"
#include "PhysicsProgram.h"
#include <format>

Slider::Slider(Vector2 size, ANCHOR_POINT anchor, Vector2 anchoredPosition, float minFill, float maxFill,
	float currentFill, Vector3 fillColour, Vector3 backgroundColour, PhysicsProgram& program, float fillPadding,
	bool fillDirectionIsLeft, bool useText, std::string label, Vector3 textColour, float textScale, float textPadding)
	: fillPadding(Vector2(fillPadding, fillPadding)), textScale(textScale), textPadding(Vector2(textPadding, textPadding)), textColour(textColour)
	, fillMin(minFill), backgroundColour(backgroundColour), fillColour(fillColour), useText(useText), label(label)
{
	fillOffset = maxFill - minFill;
	currentFill = glm::clamp(currentFill, minFill, maxFill);
	fillAmount = (currentFill - minFill)/ fillOffset;
	this->anchor = anchor;
	Vector2 position = GetPositionFromAnchoredPosition(anchoredPosition, anchor, program.GetWindowSize());
	fillAABB.max = position + size * 0.5f - this->fillPadding;
	fillAABB.min = position - size * 0.5f + this->fillPadding;


	if (fillColour.x * fillColour.x + fillColour.y * fillColour.y + fillColour.z * fillColour.z > 0.5f * 0.5f)
	{
		fillColourOnHover = fillColour * 0.75f;
		fillColourOnClick = fillColour * 0.5f;
	}
	else
	{
		fillColourOnHover = fillColour + Vector3(0.2f, 0.2f, 0.2f);
		fillColourOnClick = fillColour + Vector3(0.5f, 0.5f, 0.5f);
	}
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
	if (enabled && heldDown && onValueChanged)
	{
		onValueChanged(*this, OnValueChangedPtr, fillAmount * fillOffset);
		heldDown = false;
	}
}

void Slider::Draw(PhysicsProgram& program)
{
	if (!enabled) return;

	auto& triR = program.GetTriangleRenderer();
	//bg
	triR.QueueBox(fillAABB.min - fillPadding, fillAABB.max + fillPadding, backgroundColour);
	//fill
	triR.QueueBox(fillAABB.min, Vector2(fillAABB.min.x + fillAmount * (fillAABB.max.x - fillAABB.min.x), fillAABB.max.y), currentFillColour);

	auto& texR = program.GetTextRenderer();

	std::string numText = std::to_string((fillAmount * fillOffset + fillMin));
	numText = numText.substr(0, numText.find(".") + 3); //format with 2 decimal points (rounds down)
	texR.QueueText(label + numText, fillAABB.min + fillPadding + textPadding, textScale, textColour);
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
