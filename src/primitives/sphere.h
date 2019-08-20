#pragma once
#include <iostream>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> //https://glm.g-truc.net/0.9.4/api/a00158.html  for make quat from a pointer to a array
#include <glm/gtx/hash.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "primitive.h"

class Sphere : public Primitive
{
public:
	glm::vec3 pos;
	float radius;

	Sphere(glm::vec3 pos, float radius) : pos(pos), radius(radius)
	{
	}

	~Sphere()
	{
	}

	bool virtual intersect(std::shared_ptr<Ray> ray) override
	{


		glm::vec3 oc = ray->pos - pos;
		float a = glm::dot(ray->direction, ray->direction);
		float b = 2.0f * glm::dot(oc, ray->direction);
		float c = glm::dot(oc, oc) - radius * radius;
		float discriminant = b * b - 4.0f * a * c;
		if (discriminant < 0)
		{
			return false;
		}
		else
		{
			//std::cout << "sphere intersect true" << std::endl;
			float dist = (-b - sqrt(discriminant)) / (2.0f * a);
			if (dist < ray->distance)
			{
				//ray->result = {0,0,0,255};
				auto tmp = ray->pos * 10.0f + ray->direction * dist ;
				ray->result = {(unsigned char) tmp.x,(unsigned char)tmp.y, (unsigned char) tmp.z,255 };
				ray->distance = dist;
			}
			return true;
		}
	}

protected:



};