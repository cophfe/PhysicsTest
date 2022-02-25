#include "Collision.h"
#include "ExtraMath.hpp"
#include "CollisionManager.h"
#include <iostream>

#pragma region Circle
bool CollisionManager::CollideCircleCircle(CollisionData& data)
{
	CircleShape* a = (CircleShape*)data.a->GetCollider(data.colliderIndexA).GetShape();
	CircleShape* b = (CircleShape*)data.b->GetCollider(data.colliderIndexB).GetShape();

	Vector2 pA = data.a->GetTransform().TransformPoint(a->centrePoint), pB = data.b->GetTransform().TransformPoint(b->centrePoint);
	Vector2 delta = pA - pB;
	float deltaMagSq = delta.x * delta.x + delta.y * delta.y;
	float radiusSum = (a->radius + b->radius);

	if (deltaMagSq < radiusSum * radiusSum)
	{
		deltaMagSq = sqrtf(deltaMagSq);
		data.penetration = radiusSum - deltaMagSq;
		data.collisionNormal = delta / deltaMagSq;
		data.collisionPoints[0] = pA - data.collisionNormal * a->radius;
		return true;
	}

	return false;
}

bool CollisionManager::CollideCircleCapsule(CollisionData& data)
{
	CircleShape* a = (CircleShape*)data.a->GetCollider(data.colliderIndexA).GetShape();
	CapsuleShape* b = (CapsuleShape*)data.b->GetCollider(data.colliderIndexB).GetShape();

	//transform to global
	Vector2 pA = data.b->GetTransform().TransformPoint(b->pointA), pB = data.b->GetTransform().TransformPoint(b->pointB);
	Vector2 circleCentre = data.a->GetTransform().TransformPoint(a->centrePoint);

	//circle radius + capsule radius
	float radius = a->radius + b->radius;

	//this basically simplifies the problem to a circle-circle collision
	Vector2 pointOnCapsuleLine = em::ClosestPointOnLine(pA, pB, circleCentre);

	Vector2 delta = circleCentre - pointOnCapsuleLine;
	if (delta.x * delta.x + delta.y * delta.y < radius * radius)
	{
		//is colliding
		data.collisionNormal = glm::normalize(delta); //collision normal is always from b to a
		data.penetration = radius - glm::dot(delta, data.collisionNormal); //p = a->radius + b->radius - distancebetweencentreandclosestpointonLine
		data.collisionPoints[0] = pointOnCapsuleLine + data.collisionNormal * b->radius;
		return true;
	}

	return false;
}

bool CollisionManager::CollideCirclePlane(CollisionData& data)
{
	CircleShape* a = (CircleShape*)data.a->GetCollider(data.colliderIndexA).GetShape();
	PlaneShape* b = (PlaneShape*)data.b->GetCollider(data.colliderIndexB).GetShape();

	//transform to global
	Vector2 centre = data.a->GetTransform().TransformPoint(a->centrePoint);
	Vector2 planeDirection = data.b->GetTransform().TransformDirection(b->normal);
	float planeDistance = glm::dot(data.b->GetTransform().TransformPoint(b->distance * b->normal), planeDirection);

	//calculate penetration
	float centreDot = glm::dot(centre, planeDirection);
	float penetration = centreDot - a->radius - planeDistance;

	//if p < 0, is colliding
	if (penetration < 0)
	{
		data.collisionNormal = planeDirection;
		data.penetration = -penetration;
		data.collisionPoints[0] = centre - planeDirection * (a->radius);
		return true;
	}

	return false;
}
#pragma endregion

#pragma region Polygon

#pragma region GJK + EPA + Clipping Stuff
constexpr float DISTANCE_TOLERANCE = 0.0001f;
constexpr float CLIP_TOLERANCE = 0.001f;
constexpr int MAXMINKOWSKIPOINTS = 50;

struct Simplex
{
	Vector2 a, b, c;
	Vector2 dir;
};

Vector2 GetPerpendicularTowardOrigin(Vector2 a, Vector2 b)
{
	//triple cross method in 2d, returns a vector perpendicular to line AB, orientated towards a point
	// ((b-a) x ((0,0)-a)) x (b-a)

	Vector2 delta = b - a;
	float cross = em::Cross(delta, -a);
	return glm::normalize(Vector2(-cross * delta.y, cross * delta.x));
	
	//triple cross product can be converted into dot product. kinda cool
	//(a x b) x c == b * (c.a) - a * (c.b)
}

