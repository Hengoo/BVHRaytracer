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

	//save result here? ps: need to care with parrallel writes here when i start  with threads
	Color result;

	//TODO check that i dont render things behind the camera (and throw node collisions away that are behind the camera)
	//float distance;
	float tMax;

	//result would be primitive + distance + normal +  ?vector of secondary rays?


	Ray(glm::vec3 pos, glm::vec3 direction, bool shadowRay = false) : pos(pos), direction(direction), shadowRay(shadowRay)
	{
		this->direction = glm::normalize(direction);
		invDirection = 1.0f / this->direction;
		tMax = 222222.f;
		result = Color(0);
	}

	~Ray()
	{
	}

protected:



};