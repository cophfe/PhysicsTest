#include "Shape.h"
#include "PhysicsObject.h"
#include "PhysicsProgram.h"
#include "CollisionManager.h"

// non member functions
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static float GetAngle(Vector2 a, Vector2 b) {
	return acos(glm::dot(a, b));
}

static float Cross(Vector2 a, Vector2 b, Vector2 c) {
	//line made of a and b, is c to the left or right

	// > 0 : c is to the left
	// < 0 : c is to the right
	// == 0 : c is in front
	return ((b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x));
}

static float Cross(Vector2 a, Vector2 b) {
	return (a.x * b.y - b.x * a.y);;
}

static float SquareLength(Vector2 v) {
	return v.x * v.x + v.y * v.y;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void Shape::RenderShape(Transform transform, PhysicsProgram& program, Vector3 colour)
{
	RenderShape(program, transform, colour);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// POLYGON
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

PolygonShape::PolygonShape(Vector2* vertices, int vertexCount)
{
	//(this function organises vertices, calculates concave hull and centerpoint)
	OrganisePoints(vertices, vertexCount);
	
	CalculateNormals();
}

bool PolygonShape::PointCast(Vector2 point, Transform& transform)
{

	return false;
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
		float triArea = 0.5f * Cross(a, b);
		area += triArea;

		//fun fact: the squared length of a vector is the same as the dot product of it with itself
		in += (SquareLength(a) + SquareLength(b) + glm::dot(a, b)) * triArea / 6;
		//triangle area * triangle centre
		polyCentre += triArea * triCentre;
	}
	in = density * in;

	//set final values
	centrePoint = polyCentre / (area); //divide by total area to get the centrepoint
	mass = area * density; //multiply by density
	inertia = in - mass * SquareLength(polyCentre);//(translate mass moment of inertia to be relative to centrepoint)
}

void PolygonShape::RenderShape(PhysicsProgram& program, Transform& transform, Vector3 colour)
{
	auto& lines = program.GetLineRenderer();
	for (size_t i = 0; i < pointCount; i++)
	{
		lines.AddPointToLine(transform.TransformPoint(points[i]), colour);
	}
	lines.FinishLineLoop();
}

AABB PolygonShape::CalculateAABB(Transform& transform)
{
	float xMax = points[0].x;
	float xMin = points[0].x;

	float yMax = points[0].y;
	float yMin = points[0].y;

	for (size_t i = 1; i < pointCount; i++)
	{
		xMax = std::max(points[i].x, xMax);
		yMax = std::max(points[i].y, yMax);
		xMin = std::min(points[i].x, xMin);
		yMin = std::min(points[i].y, yMin);
	}

	return { Vector2(xMax, yMax), Vector2(xMin, yMin) };
}

SHAPE_TYPE PolygonShape::GetType()
{
	return SHAPE_TYPE::POLYGON;
}

Shape* PolygonShape::Clone()
{
	return new PolygonShape(*this);
}

PolygonShape* PolygonShape::GetRegularPolygonCollider(float radius, int pointCount)
{

	if (pointCount > max_vertices) {

		std::cout << "Error: Polygons have a max vertex count of " << max_vertices;
		pointCount = max_vertices;
	}

	Vector2* points = new Vector2[pointCount];

	float iPointCount = glm::two_pi<float>() / pointCount;
	for (size_t i = 0; i < pointCount; i++)
	{
		points[i] = Vector2(radius * cos((i + 0.5f) * iPointCount), radius * sin((i + 0.5f) * iPointCount));
	}

	return new PolygonShape(points, pointCount);
	delete[] points;
}

bool PolygonShape::OrganisePoints(Vector2* points, int pointCount)
{
	//first create convex hull using gift wrapping algorithm https://en.wikipedia.org/wiki/Gift_wrapping_algorithm
	//this should both discard all points that make the shape concave, and organise the points in a counterclockwise manner
	
	//the minimum and maximum on any axis is guaranteed to be on the convex hull, so find that for the first point

	int startPoint = 0;
	float minX = points[0].x;
	for (size_t i = 1; i < pointCount; i++)
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
		this->points[size] = points[lastPointOnHull];
		endPoint = 0;
		for (size_t i = 0; i < pointCount; i++)
		{
			float cross = Cross(points[lastPointOnHull], points[endPoint], points[i]);

			// if:
			// the endpoint is the last hull point
			// point i is further to the right than the last point
			// point i is the same amount to the right as the last, but is longer
			//
			// set the new endpoint to point i;
			if (endPoint == lastPointOnHull || cross < 0
				|| (cross == 0 && (SquareLength(points[lastPointOnHull] - points[i]) > SquareLength(points[lastPointOnHull] - points[endPoint]))))
			{
				endPoint = i;
			}
		}
		size++;
		lastPointOnHull = endPoint;
	} while (endPoint != startPoint && size <= max_vertices);
	//now set correct pointCount
	this->pointCount = size;

	//returns whether point count changed or not
	return size != pointCount;
}

