#pragma once

#include "glmInclude.h"
#include <string>

//returns the index of the largest dimension http://www.pbr-book.org/3ed-2018/Geometry_and_Transformations/Vectors.html#Vector3::MaxDimension
static inline int maxDimension(const glm::vec3& v)
{
	return (v.x > v.y) ? ((v.x > v.z) ? 0 : 2) :
		((v.y > v.z) ? 1 : 2);
}

static inline std::string vec3Print(glm::vec3 v)
{
	return "x = " + std::to_string(v.x) + ", y = " + std::to_string(v.y) + ", z = " + std::to_string(v.z);
}

static inline int maxAbsDimension(const glm::vec3& vector)
{
	auto v = glm::abs(vector);
	return (v.x > v.y) ? ((v.x > v.z) ? 0 : 2) :
		((v.y > v.z) ? 1 : 2);
}

//permutes vector according to indices http://www.pbr-book.org/3ed-2018/Geometry_and_Transformations/Points.html#Point3::Permute
static inline glm::vec3 permute(const glm::vec3& p, int x, int y, int z)
{
	return glm::vec3(p[x], p[y], p[z]);
}

static inline float maxComponent(glm::vec3& v)
{
	return std::max(std::max(v.x, v.y), v.z);
}

static inline glm::vec3 computeNormal(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3)
{
	return glm::normalize(glm::cross(p2 - p1, p3 - p1));
}

static inline bool aabbPointIntersection(const glm::vec3& aabbMin, const glm::vec3& aabbMax, const  glm::vec3& point)
{
	return
		(aabbMin[0] <= point[0] && aabbMax[0] >= point[0]) &&
		(aabbMin[1] <= point[1] && aabbMax[1] >= point[1]) &&
		(aabbMin[2] <= point[2] && aabbMax[2] >= point[2]);
}

static inline bool aabbAabbIntersection(const glm::vec3& aMin, const glm::vec3& aMax, const glm::vec3& bMin, const glm::vec3& bMax)
{
	return
		(aMin[0] <= bMax[0] && aMax[0] >= bMin[0]) &&
		(aMin[1] <= bMax[1] && aMax[1] >= bMin[1]) &&
		(aMin[2] <= bMax[2] && aMax[2] >= bMin[2]);
}

static inline float calcTriangleSurfaceArea(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2)
{
	glm::vec3 a = v1 - v0;
	glm::vec3 b = v2 - v0;
	return 0.5f * glm::length(glm::cross(a, b));
}

static inline float calcSurfaceAreaPolygon(const std::vector<glm::vec3>& points, const std::vector<std::pair<int, int>>& lines, const glm::vec3& normal)
{
	//modified from http://geomalgorithms.com/a01-_area.html
	//major changes: usage of glm and its features, and different approach to point storage

	// Copyright 2000 softSurfer, 2012 Dan Sunday
	// This code may be freely used and modified for any purpose
	// providing that this copyright notice is included with it.
	// iSurfer.org makes no warranty for this code, and cannot be held
	// liable for any real or imagined damage resulting from its use.
	// Users of this code must verify correctness for their application.
	int dimensionToSkip = maxAbsDimension(normal);
	int dim0 = (dimensionToSkip + 1) % 3, dim1 = (dimensionToSkip + 2) % 3;

	//compute area of the 2d projection
	float area = 0;
	for (int i = 1; i < lines.size(); i++)
	{
		area += points[lines[i].first][dim0] * (points[lines[i].second][dim1] - points[lines[i - 1].first][dim1]);
	}
	//wrap arround term
	int s = lines.size();
	area += points[lines[0].first][dim0] * (points[lines[0].second][dim1] - points[lines[s - 1].first][dim1]);

	//scale area
	area *= 1 / (2 * normal[dimensionToSkip]);
	return area;
}

//clips line to a single axis: results are saved in p0 and p1. Smaller side: true if "smaller" side of is inside aabb
static inline bool clipLine(glm::vec3& p0, glm::vec3& p1, int& changedP, const glm::vec3& axisPosition, const int axis, const bool smallerSide)
{
	//changedP: -1 = unchanged 0 = p0, 1 = p1
	bool p0Inside = false, p1Inside = false;
	bool p0Boundary = false, p1Boundary = false;
	changedP = -1;
	if (smallerSide)
	{
		p0Inside = p0[axis] <= axisPosition[axis];
		p1Inside = p1[axis] <= axisPosition[axis];
	}
	else
	{
		//greater or greater equal doesnt matter since that is an extra test case.
		p0Inside = p0[axis] > axisPosition[axis];
		p1Inside = p1[axis] > axisPosition[axis];
	}
	//This is done to avoid cuts with factor 0 or 1
	p0Boundary = p0[axis] == axisPosition[axis];
	p1Boundary = p1[axis] == axisPosition[axis];

	if ((p0Inside && p1Inside) || (p0Inside && p1Boundary) || (p0Boundary && p1Inside) || (p0Boundary && p1Boundary))
	{
		return true;
	}
	else if ((!p0Inside && !p1Inside) || (!p0Inside && p1Boundary) || (p0Boundary && !p1Inside))
	{
		return false;
	}
	else
	{
		if (p0Inside)
		{
			//cut p1 so its at boundary
			float factor = (axisPosition[axis] - p0[axis]) / (p1[axis] - p0[axis]);
			p1 = p0 + (p1 - p0) * factor;
			changedP = 1;
		}
		else
		{
			//cut p0 so its at the boundary
			float factor = (axisPosition[axis] - p0[axis]) / (p1[axis] - p0[axis]);
			p0 = p0 + (p1 - p0) * factor;
			changedP = 0;
		}
	}
	return true;
}