Vector2 GetPerpendicularFacingInDirection(Vector2 line, Vector2 direction)
{
	//is perpendicular to line, facing in the direction of direction
	// (direction x line) x line

	float cross = em::Cross(direction, line);

	Vector2 tripleCross = Vector2(-cross * line.y, cross * line.x);
	float len = glm::length(tripleCross);
	
	//this happens when line and direction are exactly parallel and when direction or line are 0,0
	if (len == 0)
	{
		//std::cout << "this is suspicious\n";
		//this should work if direction is 0,0 AND if line and direction are exactly parallel
		return glm::normalize(GetPerpendicularFacingInDirection(line + Vector2(0.0001f, 0.0f), direction + Vector2(0.0f, 0.0001f))); //em::normalize(em::GetPerpendicularCounterClockwise(line + Vector2(0.001f, 0)));
	}
	return tripleCross/len;
}


static inline Vector2 GetSupport(Shape* a, Shape* b, Transform& tA, Transform& tB, Vector2 d)
{
	return a->Support(d, tA) - b->Support(-d, tB);
}


static inline Vector2 ClosestPointToOrigin(Vector2 a, Vector2 b)
{
	return em::SquareLength(a) < em::SquareLength(b) ? a : b;
}

// GJK function
//the final version of this function is based on this video: https://youtu.be/ajv46BSqcK4
static bool GJK(Shape * a, Shape * b, Transform & tA, Transform & tB, Simplex * finalSimplex)
{
	//GJK checks if the minkowski difference of two shapes encloses the origin or not.
	//if the minkowski difference does enclose the origin it means the two shapes are intersecting, because it means at least one point in the area inclosed by shape a is in the same position as a point in the area inclosed by shape b.
	//this algorythm is a very interesting abstraction of that idea that breaks down the enclosing the origin check into a small number of triangle enclosing the origin checks.
	//the triangles chosen are composed of 3 points on the minkowski difference, and those three points are chosen intelligently to minimise the tri checks
	Simplex tri;
	//first direction can be anything, but is often the direction from shape a to b (it is probably more efficient on average then a random direction, idk)
	tri.dir = em::normalize(tB.position - tA.position);

	//get furthest point on the minkowski difference in the direction of tri.dir
	tri.a = GetSupport(a, b, tA, tB, tri.dir);
	//the best next direction to choose is towards the origin
	tri.dir = em::normalize(-tri.a);
	//get furthest point in the direction of the origin from point a
	tri.b = GetSupport(a, b, tA, tB, tri.dir);

	//line case
	//if point b is not on the opposite side of the origin from point a, the minkowski difference does not enclose the origin and the shapes aren't colliding
	if (glm::dot(tri.b, tri.dir) < 0)
		return false;

	//the new support point direction will be perpendicular to point a and b, towards the origin. 
	//This is because all the points on the other side of the line AB are moving away from the origin 
	tri.dir = GetPerpendicularTowardOrigin(tri.a, tri.b);

	while (true)
	{
		//at this point in the loop, tri.c is always undefined.
		tri.c = GetSupport(a, b, tA, tB, tri.dir);

		if (glm::dot(tri.c, tri.dir) < 0)
			return false;

		Vector2 lineCA = tri.a - tri.c;
		Vector2 lineCB = tri.b - tri.c;

		//check if inside the triangle defined by a,b,c
		//this method is based on voronoi regions and uses our previous knowledge about the shape to simplify the calculations
		//check if point is in voronoi region defined by line CB
		Vector2 v = GetPerpendicularFacingInDirection(lineCB, lineCA);
		if (glm::dot(v, tri.c) < 0)
		{
			//in this case, origin is in the region in the direction v from the triangle we created.
			//set the tri.dir vector to this direction, and remove point a from the triangle
			tri.dir = v;
			tri.a = tri.b;
			tri.b = tri.c;
			continue;
		}
		//check if point is in voronoi region defined by line CA
		v = GetPerpendicularFacingInDirection(lineCA, lineCB);
		if (glm::dot(v, tri.c) < 0)
		{
			//in this case, origin is in the region in the direction v from the triangle we created.
			//set the tri.dir vector to this direction, and remove point b from the triangle
			tri.dir = v;
			tri.b = tri.c;
			continue;
		}
		//if not in those regions, the triangle MUST contain the origin, so it it colliding
		if (finalSimplex != nullptr)
			*finalSimplex = tri; //return final simplex for EPA

		return true;
	}
}

