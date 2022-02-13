#pragma once
#include "Maths.h"

namespace em
{
	float GetAngle(Vector2 a, Vector2 b);
	float CheckOrder(Vector2 a, Vector2 b, Vector2 c);
	float Cross(Vector2 a, Vector2 b);
	float SquareLength(Vector2 v);
	Vector2 ClosestPointOnLine(Vector2 a, Vector2 b, Vector2 point);
	Vector2 ClosestPointOnPlane(Vector2 planeNormal, float distance, Vector2 point);
	bool DoLinesIntersect(Vector2 a, Vector2 b, Vector2 c, Vector2 d);
	bool CalculateIntersectionPoint(Vector2 p, Vector2 pPlusR, Vector2 q, Vector2 qPlusS, Vector2& intersectionPoint);
};


