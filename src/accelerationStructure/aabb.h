#pragma once
#include <iostream>
#include <vector>
#include <array>
#include <algorithm>

//for the parallel for
#include <execution>

//#include <atomic>

#include "node.h"
#include "../ray.h"

#include "../glmInclude.h"
#include "../primitives/primitive.h"
#include "../glmUtil.h"
#include "../typedef.h"

//forward declarations:
class Primitive;

struct NodePair
{
	std::unique_ptr<Node> node1;
	std::unique_ptr<Node> node2;
	std::vector<float>::iterator metricBegin;
	std::vector<float>::iterator metricEnd;
	std::vector<float>::reverse_iterator metricRevBegin;
	std::vector<float>::reverse_iterator metricRevEnd;
	int index;
};

static bool sortPrimitive(std::shared_ptr<Primitive>& p1, std::shared_ptr<Primitive>& p2, int axis)
{
	return p1->getCenter()[axis] < p2->getCenter()[axis];
}
class Aabb : public Node
{

public:
	//edge with smallest value in each dimension
	glm::vec3 boundMin;
	glm::vec3 boundMax;
	//possible rotation??? -> since its just a bvh tester it doesnt really matter

	Aabb(unsigned int depth, std::shared_ptr<primPointVector> primitives, primPointVector::iterator primitiveBegin, primPointVector::iterator primitiveEnd)
		: Node(depth, primitives, primitiveBegin, primitiveEnd)
	{
		calculateBounds();
	}

	//bool sortBy(Primitive& p1, Primitive& p2, int& axis)

	void calculateBounds()
	{
		//calculate bounds based on the primitives:
		glm::vec3 min(222222.0f), max(-222222.0f);
		glm::vec3 minp, maxp;
		std::for_each(std::execution::seq, primitiveBegin, primitiveEnd,
			[&](auto& p)
			{
				p->getBounds(minp, maxp);
				min = glm::min(min, minp);
				max = glm::max(max, maxp);
			});

		boundMin = min;
		boundMax = max;
	}

	float getSurfaceArea() override
	{
		auto d = boundMax - boundMin;
		return 2 * (d.x * d.y + d.x * d.z + d.y * d.z);
	}

	virtual void recursiveBvh(const unsigned int branchingFactor, const unsigned int leafCount)
	{
		//check primitive count. if less than x primitives, this node is finished. (pbrt would continue of leafcost is larger than split cost !!!)
		if (getPrimCount() <= leafCount)
		{
			return;
		}

		//TODO: if i want to keep primitives in this node i have to spawn them to primitiveBegin BEFORE the sort
		//also dont forget to set primitiveBegin and primitiveEnd correctly before end of method

		int axis = 0;
		//calculate distance of centers along each axis -> largest distance is the axis we want to split
		glm::vec3 min = glm::vec3(222222.0f);
		glm::vec3 max = glm::vec3(-222222.0f);
		glm::vec3 centerDistance;
		std::for_each(std::execution::seq, primitiveBegin, primitiveEnd,
			[&](auto& p)
			{
				centerDistance = p->getCenter();
				min = glm::min(min, centerDistance);
				max = glm::max(max, centerDistance);
			});
		centerDistance = max - min;

		//choose axis to split:
		//TODO: possible  version i want to try: take sum of aabb boxes and split the one with the SMALLEST sum (-> least overlapping?)
		axis = maxDimension(centerDistance);
		sortAxis = axis;

		//stop when triangle centers are at the same position (when this happens to often i might try the real center instead of the aabb center????
		//if (centerDistance[axis] <= 0.00002)
		//{
		//	return;
		//}


		//sort primitive array along axis:
		//its faster to first check if its sorted
		if (!std::is_sorted(primitiveBegin, primitiveEnd, std::bind(sortPrimitive, std::placeholders::_1, std::placeholders::_2, axis)))
		{
			//TODO test what parallel stuff like std::execution::seq or unseqpar does here
			std::sort(primitiveBegin, primitiveEnd, std::bind(sortPrimitive, std::placeholders::_1, std::placeholders::_2, axis));
		}

		//approach: test all versions (go trough sorted primitives and with each iteration add one more to the second node until we tried all.
		//test all combos with heuristic  (save results in array and take the one with best result;

		unsigned int size = getPrimCount();
		primPointVector::iterator currentSplit = primitiveBegin + 1;

		//Split by metric (in our case sah)
		std::vector<float> metric(size - 1);
		if (true)
		{
			//only do parallel versionf or first few nodes (with enought primitives) --> TODO try out values
			//its faster without parallel since the left / right sweep
			if (false && size > 5000 && depth < 2)//depth < 3 && size > 50)
			{
				//parallel version: (calculate the metric for different intervals parallel)
				//-> usefull for the first few nodes because after all nodes work parallel

				//the reason why its faster to have this large is because there are clusters that require a node bound recalculation for each decreasePrimitives
				int parallelCount = 8;
				std::vector<NodePair> work(parallelCount);
				for (size_t i = 0; i < parallelCount; i++)
				{
					int start = (size / parallelCount) * i;
					int end = (size / parallelCount) * (i + 1);

					work[i].node1 = std::make_unique<Aabb>(depth + 1, primitives, primitiveBegin, primitiveBegin + start);
					work[i].node2 = std::make_unique<Aabb>(depth + 1, primitives, primitiveBegin + end, primitiveEnd);
					work[i].metricBegin = metric.begin() + start;
					if (i == parallelCount - 1)
					{
						work[i].metricEnd = metric.end();
					}
					else
					{
						work[i].metricEnd = metric.begin() + end;
					}


					if (i == parallelCount - 1)
					{
						work[i].metricRevBegin = metric.rbegin();
					}
					else
					{
						work[i].metricRevBegin = metric.rbegin() + (size - end);
					}
					if (i == 0)
					{
						work[i].metricRevEnd = metric.rend();
					}
					else
					{
						work[i].metricRevEnd = metric.rbegin() + (size - start);
					}

					work[i].index = i;
					//std::cout << "worklaod: " << std::distance(work[i].metricBegin, work[i].metricEnd) << std::endl;
				}

				std::for_each(std::execution::par_unseq, work.begin(), work.end(),
					[&](auto& w)
					{
						float invArea = 1 / getSurfaceArea();
						int  a = 0;
						std::for_each(std::execution::seq, w.metricBegin, w.metricEnd,
							[&](auto& met)
							{
								w.node1->sweepRight();
								met += sah(*w.node1.get(), invArea, leafCount);
							});
						std::for_each(std::execution::seq, w.metricRevBegin, w.metricRevEnd,
							[&](auto& met)
							{
								w.node2->sweepLeft();
								met += sah(*w.node2.get(), invArea, leafCount);
							});
						//std::cout << "work " << w.index << " finished" <<std::endl;
					});
			}
			else
			{
				//full sequential version:
				Aabb node(depth + 1, primitives, primitiveBegin, primitiveBegin);
				float invArea = 1 / getSurfaceArea();
				int  a = 0;
				std::for_each(std::execution::seq, metric.begin(), metric.end(),
					[&](auto& met)
					{
						node.sweepRight();
						met += sah(node, invArea, leafCount);
					});
				node = Aabb(depth + 1, primitives, primitiveEnd, primitiveEnd);
				std::for_each(std::execution::seq, metric.rbegin(), metric.rend(),
					[&](auto& met)
					{
						node.sweepLeft();
						met += sah(node, invArea, leafCount);
					});
			}
			//make the split with the best metric:
			currentSplit = primitiveBegin + std::distance(metric.begin(), std::min_element(metric.begin(), metric.end())) + 1;
			//if (depth < 5)
			//{
			//	std::cout << depth << " depth , split number:" << std::distance(primitiveBegin, currentSplit) << std::endl;
			//}
		}
		else
		{
			//TODO try something that produces "full" leaf nodes (splitting by multiplicatives of leafsize?)

			//split by half
			currentSplit = primitiveBegin + size / 2;
		}



		addNode(std::make_shared<Aabb>(depth + 1, primitives, primitiveBegin, currentSplit));
		addNode(std::make_shared<Aabb>(depth + 1, primitives, currentSplit, primitiveEnd));

		//just a debug output to get warned abount eventual loop
		if (depth >= 35)
		{
			std::cout << depth << std::endl;
		}

		primitiveBegin = primitiveEnd;

		//constructs bvh of all children:
		Node::recursiveBvh(branchingFactor, leafCount);
	}