struct EPACollisionData
{
	float depth;
	Vector2 collisionNormal;
};

//the final version of this function is based on this video: https://www.youtube.com/watch?v=0XQ2FSz3EK8 and this page: https://dyn4j.org/2010/04/gjk-distance-closest-points/
static bool EPA(Shape* a, Shape* b, Transform& tA, Transform& tB, EPACollisionData* data)
{
	Simplex gjkSimplex;
	if (!GJK(a, b, tA, tB, &gjkSimplex))
	{
		return false; //gjk returned false
	}
	
	//the expanding polytope algorythm intends to 
	// find the edge closest to the origin on the minkowski difference/sum/whatever
	// it does this by adding support points to the GJK simplex, approaching the origin, until a suitible edge has been found

	//the polytope array is an array of ordered support points on the minkowski difference/sum/whatever 
	std::vector<Vector2> polytope;
	polytope.push_back(gjkSimplex.a);
	polytope.push_back(gjkSimplex.b);
	polytope.push_back(gjkSimplex.c);
	
	//temp variables containing edge information
	float lastDepth = INFINITY;
	Vector2 edgeNormal = Vector2(0,0);
	float dist;
	int index;

	while (true)
	{
		//find the closest edge to the centrepoint on the current polytope
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		dist = INFINITY;

		int i = polytope.size() - 1;
		for (int j = 0; j < polytope.size(); j++)
		{

			Vector2 delta = polytope[j] - polytope[i];
			Vector2 norm = glm::normalize(em::TripleCross(delta, polytope[i], delta));

			float d = glm::dot(norm, polytope[i]);
			if (d < dist)
			{
				//distance
				dist = d;
				//the normal to the edge pointing out
				edgeNormal = norm;
				//the second point on the edge, also the index at which a support point found using edgeNormal would need to be inserted into the polytope array
				index = j;
			}
			i = j;
		}
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		if ((dist == INFINITY) )
		{
			//could not find anything. mostly happens only when all support points are on 0,0
			data->depth = 0.1f;
			data->collisionNormal = Vector2(0, 1);
			return true;
		}
		lastDepth = dist;

		Vector2 support = GetSupport(a, b, tA, tB, edgeNormal);

		float depth = glm::dot(support, edgeNormal);
		if (depth - dist < DISTANCE_TOLERANCE)
		{
			//we have found the edge nearest the origin (or something close to it)
			//using that we can return collision info.
			data->collisionNormal = -edgeNormal; //negate so that it is from b to a
			data->depth = depth;
			return true;
		}
		else
		{
			//otherwise we haven't found the closest edge
			//add the support point into the polytope vector, in between the points that created the edge
			polytope.insert(polytope.begin() + index, support);

			if (polytope.size() > MAXMINKOWSKIPOINTS)
			{
				data->depth = 0.1f;
				
				data->collisionNormal = isnan(edgeNormal.x) ? Vector2(1,0) : edgeNormal;
				std::cout << "went over maximum\n";
				return true;
			}
		}
	}
}

struct PolygonEdge {
	Vector2 pA,
		pB;

	Vector2 maxProjectionVertex;
};
//returns index 
PolygonEdge FindPolygonCollisionEdge(PolygonShape* pS, Transform& t, Vector2 normal)
{
	//Get the point furthest along the collision normal
	Vector2 collisionNormal = t.InverseTransformDirection(normal);
	int pointIndex = 0;
	float d = glm::dot(collisionNormal, pS->points[0]);
	float d1;

	for (char i = 1; i < pS->pointCount; i++)
	{
		d1 = glm::dot(collisionNormal, pS->points[i]);
		if (d1 > d)
		{
			pointIndex = i;
			d = d1;
		}
	}

	//now see which of the two edges connected to this vertex are most perpendicular to the normal (aka the one with the dot product closest to zero
	Vector2 pointBack = pS->points[pointIndex == 0 ? pS->pointCount - 1 : pointIndex - 1];
	Vector2 pointFront = pS->points[pointIndex + 1 == pS->pointCount ? 0 : pointIndex + 1];

	Vector2 backLine = glm::normalize(pS->points[pointIndex] - pointBack);
	Vector2 frontLine = glm::normalize(pS->points[pointIndex] - pointFront);

	Vector2 transformedMaxVert = t.TransformPoint(pS->points[pointIndex]);

	if (glm::dot(backLine, collisionNormal) <= glm::dot(frontLine, collisionNormal))
	{
		return { t.TransformPoint(pointBack), transformedMaxVert , transformedMaxVert };
	}
	else
		return { transformedMaxVert, t.TransformPoint(pointFront),transformedMaxVert };
}