//WINDING ORDER: COUNTER CLOCKWISE
void PolygonShape::CalculateNormals()
{
	//calculate normals, assuming counter clockwise order
	for (size_t i = 0; i < pointCount; i++)
	{
		int j = (i + 1) % pointCount;
		Vector2 delta = glm::normalize(points[i] - points[j]);
		delta = Vector2(-delta.y, delta.x);
		normals[i] = delta;
	}
}

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


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// CIRCLE
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

CircleShape::CircleShape(float radius, Vector2 centrePoint)
{
	this->radius = radius;
	this->centrePoint = centrePoint;
}

bool CircleShape::PointCast(Vector2 point, Transform& transform)
{
	Vector2 centre = transform.TransformPoint(centrePoint);
	return SquareLength(centre - point) < radius * radius;
}

void CircleShape::CalculateMass(float& mass, float& inertia, float density)
{
	mass = density * radius * radius * glm::pi<float>();
	//circle is a cylinder with thickness of one, meaning the mass moment of inertia is mr^2/2
	inertia = mass * radius * radius * 0.5f; 
	//im a bit confused on how translating inertia tensor stuff works but I think this is correct
	inertia += mass * SquareLength(centrePoint);
}

void CircleShape::RenderShape(PhysicsProgram& program, Transform& transform, Vector3 colour)
{
	program.GetLineRenderer().DrawCircle(transform.TransformPoint(centrePoint), radius, colour);
	program.GetLineRenderer().DrawLineSegment(transform.TransformPoint(Vector2(0, radius)), transform.TransformPoint(Vector2(0, radius * 0.5f)), colour);
}

AABB CircleShape::CalculateAABB(Transform& transform)
{
	AABB aABB;
	Vector2 globalCentrePoint = transform.TransformPoint(centrePoint);
	aABB.max.x = globalCentrePoint.x + radius;
	aABB.max.y = globalCentrePoint.y + radius;
	aABB.min.x = globalCentrePoint.x - radius;
	aABB.min.y = globalCentrePoint.y - radius;
	return aABB;
}

SHAPE_TYPE CircleShape::GetType()
{
	return SHAPE_TYPE::CIRCLE;
}

Shape* CircleShape::Clone()
{
	return new CircleShape(*this);
}

CapsuleShape::CapsuleShape(Vector2 a, Vector2 b, float radius)
{
	pointA = a;
	pointB = b;
	this->radius = radius;
}

bool CapsuleShape::PointCast(Vector2 point, Transform& transform)
{
	//capsule is basically 2 circles and a rect

	//inverse transform point instead of transforming the points, one less calculation
	point = transform.InverseTransformPoint(point);

	//if point is inside the two circles, it is colliding
	if (SquareLength(pointA - point) > radius * radius || SquareLength(pointB - point) > radius * radius)
		return true;

	//if the point is inside the oriented bounding box, it is colliding
	Vector2 tangent = glm::normalize(pointA - pointB);
	Vector2 normal = { -tangent.y, tangent.x };
	
	//y axis, with 0 being the min of the OBB
	float distanceFromLine = glm::dot(normal, point) - glm::dot(normal, pointA);
	//x axis, 0 being the min of the OBB
	float distanceAlongLine = glm::dot(tangent, point) - glm::dot(tangent, pointA);
	
	if (glm::abs(distanceFromLine) < radius
		&& distanceAlongLine > 0 && distanceAlongLine < glm::dot(tangent, pointB))
	{
		return true;
	}
	else return false;
}

