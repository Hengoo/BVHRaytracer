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

#include "../glmUtil.h"

//forward declarations:
class Primitive;

class Aabb : public Node
{
public:
	//edge with smallest value in each dimension
	glm::vec3 boundMin;
	glm::vec3 boundMax;
	//possible rotation??? -> since its just a bvh tester it doesnt really matter

	Aabb(unsigned int depth, glm::vec3 boundMin = glm::vec3(-222222.0f), glm::vec3 boundMax = glm::vec3(222222.0f)) : Node(depth), boundMin(boundMin), boundMax(boundMax)
	{
	}

	virtual void recursiveBvh(const unsigned int branchingFactor, const unsigned int leafCount)
	{
		//update  bounds and center based on the primitives:
		//we dont care about previous bounds
		glm::vec3 min(222222.0f), max(-222222.0f);
		glm::vec3 minp, maxp;
		for (auto& p : primitives)
		{
			p->getBounds(minp, maxp);
			min = glm::min(min, minp);
			max = glm::max(max, maxp);
		}

		boundMin = min;
		boundMax = max;

		//check primitive count. if less than x primitives, stop.
		if (getPrimCount() <= leafCount)
		{
			//TODO i think i should create an aabb for those triangles IF the leafcount > 1
			return;
		}

		//choose axis to split:
		int axis = 0;
		//calculate distance of centers along each axis -> largest distance is the axis we want to split
		min = glm::vec3(222222.0f);
		max = glm::vec3(-222222.0f);
		glm::vec3 centerDistance;
		for (auto& p : primitives)
		{
			centerDistance = p->getCenter();
			min = glm::min(min, centerDistance);
			max = glm::max(max, centerDistance);
		}
		centerDistance = max - min;

		//TODO: possible  version i want to try: take sum of aabb boxes and split the one with the SMALLEST sum (-> least overlapping?)

		axis = maxDimension(centerDistance);

		//stop when triangle centers are at the same position (when this happens to often i might try the real center instead of the aabb center????
		if (centerDistance[axis] <= 0.00002)
		{
			//std::cout << "all triangles at same pos" << std::endl;
			return;
		}

		//spliting factor:
		float factor = 0.5f;
		//for now i split at the middle of the triangle centers
		float cent = max[axis] * 0.5f + min[axis] * 0.5f;
		factor = (cent - boundMin[axis]) / (boundMax[axis] - boundMin[axis]);

		//TODO: calculate factor acording to heuristic (i could give the heuristic as a function pointer??)

		std::vector<std::shared_ptr<Node>> boxes;

		glm::vec3 f1(0), f2(0);
		f1[axis] = factor;
		f2[axis] = 1.0f - factor;

		boxes.push_back(std::make_shared<Aabb>(depth + 1, boundMin, boundMax + ((boundMin - boundMax) * f2)));
		boxes.push_back(std::make_shared<Aabb>(depth + 1, boundMin + ((boundMax - boundMin) * f1), boundMax));

		std::vector<bool> flag(primitives.size());
		//for (auto& prim : primitives)
		for (size_t i = 0; i < primitives.size(); i++)
		{
			for (auto& box : boxes)
			{
				if (!flag[i] && primitives[i]->intersect(&*box))
				{
					//i have to check how often it happens that a primitive is added to two nodes due to floating point accuracy
					box->addPrimitive(primitives[i]);
					flag[i] = true;
				}
			}
		}

		//only add not empty nodes to the children.
		for (auto& b : boxes)
		{
			if (b->getPrimCount() != 0)
			{
				addNode(b);
			}
			else
			{
				std::cout << "tried to make node without children." << std::endl;
				return;
			}
		}

		//todo: try keeping primitives in this node when they are very large (this would keep child aabb boxes smaller (might save much for HUGE triangles)
		primitives.clear();
		primitives.shrink_to_fit();

		if (depth >= 35)
		{
			std::cout << depth << std::endl;
		}
		//constructs bvh of all children:
		Node::recursiveBvh(branchingFactor, leafCount);
	}

	//rekursive octreetree build
	virtual void recursiveOctree(const unsigned int leafCount) override
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

		boundMin = min;
		boundMax = max;

		//check primitive count. if less than x primitives, stop.
		if (primitives.size() <= leafCount)
		{
			return;
		}

		//approach: construct multiple aabb and check all primitives against it. -> if center is in it then add it
		//in theory i could try out different factors (replace 0.5 with vector with different values and check how good resulting aabb are)
		//but this would have to be done for all axis

		//octree:
		auto dim = (boundMax - boundMin) * 0.5f;
		std::vector<std::shared_ptr<Node>> boxes;

		auto minx = glm::vec3(dim.x, 0, 0);
		auto miny = glm::vec3(0, dim.y, 0);
		auto minz = glm::vec3(0, 0, dim.z);

		boxes.push_back(std::make_shared<Aabb>(depth + 1, boundMin, boundMin + dim));

		boxes.push_back(std::make_shared<Aabb>(depth + 1, boundMin + minx, boundMin + minx + dim));
		boxes.push_back(std::make_shared<Aabb>(depth + 1, boundMin + miny, boundMin + miny + dim));
		boxes.push_back(std::make_shared<Aabb>(depth + 1, boundMin + minz, boundMin + minz + dim));

		boxes.push_back(std::make_shared<Aabb>(depth + 1, boundMin + minx + minz, boundMin + minx + minz + dim));
		boxes.push_back(std::make_shared<Aabb>(depth + 1, boundMin + miny + minz, boundMin + miny + minz + dim));
		boxes.push_back(std::make_shared<Aabb>(depth + 1, boundMin + minx + miny, boundMin + minx + miny + dim));

		boxes.push_back(std::make_shared<Aabb>(depth + 1, boundMin + minx + miny + minz, boundMin + minx + miny + minz + dim));

		for (auto& prim : primitives)
		{
			for (auto& box : boxes)
			{
				if (prim->intersect(&*box))
				{
					//i probably should remove this primitive so it cannot be added do another node?
					box->addPrimitive(prim);
				}
			}
		}

		//check if one box has all primitives this node has (prevents loops caused by dumb octree splitting)
		for (auto& b : boxes)
		{
			if (b->getPrimCount() == getPrimCount())
			{
				return;
			}
		}

		//only add not empty nodes to the children.
		for (auto& b : boxes)
		{
			if (b->getPrimCount() != 0)
			{
				addNode(b);
			}
		}

		primitives.clear();
		primitives.shrink_to_fit();

		//constructs bvh of all children:
		Node::recursiveOctree(leafCount);
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
