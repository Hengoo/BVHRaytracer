#include "camera.h"

#include <iostream>
//for pi
#define _USE_MATH_DEFINES
#include <math.h>

void Camera::fillRenderInfo()
{
	renderInfos.resize(height * width);
	//fill RenderInfo array.
	for (int i = 0; i < width * height; i++)
	{
		float w = (i % width - width / 2.f);
		float h = -(i / width - height / 2.f);

		renderInfos[i] = RenderInfo(w, h, i);
	}
}

glm::vec3 Camera::getAmbientDirection(const int index, const glm::vec3& surfaceNormal, const int i)
{
	//i need two deterministic random values. Currently pixel index but could also be intersect position
	auto hashFunction = std::hash<size_t>();
	float u = hashFunction(index * 599 + i * 223) / (float)(std::numeric_limits<size_t>::max());
	float v = hashFunction(index * 181 + i * 691) / (float)(std::numeric_limits<size_t>::max());

	auto direction = sampleHemisphere(surfaceNormal, u, v);
	return direction;
}

glm::vec3 Camera::sampleHemisphere(const glm::vec3& normal, const float u, const float v)
{
	//inspired by this blogpost and adjusted for my needs (changed it a bit to be faster)
	//https://blog.thomaspoulet.fr/uniform-sampling-on-unit-hemisphere/

	//uniform sampling over 
	float phi = acos(1 - 2 * u);
	float theta = 2 * M_PI * v;

	float x = sin(phi) * cos(theta);
	float y = sin(phi) * sin(theta);
	float z = cos(phi);

	glm::vec3 result(x, y, z);
	if (glm::dot(result, normal) <= 0)
	{
		result = -result;
	}
	//return result + normal * 0.2f;
	return result;
}