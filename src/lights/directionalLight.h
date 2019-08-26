#pragma once
#include <iostream>
#include <algorithm>
#include <vector>

#include "light.h"
#include "../glmInclude.h"

class DirectionalLight : public Light
{
private:
	glm::vec3 direction;
	float intensity;
public:
	DirectionalLight(glm::vec3 direction, float intensity)
		:direction(direction), intensity(intensity)
	{
		this->direction = glm::normalize(this->direction);
	}

	//direction to the directional light
	virtual std::array<unsigned char, 4> getLightDirection(const glm::vec3 & position, glm::vec3 & direction, float& distance) override
	{
		direction = -this->direction;
		//something large
		distance = 222222.f;
		return { 255,255,255,255 };
	}
};