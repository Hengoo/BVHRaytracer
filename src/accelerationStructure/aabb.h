#pragma once
#include <iostream>
#include <vector>
#include <array>
#include <algorithm>

//for the parallel for
#include <execution>

#include "node.h"
#include "../ray.h"

#include "../glmInclude.h"
#include "../primitives/primitive.h"

//forward declarations:
class Primitive;

class Aabb : public Node
{
public:
	//edge with smallest value in each dimension
	glm::vec3 boundMin;
	glm::vec3 boundMax;
	//possible rotation??? -> since its just a bvh tester it doesnt really matter

	Aabb(glm::vec3 boundMin = glm::vec3(-222222.0f), glm::vec3 boundMax = glm::vec3(222222.0f)) : boundMin(boundMin), boundMax(boundMax)
	{
	}

	virtual void constructBvh(unsigned int depth, const unsigned int branchingFactor, const unsigned int leafCount) override
	{
		//currently very basic octree approach

		//first update bounds of current aabb according to primitive:
		glm::vec3 min(222222.0f), max(-222222.0f);
		glm::vec3 minp, maxp;
		for (auto& p : primitives)
		{
			p->getBounds(minp, maxp);
			min = glm::min(min, minp);
			max = glm::max(max, maxp);
		}

		//make sure the aabb box never gets larger
		boundMin = glm::max(min, boundMin);
		boundMax = glm::min(max, boundMax);

		//check primitive count. if less than x primitives, stop.
		if (primitives.size() <= leafCount)
		{
			return;
		}

		//approach: construct multiple aabb and check all primitives against it. -> if its in then add it
		//in theory i could try out different factors (replace 0.5 with vector with different values and check how good resulting aabb are)

		//octree:
		auto dim = (boundMax - boundMin) * 0.5f;
		std::vector<std::shared_ptr<Node>> boxes;

		auto minx = glm::vec3(dim.x, 0, 0);
		auto miny = glm::vec3(0, dim.y, 0);
		auto minz = glm::vec3(0, 0, dim.z);

		boxes.push_back(std::make_shared<Aabb>(boundMin, boundMin + dim));


		boxes.push_back(std::make_shared<Aabb>(boundMin + minx, boundMin + minx + dim));
		boxes.push_back(std::make_shared<Aabb>(boundMin + miny, boundMin + miny + dim));
		boxes.push_back(std::make_shared<Aabb>(boundMin + minz, boundMin + minz + dim));

		boxes.push_back(std::make_shared<Aabb>(boundMin + minx + minz, boundMin + minx + minz + dim));
		boxes.push_back(std::make_shared<Aabb>(boundMin + miny + minz, boundMin + miny + minz + dim));
		boxes.push_back(std::make_shared<Aabb>(boundMin + minx + miny, boundMin + minx + miny + dim));

		boxes.push_back(std::make_shared<Aabb>(boundMin + minx + miny + minz, boundMin + minx + miny + minz + dim));

		for (auto& prim : primitives)
		{
			for (auto& box : boxes)
			{
				if (prim->intersect(&*box))
				{
					box->addPrimitive(prim);
				}
			}
		}

		//check if one box has all primitives this node has
		for (auto& b : boxes)
		{
			//TODO: this can lead to quite large leaf sizes -> or to endless loops if i want to obey leafsize
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

		//todo: try keeping primitives that are placed in all children in this node (and dont have them in the children)
		primitives.clear();
		primitives.shrink_to_fit();

		//constructs bvh of all children:
		Node::constructBvh(depth, branchingFactor, leafCount);
	}

	virtual bool intersect(Ray& ray) override
	{
		float t;
		//second answer form here: https://gamedev.stackexchange.com/questions/18436/most-efficient-aabb-vs-ray-collision-algorithms
		// r.dir is unit direction vector of ray
		// lb is the corner of AABB with minimal coordinates - left bottom, rt is maximal corner
		// r.org is origin of ray

		//float t1 = (boundMin.x - ray.pos.x) * ray.invDirection.x;
		//float t2 = (boundMax.x - ray.pos.x) * ray.invDirection.x;
		//float t3 = (boundMin.y - ray.pos.y) * ray.invDirection.y;
		//float t4 = (boundMax.y - ray.pos.y) * ray.invDirection.y;
		//float t5 = (boundMin.z - ray.pos.z) * ray.invDirection.z;
		//float t6 = (boundMax.z - ray.pos.z) * ray.invDirection.z;


		//float tmin = std::max(std::max(std::min(t1, t2), std::min(t3, t4)), std::min(t5, t6));
		//float tmax = std::min(std::min(std::max(t1, t2), std::max(t3, t4)), std::max(t5, t6));

		//glm version of above code (faster)
		glm::fvec3 t1 = (boundMin - ray.pos) * ray.invDirection;
		glm::fvec3 t2 = (boundMax - ray.pos) * ray.invDirection;
		float tmin = glm::compMax(glm::min(t1, t2));
		float tmax = glm::compMin(glm::max(t1, t2));

		// if tmax < 0, ray (line) is intersecting AABB, but the whole AABB is behind us
		if (tmax < 0)
		{
			t = tmax;
			return false;
		}

		// if tmin > tmax, ray doesn't intersect AABB
		if (tmin > tmax)
		{
			t = tmax;
			return false;
		}

		//distance to aabb suface:
		//t = tmin;

		//stop when current ray distance is closer than minimum possible distance of the aabb
		if (ray.tMax < tmin)
		{
			return false;
		}

		//check if intersection is closer than

		//intersection occured:
		//intersect all primitves and nodes:
		return Node::intersect(ray);
	}
};