struct ClipInfo
{
	Vector2 points[2];
	int pointCount;
};
//clips 2 points so that they are more than or equal to clip distance along the clipping normal
ClipInfo Clip(Vector2 pointToClip1, Vector2 pointToClip2, Vector2 clippingNormal, float clipDist)
{
	ClipInfo c;
	c.pointCount = 0;

	//along normal relative to clipDist
	float point1AlongNormal = glm::dot(clippingNormal, pointToClip1) - clipDist;
	float point2AlongNormal = glm::dot(clippingNormal, pointToClip2) - clipDist;

	//the point is more than
	if (point1AlongNormal >= 0)
	{
		c.points[0] = pointToClip1;
		c.pointCount++;
	}
	if (point2AlongNormal >= 0)
	{
		c.points[c.pointCount] = pointToClip2;
		c.pointCount++;
	}

	//if point1 is on the left and p2 is on the right of clip dist
	if (point1AlongNormal * point2AlongNormal < 0)
	{
		Vector2 line = pointToClip2 - pointToClip1;
		//percentage along the line (uncapped, so it's not between 0 and 1)
		float t = point1AlongNormal / (point1AlongNormal - point2AlongNormal);
		Vector2 point = line * t + pointToClip1;
		c.points[c.pointCount] = point; //<-- ignore warning, this will not overrun
		c.pointCount++;
	}

	return c;
}
#pragma endregion

bool CollisionManager::CollideCirclePolygon(CollisionData& data)
{
	CircleShape* a = (CircleShape*)data.a->GetCollider(data.colliderIndexA).GetShape();
	PolygonShape* b = (PolygonShape*)data.b->GetCollider(data.colliderIndexB).GetShape();

	EPACollisionData epaData;
	if (EPA(a, b, data.a->transform, data.b->transform, &epaData))
	{
		data.collisionNormal = epaData.collisionNormal;
		data.penetration = epaData.depth;
		data.collisionPoints[0] = data.a->transform.TransformPoint(a->centrePoint) - a->radius * epaData.collisionNormal;
		return true;
	}

	return false;
}



bool CollisionManager::CollidePolygonPolygon(CollisionData& data)
{
	PolygonShape* a = (PolygonShape*)data.a->GetCollider(data.colliderIndexA).GetShape();
	PolygonShape* b = (PolygonShape*)data.b->GetCollider(data.colliderIndexB).GetShape();

	/*std::cout << "Do polygons collide: " << Intersection(a, b, data.a->transform, data.b->transform, nullptr)
		<< std::endl;*/

	EPACollisionData epaData;
	if (EPA(a, b, data.a->transform, data.b->transform, &epaData))
	{
		//now find the collision points using the clipping method.
		PolygonEdge reference = FindPolygonCollisionEdge(a, data.a->transform, -epaData.collisionNormal); 
		PolygonEdge incident = FindPolygonCollisionEdge(b, data.b->transform, epaData.collisionNormal);
		
		//reference edge: this edge clips the incident edge to get the contact points

		//the edge most perpendicular to the normal (the one with the dot product closest to zero) is the reference edge, the other is the incident edge
		bool flipEdges = em::Sq(glm::dot(reference.pB - reference.pA, epaData.collisionNormal)) > em::Sq(glm::dot(incident.pB - incident.pA, epaData.collisionNormal));
		if (flipEdges)
		{
			PolygonEdge temp = reference;
			reference = incident;
			incident = temp;
		}

		Vector2 referenceTangent = glm::normalize(reference.pB - reference.pA);

		float referenceStart = glm::dot(reference.pA, referenceTangent);
		ClipInfo c;
		/*c.pointCount = 2;
		c.points[0] = incident.pA;
		c.points[1] = incident.pB;*/
		c = Clip(incident.pA, incident.pB, referenceTangent, referenceStart);
		if (c.pointCount < 2) return false;
		
		float referenceEnd = glm::dot(reference.pB, referenceTangent);
		c = Clip(c.points[0], c.points[1], -referenceTangent, -referenceEnd);
		if (c.pointCount < 2) return false;

		//delete vertices that are above the edge
		Vector2 referenceNormal = em::GetPerpendicularClockwise(referenceTangent);
		//if (flipEdges)
		//	referenceNormal = -referenceNormal;

		float max = glm::dot(reference.maxProjectionVertex, referenceNormal);
		if (glm::dot(c.points[0], referenceNormal) > max)
		{
			if (c.pointCount > 1)
				c.points[0] = c.points[1];
			c.pointCount--;
		}
		if (glm::dot(c.points[1], referenceNormal) > max)
		{
			c.pointCount--;
		}
		if (c.pointCount < 1) return false;

		data.collisionNormal = epaData.collisionNormal;
		data.penetration = epaData.depth;
		data.collisionPoints[0] = c.points[0];
		data.collisionPoints[1] = c.points[1];
		data.pointCount = c.pointCount;


		return true;
	}

	return false;
}

