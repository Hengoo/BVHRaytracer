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

	//float distance;
	float tMax;

	//ray : pos + time * distance
	glm::vec3 pos;
	glm::vec3 invDirection;
	glm::vec3 direction;

	void updateDirection(glm::vec3& newDirection);

	FastRay(const glm::vec3& pos, const glm::vec3& direction, float maxDistance = 222222.f);
	FastRay()
	{};
};