void CapsuleShape::CalculateMass(float& mass, float& inertia, float density)
{
	float len = glm::length(pointA - pointB);
	mass = density * (2 * radius * len + radius * radius * glm::pi<float>());
	inertia = 0; 

	//inertia is equal to 2 translated semicircles + inertia of rectangle
	float semiCircleMass = 0.5f * density * radius * radius * glm::pi<float>();
	float semiCircleInertia = semiCircleMass * radius * radius * 0.5f;
	//translate inertia by adding mr^2 (proof in notebook)
	//the semi circle is currently rotating around what would be it's centre if it was a full circle
	//so need to move it by half of length of the capsule's rectangle
	semiCircleInertia += semiCircleMass * 0.25 * len * len;
	//now get the rect inertia
	float width = radius * 2;
	float rectMass = len * width * density;
	float rectInertia = rectMass * (width * width + len * len) / 12.0f;
	//rect inertia is already about the right axis (relative to the semicircles.
	//now add all inertias together 
	inertia = rectInertia + semiCircleInertia * 2;
	//now translate it by the distance between the centrepoint of the capsule and the centrepoint of the physicsObject (0,0), so it is correct when rotating about that axis
	float dist = glm::length(0.5f * (pointA + pointB));
	inertia += mass * dist * dist;

	//success! (probably, I have no idea how to check)


}

void CapsuleShape::RenderShape(PhysicsProgram& program, Transform& transform, Vector3 colour)
{
	//if the buffer is big enough, this is a capsule, otherwise treat it as a line
	if (radius > 0.01f)
	{
		Vector2 a = transform.TransformPoint(pointA);
		Vector2 b = transform.TransformPoint(pointB);
		auto& lR = program.GetLineRenderer();
		lR.DrawCircle(a, radius, colour, 8);
		lR.DrawCircle(b, radius, colour, 8);

		Vector2 radiusAddition = glm::normalize(a - b);
		radiusAddition = { radiusAddition.y, -radiusAddition.x };

		lR.DrawLineSegment(a + radiusAddition, b + radiusAddition, colour);
		lR.DrawLineSegment(a - radiusAddition, b - radiusAddition, colour);
	}
	else {
		LineRenderer& lR = program.GetLineRenderer();
		lR.DrawLineSegment(transform.TransformPoint(pointA),
			transform.TransformPoint(pointB), colour);
	}
	
}

AABB CapsuleShape::CalculateAABB(Transform& transform)
{
	AABB aABB;
	Vector2 pA = transform.TransformPoint(pointA), pB = transform.TransformPoint(pointB);

	if (pA.x > pB.x) {
		aABB.max.x = pA.x + radius;
		aABB.min.x = pB.x - radius;
	}
	else 
	{
		aABB.max.x = pB.x + radius;
		aABB.min.x = pA.x - radius;
	}

	if (pA.y > pB.y) {
		aABB.max.y = pA.y + radius;
		aABB.min.y = pB.y - radius;
	}
	else
	{
		aABB.max.y = pB.y + radius;
		aABB.min.y = pA.y - radius;
	}
	return aABB;
}

SHAPE_TYPE CapsuleShape::GetType()
{
	return SHAPE_TYPE::CAPSULE;
}

Shape* CapsuleShape::Clone()
{
	return new CapsuleShape(*this);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Plane
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

PlaneShape::PlaneShape(Vector2 normal, float d) : normal(normal), distance(d)
{}

PlaneShape::PlaneShape(Vector2 normal, Vector2 startPosition)
{
	this->normal = glm::normalize(normal);
	distance = glm::dot(normal, startPosition);
}

PlaneShape::PlaneShape(Vector2 pointA, Vector2 pointB, void* null)
{
	Vector2 normal = glm::normalize(pointA - pointB);
	this->normal = Vector2(-normal.y, normal.x);
	distance = glm::dot(normal, pointB);
}

bool PlaneShape::PointCast(Vector2 point, Transform& transform)
{
	return glm::dot(transform.InverseTransformPoint(point), normal) < distance;
}

void PlaneShape::CalculateMass(float& mass, float& inertia, float density)
{
	mass = 0;
	inertia = 0;
}

void PlaneShape::RenderShape(PhysicsProgram& program, Transform& transform, Vector3 colour)
{
	auto& lR = program.GetLineRenderer();
	
	Vector2 tNormal = transform.TransformDirection(normal);

	Vector2 tPlanePoint = transform.TransformPoint(normal*distance);
	float tDist = glm::dot(normal, tPlanePoint);

	Vector2 tangent = 1000.0f * Vector2{ tNormal.y, -tNormal.x };
	lR.DrawLineSegment(tPlanePoint + tangent, tPlanePoint - tangent, colour);
}

AABB PlaneShape::CalculateAABB(Transform& transform)
{
	return AABB{ Vector2{INFINITY, INFINITY}, Vector2{-INFINITY, -INFINITY} };
}

SHAPE_TYPE PlaneShape::GetType()
{
	return SHAPE_TYPE::PLANE;
}

Shape* PlaneShape::Clone()
{
	return new PlaneShape(*this);
}
