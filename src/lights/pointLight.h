#pragma once
#include <iostream>
#include <algorithm>
#include <vector>

#include "light.h"
#include "../glmInclude.h"

class PointLight : public Light
{
private:
	glm::vec3 pos;
	float intensity;
public:
	PointLight(glm::vec3 pos, float intensity)
		:pos(pos), intensity(intensity)
	{
	}

	//direction from the position to the light
	virtual std::array<uint8_t, 4> getLightDirection(const glm::vec3 & position, glm::vec3 & direction, float& distance) override
	{
		direction = glm::normalize(position - pos);
		distance = glm::length(position - pos);
		return { 255,255,255,255 };
	}
};