	/* todo: this is broken since i dont have a primitive array anymore
	//rekursive octreetree build
	virtual void recursiveOctree(const unsigned int leafCount) override
	{
		//currently very basic octree approach

		//first update bounds of current aabb according to primitive:
		glm::vec3 min(222222.0f), max(-222222.0f);
		glm::vec3 minp, maxp;
		//for (auto& p : primitives)
		std::for_each(std::execution::par_unseq, primitiveBegin, primitiveEnd,
			[&](auto& info)
			{
				p->getBounds(minp, maxp);
				min = glm::min(min, minp);
				max = glm::max(max, maxp);
			});

		boundMin = min;
		boundMax = max;

		//check primitive count. if less than x primitives, stop.
		if ((*primitives).size() <= leafCount)
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
	*/

	virtual bool intersectNode(Ray& ray, float& distance) override
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
		//ray.result.g += 0.002f;

		//tmax gives less triangle intersections when sorting
		distance = tmax;
		return true;
	}

	void increasePrimitives() override
	{
		//increment End iterator and adjust the bounds according to the new primitive;
		primitiveEnd = primitiveEnd + 1;

		glm::vec3 minp, maxp;
		(*primitiveEnd)->getBounds(minp, maxp);
		boundMin = glm::min(boundMin, minp);
		boundMax = glm::max(boundMax, maxp);
	}

	void decreasePrimitives() override
	{
		//increment Begin iterator and recalculate the bounds when the "lost" primitive was part of the bounds;
		glm::vec3 minp, maxp;
		(*primitiveBegin)->getBounds(minp, maxp);
		if (minp.x == boundMin.x || minp.y == boundMin.y || minp.z == boundMin.z
			|| maxp.x == boundMax.x || maxp.y == boundMax.y || maxp.z == boundMax.z)
		{
			//recalc bounds
			calculateBounds();
		}
		boundMin = glm::min(boundMin, minp);
		boundMax = glm::max(boundMax, maxp);
		primitiveBegin = primitiveBegin + 1;
	}

	void Node::sweepRight()
	{
		primitiveEnd = primitiveEnd + 1;

		glm::vec3 minp, maxp;
		(*primitiveEnd)->getBounds(minp, maxp);
		boundMin = glm::min(boundMin, minp);
		boundMax = glm::max(boundMax, maxp);
	}
	void Node::sweepLeft()
	{
		primitiveBegin = primitiveBegin - 1;

		glm::vec3 minp, maxp;
		(*primitiveBegin)->getBounds(minp, maxp);
		boundMin = glm::min(boundMin, minp);
		boundMax = glm::max(boundMax, maxp);
	}
};