bool CollisionManager::CollidePolygonCapsule(CollisionData& data)
{
	PolygonShape* a = (PolygonShape*)data.a->GetCollider(data.colliderIndexA).GetShape();
	CapsuleShape* b = (CapsuleShape*)data.b->GetCollider(data.colliderIndexB).GetShape();

	EPACollisionData epaData;
	if (EPA(a, b, data.a->transform, data.b->transform, &epaData))
	{
		data.collisionNormal = epaData.collisionNormal;
		data.penetration = epaData.depth;
		
		//data.collisionPoints[0] = em::ClosestPointOnLine(data.b->transform.TransformPoint(b->pointA), data.b->transform.TransformPoint(b->pointB),
		//	data.a->transform.TransformPoint(a->centrePoint)) - b->radius * data.collisionNormal; //<-- once again, this works really well for something that is not accurate.
		
		//Now find collision point by comparing 
		PolygonEdge aEdge = FindPolygonCollisionEdge(a, data.a->transform, -data.collisionNormal);
		Vector2 bPointA = data.b->GetTransform().TransformPoint(b->pointA),
			bPointB = data.b->GetTransform().TransformPoint(b->pointB);

		Vector2 intersectionPoint;
		/*if (em::CalculateIntersectionPoint(aEdge.pA, aEdge.pB, bPointA, bPointB, intersectionPoint))
		{
			//in this case, when both polygon edge points are on the opposite side of the capsule centre line than the normal direction,
			//it probably returns the wrong value. but idk it's hard to say
		}*/
		//((b-a) x ((0,0)-a)) x (b-a)
		Vector2 bNormal = em::GetPerpendicularCounterClockwise(bPointB - bPointA);//, data.a->transform.position);
		bNormal = GetPerpendicularFacingInDirection(bPointB - bPointA, -data.collisionNormal);
		float stadiumDistance = glm::dot(bNormal, bPointA);

		float pA = glm::dot(aEdge.pA, bNormal);
		float pB = glm::dot(aEdge.pB, bNormal);
		if (pA - stadiumDistance < pB - stadiumDistance )
		{
			data.collisionPoints[0] = em::ClosestPointOnLine(bPointA, bPointB, aEdge.pA) + b->radius * data.collisionNormal;
		}
		else
			data.collisionPoints[0] = em::ClosestPointOnLine(bPointA, bPointB, aEdge.pB) + b->radius * data.collisionNormal;
		return true;
	}


	return false;
}

