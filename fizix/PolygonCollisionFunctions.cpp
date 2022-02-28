#include "fzx.h"

namespace fzx
{
	constexpr float DISTANCE_TOLERANCE = 0.0001f;
	constexpr float CLIP_TOLERANCE = 0.001f;
	constexpr int MAXMINKOWSKIPOINTS = 50;

	Vector2 PhysicsSystem::GetPerpendicularTowardOrigin(Vector2 a, Vector2 b)
	{
		//triple cross method in 2d, returns a vector perpendicular to line AB, orientated towards a point
		// ((b-a) x ((0,0)-a)) x (b-a)

		Vector2 delta = b - a;
		float cross = em::Cross(delta, -a);
		return glm::normalize(Vector2(-cross * delta.y, cross * delta.x));

		//triple cross product can be converted into dot product. kinda cool
		//(a x b) x c == b * (c.a) - a * (c.b)
	}

	Vector2 PhysicsSystem::GetPerpendicularFacingInDirection(Vector2 line, Vector2 direction)
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
		return tripleCross / len;
	}


	Vector2 PhysicsSystem::GetSupport(Shape* a, Shape* b, Transform& tA, Transform& tB, Vector2 d)
	{
		return a->Support(d, tA) - b->Support(-d, tB);
	}


	Vector2 PhysicsSystem::ClosestPointToOrigin(Vector2 a, Vector2 b)
	{
		return em::SquareLength(a) < em::SquareLength(b) ? a : b;
	}

	// GJK function
	//the final version of this function is based on this video: https://youtu.be/ajv46BSqcK4
	bool PhysicsSystem::GJK(Shape* a, Shape* b, Transform& tA, Transform& tB, Simplex* finalSimplex)
	{
		//GJK checks if the minkowski difference of two shapes encloses the origin or not.
		//if the minkowski difference does enclose the origin it means the two shapes are intersecting, because it means at least one point in the area inclosed by shape a is in the same position as a point in the area inclosed by shape b.
		//this algorythm is a very interesting abstraction of that idea that breaks down the enclosing the origin check into a small number of triangle enclosing the origin checks.
		//the triangles chosen are composed of 3 points on the minkowski difference, and those three points are chosen intelligently to minimise the tri checks
		Simplex tri;
		//first direction can be anything, but is often the direction from shape a to b (it is probably more efficient on average then a random direction, idk)
		tri.dir = glm::normalize(tB.position - tA.position);

		//get furthest point on the minkowski difference in the direction of tri.dir
		tri.a = GetSupport(a, b, tA, tB, tri.dir);
		//the best next direction to choose is towards the origin
		tri.dir = glm::normalize(-tri.a);
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

	//the final version of this function is based on this video: https://www.youtube.com/watch?v=0XQ2FSz3EK8 and this page: https://dyn4j.org/2010/04/gjk-distance-closest-points/

	bool PhysicsSystem::EPA(Shape* a, Shape* b, Transform& tA, Transform& tB, EPACollisionData* data)
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
		Vector2 edgeNormal = Vector2(0, 0);
		float dist;
		int index;

		while (true)
		{
			//find the closest edge to the centrepoint on the current polytope
			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			dist = INFINITY;

			int i = (int)polytope.size() - 1;
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

			if ((dist == INFINITY))
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

					data->collisionNormal = isnan(edgeNormal.x) ? Vector2(1, 0) : edgeNormal;
					std::cout << "went over maximum\n";
					return true;
				}
			}
		}
	}


	//returns index 

	PhysicsSystem::PolygonEdge PhysicsSystem::FindPolygonCollisionEdge(PolygonShape* pS, Transform& t, Vector2 normal)
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


	//clips 2 points so that they are more than or equal to clip distance along the clipping normal
	PhysicsSystem::ClipInfo PhysicsSystem::Clip(Vector2 pointToClip1, Vector2 pointToClip2, Vector2 clippingNormal, float clipDist)
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
}