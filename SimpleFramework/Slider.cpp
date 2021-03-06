#include "Slider.h"
#include "PhysicsProgram.h"
#include <format>

Slider::Slider(Vector2 size, ANCHOR_POINT anchor, Vector2 anchoredPosition, float minFill, float maxFill,
	float currentFill, Vector3 fillColour, Vector3 backgroundColour, Vector3 edgeColour, PhysicsProgram& program, float fillPadding,
	bool fillDirectionIsLeft, bool useText, std::string label, Vector3 textColour, float textScale, float textPadding, bool useDecimals)
	: fillPadding(Vector2(fillPadding, fillPadding)), textScale(textScale), textPadding(Vector2(textPadding, textPadding)), textColour(textColour)
	, fillMin(minFill), backgroundColour(backgroundColour), fillColour(fillColour), useText(useText), label(label), edgeColour(edgeColour), useDecimals(useDecimals)
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

	if (textScale == 0)
	{
		AutoScaleText(program, textPadding, fillAABB.max - fillAABB.min);
	}
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
		if (useDecimals)
		{
			fillAmount = glm::clamp((program.GetScreenCursorPos().x - fillAABB.min.x) / (fillAABB.max.x - fillAABB.min.x), 0.0f, 1.0f);
		}
		else 
		{
			fillAmount = glm::round(fillOffset * glm::clamp((program.GetScreenCursorPos().x - fillAABB.min.x) / (fillAABB.max.x - fillAABB.min.x), 0.0f, 1.0f)) / fillOffset;
		}
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
		onValueChanged(*this, OnValueChangedPtr, fillAmount * fillOffset + fillMin);
		heldDown = false;
	}
}

void Slider::Draw(PhysicsProgram& program)
{
	if (!enabled) return;

	auto& triR = program.GetTriangleRenderer();
	//bg
	AABB backgroundAABB = AABB{ fillAABB.max + fillPadding, fillAABB.min - fillPadding };
	triR.QueueBox(backgroundAABB.min, backgroundAABB.max, backgroundColour);
	//fill
	triR.QueueBox(fillAABB.min, Vector2(fillAABB.min.x + fillAmount * (fillAABB.max.x - fillAABB.min.x), fillAABB.max.y), currentFillColour);

	auto& lineR = program.GetUILineRenderer();
	lineR.DrawLineSegment(backgroundAABB.max, Vector2(backgroundAABB.max.x, backgroundAABB.min.y), edgeColour);
	lineR.DrawLineSegment(Vector2(backgroundAABB.max.x, backgroundAABB.min.y), backgroundAABB.min, edgeColour);
	lineR.DrawLineSegment(backgroundAABB.min, Vector2(backgroundAABB.min.x, backgroundAABB.max.y), edgeColour);
	lineR.DrawLineSegment(Vector2(backgroundAABB.min.x, backgroundAABB.max.y), backgroundAABB.max, edgeColour);

	auto& texR = program.GetTextRenderer();
	texR.QueueText(GetSliderText(), fillAABB.min + textPadding, textScale, textColour);
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

void Slider::AutoScaleText(PhysicsProgram& program, float padding, Vector2 boxSize)
{
	float width, height;
	float save = fillAmount; //scale the slider text based on the maximum fill amount (when the text string will be the maximum size
	fillAmount = 1;
	program.GetTextRenderer().GetTextWidthHeight(GetSliderText(), width, height);
	fillAmount = save;

	//first calculate as if height is the constraining factor
	float newHeight = boxSize.y - 2 * padding;
	textScale = newHeight / height;

	//then check if the width is actually the constraining factor, if so recalculate text scale
	if (width * textScale > boxSize.x - 2 * padding)
	{
		textScale = (boxSize.x - 2 * padding) / width;
	}

	//textPadding.x = 0.5f * (boxSize.x - textScale * width);
	
	//centre on y
	textPadding.y = 0.5f * (boxSize.y - textScale * height);
}

std::string Slider::GetSliderText()
{
	std::string numText;
	if (useDecimals)
	{
		numText = std::to_string((fillAmount * fillOffset + fillMin));
		numText = numText.substr(0, numText.find(".") + 3); //format with 2 decimal points (rounds down)
	}
	else
	{
		numText = std::to_string((int)(fillAmount * fillOffset + fillMin));
	}
	numText = label + numText;
	return numText;
}
