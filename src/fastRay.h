#pragma once
#include <iostream>
#include <vector>
#include <array>

#include "glmInclude.h"
//#include "glmUtil.h"
//#include "util.h"
//#include "global.h"

class FastRay
{
public:

	//ray : pos + time * distance
	glm::vec3 pos;
	//direction normalised
	glm::vec3 direction;
	glm::vec3 invDirection;

	//float distance;
	float tMax;

	//triangle info to recover the position and normal after the primary ray intersection.
	uint32_t leafIndex;
	uint8_t triIndex;


	FastRay(const glm::vec3& pos, const glm::vec3& direction);
};
