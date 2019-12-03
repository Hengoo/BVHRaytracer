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

glm::vec3 Camera::getAmbientDirection(RenderInfo& info, int i, glm::vec3& surfaceNormal)
{
	//i need two deterministic random values. Currently pixel index but could also be intersect position
	auto hashFunction = std::hash<size_t>();
	float u = hashFunction(info.index * 599 + i * 223) / (float)(std::numeric_limits<size_t>::max());
	float v = hashFunction(info.index * 181 + i * 691) / (float)(std::numeric_limits<size_t>::max());

	auto direction = sampleHemisphere(u, v, surfaceNormal);
	return direction;
}

glm::vec3 Camera::sampleHemisphere(float u, float v, glm::vec3& normal, int m)
{
	//inspired by this blogpost and adjusted for my needs
	//https://blog.thomaspoulet.fr/uniform-sampling-on-unit-hemisphere/
	float theta = acos(pow(1 - u, 1 / (float)(1 + m)));
	float phi = 2 * M_PI * v;

	float x = sin(theta) * cos(phi);
	float y = sin(theta) * sin(phi);
	float z = -cos(theta);

	//i dont like this approach but for now it has to do
	glm::vec4 tmp(x, y, z, 0);
	glm::vec3 up(0);
	if (abs(normal.y) < 0.9f)
	{
		up.y = 1;
	}
	else
	{
		up.x = 1;
	}
	auto matrix = glm::lookAt(glm::vec3(0), normal, up);

	auto res2 = tmp * matrix;
	return glm::vec3(res2);
}

glm::vec3 Camera::getRayTargetPosition(RenderInfo& info)
{
	glm::vec4 centerOffset = (glm::vec4(0, 1, 0, 0) * (float)info.h + glm::vec4(1, 0, 0, 0) * (float)info.w) * (1.0f / width) + glm::vec4(0, 0, -focalLength, 0);
	//next line to get perfect forward ray
	//centerOffset = glm::vec4(0, 0, -1, 0);
	glm::vec3 pos = position + glm::vec3(transform * centerOffset);
	return pos;
}