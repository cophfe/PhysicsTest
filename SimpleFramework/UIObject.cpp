#include "UIObject.h"
#include "PhysicsProgram.h"

void UIObject::OnWindowChange(Vector2Int oldSize, Vector2Int newSize)
{
	Vector2 pos;
	switch (anchor)
	{
	case ANCHOR_POINT::BOTTOM_LEFT:
	{
		//do nothing, we good
		break;
	}
	case ANCHOR_POINT::BOTTOM_RIGHT:
	{
		pos = GetPosition();
		pos.x = pos.x - oldSize.x + newSize.x;
		SetPosition(pos);
		break;
	}
	case ANCHOR_POINT::TOP_LEFT:
		pos = GetPosition();
		pos.y = pos.y - oldSize.y + newSize.y;
		SetPosition(pos);
		break;
	case ANCHOR_POINT::TOP_RIGHT:
		pos = GetPosition();
		pos.y = pos.y - oldSize.y + newSize.y;
		pos.x = pos.x - oldSize.x + newSize.x;
		SetPosition(pos);
		break;
	case ANCHOR_POINT::CENTRE:
		pos = GetPosition();
		pos.x = pos.x - 0.5f * (oldSize.x - newSize.x);
		pos.y = pos.y - 0.5f * (oldSize.y - newSize.y);
		SetPosition(pos);
		break;
	}
}

Vector2 UIObject::GetPositionFromAnchoredPosition(Vector2 anchoredPosition, ANCHOR_POINT anchor, Vector2Int windowSize)
{
	switch (anchor)
	{
	case ANCHOR_POINT::BOTTOM_LEFT:
		return anchoredPosition;
	case ANCHOR_POINT::BOTTOM_RIGHT:
		return Vector2(windowSize.x - anchoredPosition.x, anchoredPosition.y);
	case ANCHOR_POINT::TOP_LEFT:
		return Vector2(anchoredPosition.x, windowSize.y - anchoredPosition.y);
	case ANCHOR_POINT::TOP_RIGHT:
		return Vector2(windowSize.x - anchoredPosition.x, windowSize.y - anchoredPosition.y);
	case ANCHOR_POINT::CENTRE:
		return anchoredPosition + 0.5f * (Vector2)windowSize;
	}
	return anchoredPosition;
}

Vector2 UIObject::GetAnchoredPositionFromPosition(Vector2 position, ANCHOR_POINT anchor, Vector2Int windowSize)
{
	switch (anchor)
	{
	case ANCHOR_POINT::BOTTOM_LEFT:
		return position;
	case ANCHOR_POINT::BOTTOM_RIGHT:
		return Vector2(windowSize.x - position.x, position.y);
	case ANCHOR_POINT::TOP_LEFT:
		return Vector2(position.x, windowSize.y - position.y);
	case ANCHOR_POINT::TOP_RIGHT:
		return Vector2(windowSize.x - position.x, windowSize.y - position.y);
	case ANCHOR_POINT::CENTRE:
		return position - 0.5f * (Vector2)windowSize;
	}
	return position;
}
