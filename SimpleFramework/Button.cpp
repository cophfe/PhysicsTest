#include "Button.h"
#include "PhysicsProgram.h";

Button::Button(Vector2 size, Vector2 position, std::string text, Vector3 textColour, Vector3 edgeColour, PhysicsProgram& program, float textScale)
	: text(text), textColour(textColour), edgeColour(edgeColour), textScale(textScale)
{
	buttonAABB.max = position + size * 0.5f;
	buttonAABB.min = position - size * 0.5f;

	program.GetTextRenderer().GetTextWidthHeight(text, textOffset.x, textOffset.y);
	textOffset.x = 0.5f * (size.x - textScale * textOffset.x);
	textOffset.y = 0.5f * (size.y - textScale * textOffset.y);
}

void Button::Update(PhysicsProgram& program)
{
	program.GetCursorPos();
}

void Button::Draw(PhysicsProgram& program)
{
	/*LineRenderer& lR = program.GetLineRenderer();
	lR.DrawLineSegment(buttonAABB.max, Vector2(buttonAABB.max.x, buttonAABB.min.y), edgeColour);
	lR.DrawLineSegment(Vector2(buttonAABB.max.x, buttonAABB.min.y), buttonAABB.min, edgeColour);
	lR.DrawLineSegment(buttonAABB.min, Vector2(buttonAABB.min.x, buttonAABB.max.y), edgeColour);
	lR.DrawLineSegment(Vector2(buttonAABB.min.x, buttonAABB.max.y), buttonAABB.max, edgeColour);*/

	TriangleRenderer& triR = program.GetTriangleRenderer();
	triR.QueueBox(buttonAABB.min, buttonAABB.max, edgeColour);

	TextRenderer& tR = program.GetTextRenderer();
	tR.QueueText(text, buttonAABB.min + textOffset, textScale, textColour);
}
