#pragma once
#include <iostream>
#include <vector>
#include <array>


#include "glmInclude.h"

class Ray
{
public:

	//ray : pos + time * distance
	glm::vec3 pos;
	//direction normalised
	glm::vec3 direction;

	glm::vec3 invDirection;



	//save result here? ps: need to care with parrallel writes here when i start  with threads
	std::array<unsigned char, 4> result{ 0,0,0,255 };
	float distance;

	//result would be primitive + distance + normal +  ?vector of secondary rays?


	Ray(glm::vec3 pos, glm::vec3 direction) : pos(pos), direction(direction)
	{
		this->direction = glm::normalize(direction);
		invDirection = 1.0f / this->direction;
		distance = INT_MAX;
	}

	~Ray()
	{
	}

protected:



};