bool CollisionManager::CollidePolygonPlane(CollisionData& data) 
{
	PolygonShape* a = (PolygonShape*)data.a->GetCollider(data.colliderIndexA).GetShape();
	PlaneShape* b = (PlaneShape*)data.b->GetCollider(data.colliderIndexB).GetShape();
	
	Vector2 planeNormal = data.b->GetTransform().TransformPoint(b->normal);
	float planeDistance = glm::dot(data.b->GetTransform().TransformPoint(b->distance * b->normal), planeNormal);

	float minPenetration = 1;
	Vector2 collisionPoint;
	for (size_t i = 0; i < a->pointCount; i++)
	{
		Vector2 point = data.a->GetTransform().TransformPoint(a->points[i]);
		float p = glm::dot(point, planeNormal) - planeDistance;

		if (p < minPenetration)
		{
			collisionPoint = point;
			minPenetration = p;
		}
	}

	if (minPenetration < 0)
	{
		data.collisionNormal = planeNormal;
		data.penetration = -minPenetration;
		data.collisionPoints[0] = collisionPoint;

		data.pointCount = 0;
		//now find collision points
		//now find the collision points using the clipping method.
		PolygonEdge incident = FindPolygonCollisionEdge(a, data.a->transform, -planeNormal);
		//the reference edge is perpendicular to the plane normal

		//add vertices that aren't above the plane
		if ( glm::dot(incident.pA, planeNormal) <= planeDistance)
		{
			data.collisionPoints[0] = incident.pA;
			data.pointCount++;
		}
		if ( glm::dot(incident.pB, planeNormal) <= planeDistance)
		{
			data.collisionPoints[data.pointCount] = incident.pB;
			data.pointCount++;
		}
		if (data.pointCount < 1)
			return false;
		else
			return true;
	}
	return false;
}
#pragma endregion

#pragma region Capsule
bool CollisionManager::CollideCapsuleCapsule(CollisionData& data)
{
	CapsuleShape* a = (CapsuleShape*)data.a->GetCollider(data.colliderIndexA).GetShape();
	CapsuleShape* b = (CapsuleShape*)data.b->GetCollider(data.colliderIndexB).GetShape();

	Vector2 aPointA = data.a->GetTransform().TransformPoint(a->pointA),
		aPointB = data.a->GetTransform().TransformPoint(a->pointB)
		, bPointA = data.b->GetTransform().TransformPoint(b->pointA),
		bPointB = data.b->GetTransform().TransformPoint(b->pointB);;

	//stadium checking comes down to 4 point-line distance checks, an intersection test, and 4 point normal tests

	Vector2 intersection;
	if (em::CalculateIntersectionPoint(aPointA, aPointB, bPointA, bPointB, intersection))
	{
		Vector2 aTangent = glm::normalize(aPointB - aPointA);
		Vector2 bTangent = glm::normalize(bPointB - bPointA);

		Vector2 aNormal = { -aTangent.y, aTangent.x };
		Vector2 bNormal = { -bTangent.y, bTangent.x };

		//this is essentially 4 point-plane checks
		//the point with the smallest penetration wins, and the capsule will be pushed out along that axis
		Vector2 normal = aNormal;
		float aPoint = glm::dot(aNormal, aPointA);
		float bPoint = glm::dot(bNormal, bPointA);
		float p1 = aPoint - glm::dot(aNormal, bPointA);
		float p2 = aPoint - glm::dot(aNormal, bPointB);

		if (p1 * p1 > p2 * p2)
			p1 = p2;

		p2 = glm::dot(bNormal, aPointA) - bPoint;
		if (p1 * p1 > p2 * p2)
		{
			normal = bNormal;
			p1 = p2;
		}

		p2 = glm::dot(bNormal, aPointB) - bPoint;

		if (p1 * p1 > p2 * p2)
		{
			p1 = p2;
			normal = bNormal;
		}

		//since we know they are intersecting there is no need to perform a check here
		float sign = glm::sign(p1); //<-- could probably get rid of this somehow with the triple cross product thing
		data.collisionNormal = sign * -normal;
		data.penetration = sign * p1 + a->radius + b->radius;
		data.collisionPoints[0] = intersection + data.collisionNormal * a->radius;
		return true;
	}
	else
	{
		//this is essentially 4 different circle checks
		//we will use the one with the smallest distance between circle centres

		struct {
			Vector2 collisionDelta;
			float collisionDistanceSq;
		} collision, newCollision;

		float collisionMul = -a->radius;
		Vector2 collisionPoint = aPointA;
		collision.collisionDelta = aPointA - em::ClosestPointOnLine(bPointA, bPointB, aPointA);
		collision.collisionDistanceSq = em::SquareLength(collision.collisionDelta);

		newCollision.collisionDelta = aPointB - em::ClosestPointOnLine(bPointA, bPointB, aPointB);
		newCollision.collisionDistanceSq = em::SquareLength(newCollision.collisionDelta);
		if (collision.collisionDistanceSq > newCollision.collisionDistanceSq)
		{
			collision = newCollision;
			collisionPoint = aPointB;
		}

		newCollision.collisionDelta = em::ClosestPointOnLine(aPointA, aPointB, bPointA) - bPointA;
		newCollision.collisionDistanceSq = em::SquareLength(newCollision.collisionDelta);
		if (collision.collisionDistanceSq > newCollision.collisionDistanceSq)
		{
			collision = newCollision;
			collisionPoint = bPointA;
			collisionMul = b->radius;
		}

		newCollision.collisionDelta = em::ClosestPointOnLine(aPointA, aPointB, bPointB) - bPointB;
		newCollision.collisionDistanceSq = em::SquareLength(newCollision.collisionDelta);
		if (collision.collisionDistanceSq > newCollision.collisionDistanceSq)
		{
			collision = newCollision;
			collisionPoint = bPointB;
			collisionMul = b->radius;
		}

		float radius = a->radius + b->radius;

		//if the smallest collision distance is less than the sum of the radii, it is colliding
		if (collision.collisionDistanceSq < radius * radius)
		{
			float length = sqrtf(collision.collisionDistanceSq);
			data.penetration = radius - length;
			data.collisionNormal = collision.collisionDelta / length;
			data.collisionPoints[0] = collisionPoint + data.collisionNormal * collisionMul;
			return true;
		}
	}

	return false;
}

