
#pragma once
#include <iostream>
#include <vector>
#include <array>
#include "accelerationStructure/bvh.h"

#include "glmInclude.h"
#include "color.h"
//#include "glmUtil.h"
//#include "util.h"
//#include "global.h"

class Ray
{
public:

	//ray : pos + time * distance
	glm::vec3 pos;
	//direction normalised
	glm::vec3 direction;
	glm::vec3 invDirection;

	bool shadowRay;

	Color surfaceColor;
	glm::vec3 surfaceNormal;
	glm::vec3 surfacePosition;

	//float distance;
	float tMax;

	std::vector<uint16_t> nodeIntersectionCount;
	std::vector<uint16_t> leafIntersectionCount;
	std::vector<uint16_t> childFullness;
	std::vector<uint16_t> primitiveFullness;
	uint16_t primitiveIntersectionCount;
	uint16_t successfulPrimitiveIntersectionCount;
	uint16_t successfulAabbIntersectionCount;
	uint16_t aabbIntersectionCount;

	Ray(glm::vec3 pos, glm::vec3 direction, Bvh bvh, bool shadowRay = false);

	~Ray()
	{
	}
};