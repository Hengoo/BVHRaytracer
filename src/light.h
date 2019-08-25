#pragma once
#include <iostream>
#include <algorithm>
#include <vector>

#include "glmInclude.h"

class Light
{
public:
	glm::vec3 pos;
	float intensity;

	Light(glm::vec3 pos, float intensity)
		:pos(pos), intensity(intensity)
	{
	}
};