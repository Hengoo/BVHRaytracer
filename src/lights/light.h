#pragma once
#include <iostream>
#include <algorithm>
#include <vector>

#include "../glmInclude.h"

class Light
{
public:
	//direction points towards the light
	virtual std::array<uint8_t, 4> getLightDirection(const glm::vec3 & position, glm::vec3 & direction, float& distance) = 0;
};