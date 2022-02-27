#include "fzx.h"
#include <stdexcept>
#include <format>
#include <string>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// POLYGON
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
namespace fzx
{
	PolygonShape::PolygonShape(Vector2* vertices, int vertexCount)
	{
		//(this function organises vertices, calculates concave hull and centerpoint)
		OrganisePoints(vertices, vertexCount);

		//CalculateNormals();
	}

	bool PolygonShape::PointCast(Vector2 point, Transform& transform)
	{
		point = transform.InverseTransformPoint(point);
		//draw a line. if it intersects once with the edges of the polygon, then it is inside the shape

		/*int intersectCount = 0;
		Vector2 ray = Vector2(1000000, 0);
		Vector2 lastPoint = points[pointCount - 1];
		for (size_t i = 0; i < pointCount; i++)
		{
			intersectCount += (int)em::DoLinesIntersect(point, point + ray, lastPoint, points[i]);
			lastPoint = points[i];

		}*/

		Vector2 lastPoint = points[pointCount - 1];
		for (size_t i = 0; i < pointCount; i++)
		{
			if (em::CheckOrder(lastPoint, points[i], point) < 0)
				return false;
			lastPoint = points[i];
		}
		return true;

		//return intersectCount == 1;

	}

	void PolygonShape::CalculateMass(float& mass, float& inertia, float density)
	{
		//centrepoint is the average of the compositional triangles centrepoints, weighted by area
		Vector2 polyCentre = Vector2(0, 0);
		//mass moment of inertia is equal to the sum of the compositional triangle's mass moments
		float in = 0;
		float area = 0;

		for (size_t i = 0; i < pointCount; i++)
		{
			Vector2& a = this->points[i], b = this->points[(i + 1) % pointCount];

			Vector2 triCentre = (a + b) / 3.0f;
			float triArea = 0.5f * em::Cross(a, b);
			area += triArea;

			//fun fact: the squared length of a vector is the same as the dot product of it with itself
			in += (em::SquareLength(a) + em::SquareLength(b) + glm::dot(a, b)) * triArea / 6;
			//triangle area * triangle centre
			polyCentre += triArea * triCentre;
		}
		in = density * in;

		//set final values
		centrePoint = polyCentre / (area); //divide by total area to get the centrepoint
		mass = area * density; //multiply by density
		inertia = in - mass * em::SquareLength(centrePoint);//(translate mass moment of inertia to be relative to centrepoint)
	}

	AABB PolygonShape::CalculateAABB(Transform& transform)
	{
		//return AABB{ Vector2{INFINITY, INFINITY}, Vector2{-INFINITY, -INFINITY} };

		Vector2 point = transform.TransformPoint(points[0]);

		float xMax = point.x;
		float xMin = point.x;

		float yMax = point.y;
		float yMin = point.y;

		for (size_t i = 1; i < pointCount; i++)
		{
			point = transform.TransformPoint(points[i]);
			xMax = std::max(point.x, xMax);
			yMax = std::max(point.y, yMax);
			xMin = std::min(point.x, xMin);
			yMin = std::min(point.y, yMin);
		}

		return { Vector2(xMax, yMax), Vector2(xMin, yMin) };
	}

	SHAPE_TYPE PolygonShape::GetType()
	{
		return SHAPE_TYPE::POLYGON;
	}

	Vector2 PolygonShape::GetCentrePoint()
	{
		return centrePoint;
	}

	Shape* PolygonShape::Clone()
	{
		return new PolygonShape(*this);
	}

	Vector2 PolygonShape::Support(Vector2 v, Transform& transform)
	{
		v = transform.InverseTransformDirection(v);

		Vector2 p = points[0];
		float d = glm::dot(v, p);
		float d1;

		for (char i = 1; i < pointCount; i++)
		{
			d1 = glm::dot(v, points[i]);
			if (d1 > d)
			{
				p = points[i];
				d = d1;
			}
		}

		return transform.TransformPoint(p);
	}

	PolygonShape* PolygonShape::GetRegularPolygonCollider(float radius, int pointCount)
	{
		if (pointCount > fzx_max_vertices) {

			const std::string s = std::to_string(fzx_max_vertices) + "\n";
			throw std::runtime_error(std::string("Error: Polygons have a max vertex count of ") + s);
			pointCount = fzx_max_vertices;
		}

		Vector2 points[fzx_max_vertices];

		float iPointCount = glm::two_pi<float>() / pointCount;
		for (size_t i = 0; i < pointCount; i++)
		{
			points[i] = Vector2(radius * cos((i + 0.5f) * iPointCount), radius * sin((i + 0.5f) * iPointCount));
		}

		return new PolygonShape(points, pointCount);
	}

	bool PolygonShape::OrganisePoints(Vector2* points, int pointCount, bool clipPoints)
	{
		//first create convex hull using gift wrapping algorithm https://en.wikipedia.org/wiki/Gift_wrapping_algorithm
		//this should both discard all points that make the shape concave, and organise the points in a counterclockwise manner

		//the minimum and maximum on any axis is guaranteed to be on the convex hull, so find that for the first point

		int startPoint = 0;
		float minX = points[0].x;
		for (int i = 1; i < pointCount; i++)
		{
			if (points[i].x < minX)
			{
				minX = points[i].x;
				startPoint = i;
			}
		}

		int lastPointOnHull = startPoint;
		int size = 0;
		int endPoint = 0;
		do {
			if (size >= fzx_max_vertices)
			{
				if (!clipPoints)
				{
					const std::string s = std::to_string(fzx_max_vertices) + "\n";
					throw std::runtime_error(std::string("Error: Polygons have a max vertex count of ") + s);
				}
				size = fzx_max_vertices - 1;
				break;
			}

			this->points[size] = points[lastPointOnHull];
			endPoint = 0;
			for (int i = 0; i < pointCount; i++)
			{
				float cross = em::CheckOrder(points[lastPointOnHull], points[endPoint], points[i]);

				// if:
				// the endpoint is the last hull point
				// point i is further to the right than the last point
				// point i is the same amount to the right as the last, but is longer
				//
				// set the new endpoint to point i;
				if (endPoint == lastPointOnHull || cross < 0
					|| (cross == 0 && (em::SquareLength(points[lastPointOnHull] - points[i]) > em::SquareLength(points[lastPointOnHull] - points[endPoint]))))
				{
					endPoint = i;
				}
			}
			size++;
			lastPointOnHull = endPoint;
		} while (endPoint != startPoint);
		//now set correct pointCount
		this->pointCount = size;

		//returns whether point count changed or not
		return size != pointCount;
	}

	//WINDING ORDER: COUNTER CLOCKWISE
	//void PolygonShape::CalculateNormals()
	//{
	//	//calculate normals, assuming counter clockwise order
	//	for (size_t i = 0; i < pointCount; i++)
	//	{
	//		int j = (i + 1) % pointCount;
	//		Vector2 delta = glm::normalize(points[i] - points[j]);
	//		delta = Vector2(-delta.y, delta.x);
	//		normals[i] = delta;
	//	}
	//}

	void PolygonShape::CalculateCentrePoint()
	{
		Vector2 average = Vector2(0, 0);
		for (size_t i = 0; i < pointCount; i++)
		{
			average += points[i];
		}
		average /= pointCount;

		centrePoint = average;
	}
}