bool CollisionManager::CollideCapsulePlane(CollisionData& data)
{
	CapsuleShape* a = (CapsuleShape*)data.a->GetCollider(data.colliderIndexA).GetShape();
	PlaneShape* b = (PlaneShape*)data.b->GetCollider(data.colliderIndexB).GetShape();

	Vector2 pointA = data.a->GetTransform().TransformPoint(a->pointA), pointB = data.a->GetTransform().TransformPoint(a->pointB);
	Vector2 planeDirection = data.b->GetTransform().TransformDirection(b->normal);
	float planeDistance = glm::dot(data.b->GetTransform().TransformPoint(b->distance * b->normal), planeDirection);

	Vector2 collisionPoint = pointA;
	float start = glm::dot(pointA, planeDirection);
	float end = glm::dot(pointB, planeDirection);
	if (start > end)
	{
		float temp = start;
		start = end;
		end = temp;
		collisionPoint = pointB;
	}

	start -= a->radius;
	end += a->radius;

	//if colliding
	if (planeDistance >= start)
	{
		data.collisionNormal = planeDirection;
		data.penetration = planeDistance - start;
		data.collisionPoints[0] = collisionPoint - planeDirection * a->radius;
		return true;
	}

	return false;
}

bool CollisionManager::CollideInvalid(CollisionData& data)
{
	return false;
}
#pragma endregion

#pragma region Flipped Functions

bool CollisionManager::CollidePolygonCircle(CollisionData& data)
{
	auto temp = data.a;
	data.a = data.b;
	data.b = temp;
	return CollideCirclePolygon(data);
}

bool CollisionManager::CollideCapsuleCircle(CollisionData& data)
{
	auto temp = data.a;
	data.a = data.b;
	data.b = temp;
	return CollideCircleCapsule(data);
}

bool CollisionManager::CollidePlaneCircle(CollisionData& data)
{
	auto temp = data.a;
	data.a = data.b;
	data.b = temp;
	return CollideCirclePlane(data);
}

bool CollisionManager::CollideCapsulePolygon(CollisionData& data)
{
	auto temp = data.a;
	data.a = data.b;
	data.b = temp;
	return CollidePolygonCapsule(data);
}

bool CollisionManager::CollidePlanePolygon(CollisionData& data)
{
	auto temp = data.a;
	data.a = data.b;
	data.b = temp;
	return CollidePolygonPlane(data);
}

bool CollisionManager::CollidePlaneCapsule(CollisionData& data)
{
	auto temp = data.a;
	data.a = data.b;
	data.b = temp;
	return CollideCapsulePlane(data);
}
#pragma endregion