/*
	Below is the unused line aabb clip. Kinda want to keep it for now

	//clips line to aabb. result points are stored in p0 and p1
	bool clipLineAabb(glm::vec3& p0, glm::vec3& p1, const glm::vec3& boundMin, const glm::vec3& boundMax)
	{
		//modified version of this algorithm https://en.wikipedia.org/wiki/Cohen%E2%80%93Sutherland_algorithm
		//major modification: works in 3d

		//some constants for line clipping
		const int INSIDE = 0; // 000000
		const int LEFT = 1;   // 000001
		const int RIGHT = 2;  // 000010
		const int BOTTOM = 4; // 000100
		const int TOP = 8;    // 001000
		const int FRONT = 16; // 010000
		const int BACK = 32;  // 100000

		// compute outcodes for P0, P1, and whatever point lies outside the clip rectangle
		int outcode0 = computeOutCode(p0, boundMin, boundMax);
		int outcode1 = computeOutCode(p1, boundMin, boundMax);
		bool accept = false;

		while (true) {
			if (!(outcode0 | outcode1)) {
				// bitwise OR is 0: both points inside window; trivially accept and exit loop
				accept = true;
				break;
			}
			else if (outcode0 & outcode1) {
				// bitwise AND is not 0: both points share an outside zone (LEFT, RIGHT, TOP,
				// or BOTTOM), so both must be outside window; exit loop (accept is false)
				break;
			}
			else {
				// failed both tests, so calculate the line segment to clip
				// from an outside point to an intersection with clip edge
				float x, y, z;

				// At least one endpoint is outside the clip rectangle; pick it.
				int outcodeOut = outcode0 ? outcode0 : outcode1;

				// Now find the intersection point;
				// use formulas:
				//   slope = (y1 - y0) / (x1 - x0)
				//   x = x0 + (1 / slope) * (ym - y0), where ym is ymin or ymax
				//   y = y0 + slope * (xm - x0), where xm is xmin or xmax
				// No need to worry about divide-by-zero because, in each case, the
				// outcode bit being tested guarantees the denominator is non-zero
				if (outcodeOut & TOP) {           // point is above the clip window
					float factor = (boundMax[1] - p0[1]) / (p1[1] - p0[1]);
					x = p0[0] + (p1[0] - p0[0]) * factor;
					y = boundMax[1];
					z = p0[2] + (p1[2] - p0[2]) * factor;
				}
				else if (outcodeOut & BOTTOM) { // point is below the clip window
					float factor = (boundMin[1] - p0[1]) / (p1[1] - p0[1]);
					x = p0[0] + (p1[0] - p0[0]) * factor;
					y = boundMin[1];
					z = p0[2] + (p1[2] - p0[2]) * factor;
				}
				else if (outcodeOut & RIGHT) {  // point is to the right of clip window
					float factor = (boundMax[0] - p0[0]) / (p1[0] - p0[0]);
					x = boundMax[0];
					y = p0[1] + (p1[1] - p0[1]) * factor;
					z = p0[2] + (p1[2] - p0[2]) * factor;
				}
				else if (outcodeOut & LEFT) {   // point is to the left of clip window
					float factor = (boundMin[0] - p0[0]) / (p1[0] - p0[0]);
					x = boundMin[0];
					y = p0[1] + (p1[1] - p0[1]) * factor;
					z = p0[2] + (p1[2] - p0[2]) * factor;
				}
				else if (outcodeOut & BACK) {  // point is to the right of clip window
					float factor = (boundMax[2] - p0[2]) / (p1[2] - p0[2]);
					x = p0[0] + (p1[0] - p0[0]) * factor;
					y = p0[1] + (p1[1] - p0[1]) * factor;
					z = boundMax[2];
				}
				else if (outcodeOut & FRONT) {   // point is to the left of clip window
					float factor = (boundMin[2] - p0[2]) / (p1[2] - p0[2]);
					x = p0[0] + (p1[0] - p0[0]) * factor;
					y = p0[1] + (p1[1] - p0[1]) * factor;
					z = boundMin[2];
				}

				// Now we move outside point to intersection point to clip
				// and get ready for next pass.
				if (outcodeOut == outcode0) {
					p0[0] = x;
					p0[1] = y;
					p0[2] = z;
					outcode0 = computeOutCode(p0, boundMin, boundMax);
				}
				else {
					p1[0] = x;
					p1[1] = y;
					p1[2] = z;
					outcode1 = computeOutCode(p1, boundMin, boundMax);
				}
			}
		}
		return accept;
	}

	int computeOutCode(const glm::vec3& p, const glm::vec3& boundMin, const glm::vec3& boundMax)
	{
		//modified version of this algorithm https://en.wikipedia.org/wiki/Cohen%E2%80%93Sutherland_algorithm

		//some constants for line clipping
		const int INSIDE = 0; // 000000
		const int LEFT = 1;   // 000001
		const int RIGHT = 2;  // 000010
		const int BOTTOM = 4; // 000100
		const int TOP = 8;    // 001000
		const int FRONT = 16; // 010000
		const int BACK = 32;  // 100000

		int code;

		code = INSIDE;	// initialised as being inside of [[clip window]]

		if (p[0] < boundMin[0])			// to the left of clip window
			code |= LEFT;
		else if (p[0] > boundMax[0])	// to the right of clip window
			code |= RIGHT;
		if (p[1] < boundMin[1])			// below the clip window
			code |= BOTTOM;
		else if (p[1] > boundMax[1])	// above the clip window
			code |= TOP;
		if (p[2] < boundMin[2])			// below the clip window
			code |= FRONT;
		else if (p[2] > boundMax[2])	// above the clip window
			code |= BACK;

		return code;
	}
*/