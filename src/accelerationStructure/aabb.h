#pragma once
#include <iostream>
#include <algorithm>
#include <vector>


#include "node.h"
#include "../ray.h"
#include "../primitives/primitive.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> //https://glm.g-truc.net/0.9.4/api/a00158.html  for make quat from a pointer to a array
#include <glm/gtx/hash.hpp>
#include <glm/gtx/matrix_decompose.hpp>

class Aabb : public Node
{
public:
	//edge with smallest value in each dimension
	glm::vec3 pos;
	glm::vec3 dimension;
	//possible rotation??? -> since its just a bvh tester it doesnt really matter

	Aabb(glm::vec3 pos, glm::vec3 dimension): pos(pos), dimension(dimension)
	{

	}

	virtual bool intersect(std::shared_ptr<Ray> ray) override
	{
		//mostly from here https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection

		//TODO replace / with * inverse direction so its only calc once
		glm::vec3 tmin = (pos - ray->pos) / ray->direction;
		glm::vec3 tmax = (pos - ray->pos  + dimension) / ray->direction;

		//if oriented box: rotate ray and rotate origin by box rotation (this has to bed one before the 

		if ((tmin.x > tmax.y) || (tmin.y > tmax.x))
		{
			return false;
		}
		if (tmin.y > tmin.x)
		{
			tmin.x = tmin.y;
		}
		if (tmax.y < tmax.x)
		{
			tmax.x = tmax.y;
		}

		//in theory i only need to compute the z part here
		if ((tmin.x > tmax.z) || (tmin.z > tmax.x))
		{
			return false;
		}
		if (tmin.z > tmin.x)
		{
			tmin.x = tmin.z;
		}
		if (tmax.z < tmax.x)
		{
			tmax.x = tmax.z;
		}
		//intersect all primitves and nodes:
		return Node::intersect(ray);
	}

protected:





};
