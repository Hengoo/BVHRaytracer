#pragma once

#include "glmInclude.h"

//returns the index of the largest dimension http://www.pbr-book.org/3ed-2018/Geometry_and_Transformations/Vectors.html#Vector3::MaxDimension
static inline int maxDimension(glm::vec3& v)
{
	return (v.x > v.y) ? ((v.x > v.z) ? 0 : 2) :
		((v.y > v.z) ? 1 : 2);
}

static inline int maxAbsDimension(glm::vec3 v)
{
	v = glm::abs(v);
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
	return glm::normalize(glm::cross(p3 - p1, p2 - p1));
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
	return 0.5 * glm::length(glm::cross(a, b));
}