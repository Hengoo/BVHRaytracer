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

	glm::vec3 surfaceNormal;
	glm::vec3 surfacePosition;

	//float distance;
	float tMax;
	bool shadowRay;

	FastRay(glm::vec3 pos, glm::vec3 direction, bool shadowRay = false);
};
