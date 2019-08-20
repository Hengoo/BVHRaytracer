#pragma once
#include <iostream>
#include <vector>
#include <array>


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> //https://glm.g-truc.net/0.9.4/api/a00158.html  for make quat from a pointer to a array
#include <glm/gtx/hash.hpp>
#include <glm/gtx/matrix_decompose.hpp>

class Ray
{
public:

	//ray : pos + time * distance
	glm::vec3 pos;
	//direction normalised
	glm::vec3 direction;


	//save result here? ps: need to care with parrallel writes here when i start  with threads
	std::array<unsigned char, 4> result{0,0,0,255};
	float distance;

	//result would be primitive + distance + normal +  ?vector of secondary rays?


	Ray(glm::vec3 pos, glm::vec3 direction) : pos(pos), direction(direction)
	{
		direction = glm::normalize(direction);
		distance = INT_MAX;
	}

	~Ray()
	{
	}

protected:



};