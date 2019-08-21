#pragma once
#include <iostream>
#include <algorithm>
#include <vector>

//for the parallel for
#include <execution>

#include "node.h"
#include "../ray.h"
//#include "../primitives/primitive.h"

#include "glmInclude.h"
#include "../primitives/primitive.h"

//forward declarations:
class Primitive;

class Aabb : public Node
{
public:
	//edge with smallest value in each dimension
	glm::vec3 minBound;
	glm::vec3 boundDimension;
	//possible rotation??? -> since its just a bvh tester it doesnt really matter

	Aabb(glm::vec3 pos = glm::vec3(0), glm::vec3 dimension = glm::vec3(0)) : minBound(pos), boundDimension(dimension)
	{
	}

	virtual void constructBvh() override
	{
		//first update bounds of current aabb according to primitive:
		glm::vec3 min(INT_MAX), max(-INT_MAX);
		glm::vec3 minp, maxp;
		for (auto& p : primitives)
		{
			p->getBounds(minp, maxp);
			min = glm::min(min, minp);
			max = glm::max(max, maxp);
		}
		minBound = min;
		boundDimension = max - min;

		//check primitive count. if less than x primitives, stop.
		if (primitives.size() <= 1)
		{
			return;
		}

		//basic approach: construct multiple aabb and check all primitives against it. -> if its in then add it

		//octree:
		auto dim = boundDimension * 0.5f;
		std::vector<std::shared_ptr<Node>> boxes;

		auto minx = glm::vec3(dim.x, 0, 0);
		auto miny = glm::vec3(0, dim.y, 0);
		auto minz = glm::vec3(0, 0, dim.z);

		boxes.push_back(std::make_shared<Aabb>(minBound, dim));


		boxes.push_back(std::make_shared<Aabb>(minBound + minx, dim));
		boxes.push_back(std::make_shared<Aabb>(minBound + miny, dim));
		boxes.push_back(std::make_shared<Aabb>(minBound + minz, dim));

		boxes.push_back(std::make_shared<Aabb>(minBound + minx + minz, dim));
		boxes.push_back(std::make_shared<Aabb>(minBound + miny + minz, dim));
		boxes.push_back(std::make_shared<Aabb>(minBound + minx + miny, dim));

		boxes.push_back(std::make_shared<Aabb>(minBound + minx + miny + minz, dim));

		std::for_each(std::execution::seq, primitives.begin(), primitives.end(),
			[&](auto& prim)
			{
				for (auto& box : boxes)
				{
					if (prim->intersect(box))
					{
						box->addPrimitive(prim);
					}
				}
			});

		//check if one box has all primitives this node has
		for (auto& b : boxes)
		{
			if (b->getPrimCount() == getPrimCount())
			{
				return;
			}
		}

		//only add not empty aabb to the children.
		for (auto& b : boxes)
		{
			if (b->getPrimCount() != 0)
			{
				addNode(b);
			}
		}


		//todo: keep primitives that are placed in all children in this node

		primitives.clear();

		//constructs bvh of all children:
		Node::constructBvh();
	}

	virtual bool intersect(std::shared_ptr<Ray> ray) override
	{
		//mostly from here https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection

		//TODO replace / with * inverse direction so its only calc once
		glm::vec3 tmin = (minBound - ray->pos) * ray->invDirection;
		glm::vec3 tmax = (minBound - ray->pos + boundDimension) * ray->invDirection;

		float tmp = 0;
		if (ray->direction.x < 0)
		{
			tmp = tmin.x;
			tmin.x = tmax.x;
			tmax.x = tmp;
		}
		if (ray->direction.y < 0)
		{
			tmp = tmin.y;
			tmin.y = tmax.y;
			tmax.y = tmp;
		}
		if (ray->direction.z < 0)
		{
			tmp = tmin.z;
			tmin.z = tmax.z;
			tmax.z = tmp;
		}
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

		//ray->result[1] += 10;
		//intersect all primitves and nodes:
		return Node::intersect(ray);
	}

protected:





};
