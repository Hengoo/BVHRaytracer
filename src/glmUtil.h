#pragma once

#include "glmInclude.h"

//returns the index of the largest dimension http://www.pbr-book.org/3ed-2018/Geometry_and_Transformations/Vectors.html#Vector3::MaxDimension
static inline int maxDimension(glm::vec3& v)
{
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