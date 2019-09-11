#pragma once
#include <iostream>
#include <vector>
#include <array>


#include "glmInclude.h"
#include "color.h"

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

	//TODO check that i dont render things behind the camera (and throw node collisions away that are behind the camera)
	//float distance;
	float tMax;

	std::vector<unsigned int> nodeIntersectionCount;
	std::vector<unsigned int> leafIntersectionCount;
	unsigned int primitiveIntersectionCount;
	unsigned int successfulPrimitiveIntersectionCount;
	unsigned int successfulNodeIntersectionCount;
	unsigned int successfulLeafIntersectionCount;
	//result would be primitive + distance + normal +  ?vector of secondary rays?

	Ray(glm::vec3 pos, glm::vec3 direction, bool shadowRay = false) : pos(pos), direction(direction), shadowRay(shadowRay)
	{
		this->direction = glm::normalize(direction);
		invDirection = 1.0f / this->direction;
		tMax = 222222.f;
		surfaceColor = Color(0);
		surfaceNormal = glm::vec3(0, 1, 0);
		surfacePosition = glm::vec3(0, 0, 0);
		primitiveIntersectionCount = 0;
		successfulPrimitiveIntersectionCount = 0;
		successfulNodeIntersectionCount = 0;
		successfulLeafIntersectionCount = 0;
	}

	~Ray()
	{
	}
};