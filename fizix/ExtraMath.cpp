#include "fzx.h"

namespace fzx
{
	namespace em
	{
		float GetAngle(Vector2 a, Vector2 b) {
			return acos(glm::dot(a, b));
		}

		float CheckOrder(Vector2 a, Vector2 b, Vector2 c) {

			// > 0 : c is to the left of ab (abc is counterclockwise order)
			// < 0 : c is to the right of ab (abc is clockwise order)
			// == 0 : c is in front or behind ab (abc is a line)

			return Cross(b - a, c - a);
		}

		//Cross product of a x b is:
		// ( a.y * b.z - a.z * b.y,
		//   a.z * b.x - a.x * b.z,
		//	 a.x * b.y - a.y * a.x )


		//This isn't technically the cross product, its the z component of the cross product of two 3D vectors
		// the x and y components are always zero when a and b have z values of zero (like when we use Vector2s)
		//it is useful a lot of the time
		float Cross(Vector2 a, Vector2 b) {
			return (a.x * b.y - b.x * a.y);;
		}

		Vector2 GetPerpendicularTowardsPoint(Vector2 a, Vector2 b, Vector2 point)
		{
			//triple cross method in 2d
			Vector2 AB = b - a;
			float cross = em::Cross(AB, point - a);
			return glm::normalize(Vector2(-cross * AB.y, cross * AB.x));
		}

		Vector2 TripleCross(Vector2 a, Vector2 b, Vector2 c)
		{
			//just a more transparent version of GetPerpindicularTowardsPoint, literaly just (a x b) x c
			float cross = em::Cross(a, b);
			return glm::normalize(Vector2(-cross * c.y, cross * c.x));
		}

		float Sq(float f)
		{
			return f * f;
		}

		Vector2 NormalizeSafe(Vector2 v, Vector2 ifZero)
		{
			float len = glm::length(v);
			return (isnan(len) || len == 0) ? ifZero : v / len;
		}

		float SquareLength(Vector2 v) {
			return v.x * v.x + v.y * v.y;
		}

		Vector2 ClosestPointOnLine(Vector2 a, Vector2 b, Vector2 point)
		{
			Vector2 lineDelta = b - a;
			float t = glm::dot(point - a, lineDelta) / SquareLength(lineDelta); //div by lineDelta means nothing needs to be normalised
			return a + (glm::min(glm::max(t, 0.0f), 1.0f) * lineDelta);
		}

		Vector2 ClosestPointOnPlane(Vector2 planeNormal, float distance, Vector2 point)
		{
			if (glm::dot(planeNormal, point) <= 0)
			{
				return point;
			}
			else {
				Vector2 tangent = { -planeNormal.y, planeNormal.x };
				return glm::dot(point, tangent) * tangent + planeNormal * distance;
			}
		}

		bool DoLinesIntersect(Vector2 a, Vector2 b, Vector2 c, Vector2 d)
		{
			//idk how this works but it does, its from the internet
			//i already had an order checker function for ordering 
			return CheckOrder(a, c, d) > 0 != CheckOrder(b, c, d) > 0 &&
				CheckOrder(a, b, c) > 0 != CheckOrder(a, b, d) > 0;
		}

		//this is based on this stackoverflow answer https://stackoverflow.com/a/565282
		bool CalculateIntersectionPoint(Vector2 a, Vector2 b, Vector2 c, Vector2 d, Vector2& intersectionPoint)
		{
			Vector2 deltaA = b - a;
			Vector2 deltaB = d - c;

			float dACrossDB = Cross(deltaA, deltaB);

			if (dACrossDB == 0)
			{
				//if Cross(c - a, deltaA) == 0 they are on the same line (not guaranteed intersecting, lines still have to be overlapping)
				//whatever hopefully it won't come up
				return false;
			}

			//percent of line CD the intersection point is at
			float t = Cross(c - a, deltaA) / dACrossDB;
			//percent of line AB the intersection point is at
			float t2 = Cross(c - a, deltaB) / dACrossDB;

			intersectionPoint = c + t * deltaB;
			return t >= 0 && t <= 1 && t2 >= 0 && t2 <= 1;
		}
		Vector2 GetPerpendicularClockwise(Vector2 v)
		{
			return Vector2(v.y, -v.x);
		}
		Vector2 GetPerpendicularCounterClockwise(Vector2 v)
		{
			return Vector2(-v.y, v.x);
		}
	}
}