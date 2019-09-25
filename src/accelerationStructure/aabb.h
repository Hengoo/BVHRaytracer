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
	//Used for bvh sweeping
	struct PrimIntervall
	{
		primPointVector::iterator primitiveBegin;
		primPointVector::iterator primitiveEnd;

		//assumes primitives are sorted (axis doesnt matter). Calculates bestSplit and bestSplitMetric
		PrimIntervall(primPointVector::iterator primitiveBegin, primPointVector::iterator primitiveEnd)
			:primitiveBegin(primitiveBegin), primitiveEnd(primitiveEnd)
		{
		}

		primPointVector::iterator computerBestSplit(float invSurfaceArea, int leafTarget, int bucketCount)
		{
			//method should be usable for all nodes?


			//version to sort each intervall itself -> mixed result, (without implementing correct traversal)
			/*
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
			int sortAxis = maxDimension(centerDistance);

			//sort primitive array along axis:
			//its faster to first check if its sorted
			if (!std::is_sorted(primitiveBegin, primitiveEnd, std::bind(sortPrimitive, std::placeholders::_1, std::placeholders::_2, sortAxis)))
			{
				//TODO test what parallel stuff like std::execution::seq or unseqpar does here
				std::sort(primitiveBegin, primitiveEnd, std::bind(sortPrimitive, std::placeholders::_1, std::placeholders::_2, sortAxis));
			}
			*/

			int size = getPrimCount();

			if (bucketCount <= 0 || size < bucketCount)
			{
				//test all combos with heuristic:
				//save results in array and take the one with best result

				// Split by metric (currently SAH)
				std::vector<float> metric(size - 1);

				Aabb node(0.f, primitiveBegin, primitiveBegin);
				std::for_each(std::execution::seq, metric.begin(), metric.end(),
					[&](auto& met)
					{
						node.sweepRight();
						met += node.sah(invSurfaceArea, leafTarget);
					});
				node = Aabb(0, primitiveEnd, primitiveEnd);
				std::for_each(std::execution::seq, metric.rbegin(), metric.rend(),
					[&](auto& met)
					{
						node.sweepLeft();
						met += node.sah(invSurfaceArea, leafTarget);
					});
				//make the split with the best metric:
				auto bestElement = std::min_element(metric.begin(), metric.end());
				return primitiveBegin + std::distance(metric.begin(), bestElement) + 1;
			}
			else
			{
				//greedy approach with buckets:
				std::vector<float> metric(bucketCount - 1);
				std::vector<float> metric2(bucketCount - 1);
				//split primitives into buckets:
				std::vector<std::unique_ptr<Aabb>> buckets;
				//easy version:
				for (size_t i = 0; i < bucketCount - 1; i++)
				{
					buckets.push_back(std::make_unique<Aabb>(0.f, primitiveBegin + (size / bucketCount) * i, primitiveBegin + (size / bucketCount) * (i + 1)));
				}
				buckets.push_back(std::make_unique<Aabb>(0.f, primitiveBegin + (size / bucketCount) * (bucketCount - 1), primitiveEnd));

				//better spacial version:
				//TODO

				//iterate trough buckets
				Aabb node(0.f, primitiveBegin, primitiveBegin);
				for (size_t i = 0; i < bucketCount - 1; i++)
				{
					//use all but the first bucket
					node.sweepRight(&*buckets[i + 1]);
					metric[i] = node.sah(invSurfaceArea, leafTarget);
				}
				node = Aabb(0.f, primitiveEnd, primitiveEnd);
				for (int i = bucketCount - 2; i >= 0; i--)
				{
					//use all but the last bucket
					node.sweepLeft(&*buckets[i]);
					metric[i] += node.sah(invSurfaceArea, leafTarget);
				}
				//make the split with the best metric:
				auto bestElement = std::min_element(metric.begin(), metric.end());
				//return best cut position:
				return buckets[std::distance(metric.begin(), bestElement)]->primitiveEnd;
			}
		}

		size_t getPrimCount()
		{
			return std::distance(primitiveBegin, primitiveEnd);
		}
	};

public:
	//edge with smallest value in each dimension
	glm::vec3 boundMin;
	glm::vec3 boundMax;
	//possible rotation??? -> since its just a bvh tester it doesnt really matter

	Aabb(unsigned int depth, primPointVector::iterator primitiveBegin, primPointVector::iterator primitiveEnd)
		: Node(depth, primitiveBegin, primitiveEnd)
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

	float getVolume() override
	{
		auto d = boundMax - boundMin;
		return d.x * d.y * d.z;
	}

	virtual void recursiveBvh(const unsigned int branchingFactor, const unsigned int leafTarget, int bucketCount)
	{
		//check primitive count. if less than x primitives, this node is finished. (pbrt would continue of leafcost is larger than split cost !!!)
		if (getPrimCount() <= leafTarget)
		{
			return;
		}

		//TODO: if i want to keep primitives in this node i have to spawn them to primitiveBegin BEFORE the sort
		//also dont forget to set primitiveBegin and primitiveEnd correctly before end of method


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
		sortAxis = maxDimension(centerDistance);

		//stop when triangle centers are at the same position (when this happens to often i might try the real center instead of the aabb center????
		//if (centerDistance[axis] <= 0.00002)
		//{
		//	return;
		//}


		//sort primitive array along axis:
		//its faster to first check if its sorted
		if (!std::is_sorted(primitiveBegin, primitiveEnd, std::bind(sortPrimitive, std::placeholders::_1, std::placeholders::_2, sortAxis)))
		{
			//TODO test what parallel stuff like std::execution::seq or unseqpar does here
			std::sort(primitiveBegin, primitiveEnd, std::bind(sortPrimitive, std::placeholders::_1, std::placeholders::_2, sortAxis));
		}


		std::vector<PrimIntervall> workIntervall;
		workIntervall.push_back(PrimIntervall(primitiveBegin, primitiveEnd));
		int bestI = 0;
		float primCounter = 0;

		for (size_t b = 0; b < branchingFactor - 1; b++)
		{
			primCounter = 0;
			for (size_t i = 0; i < workIntervall.size(); i++)
			{
				//choose the PrimIntervall with most primitives

				if (primCounter < workIntervall[i].getPrimCount())
				{
					primCounter = workIntervall[i].getPrimCount();
					bestI = i;
				}
			}
			if (primCounter <= leafTarget)
			{
				break;
			}
			auto bestSplit = workIntervall[bestI].computerBestSplit(1 / getSurfaceArea(), leafTarget, bucketCount);
			//split at bestSplit of best intervall:
			PrimIntervall p1(workIntervall[bestI].primitiveBegin, bestSplit);
			PrimIntervall p2(bestSplit, workIntervall[bestI].primitiveEnd);
			workIntervall[bestI] = p1;
			workIntervall.insert(workIntervall.begin() + bestI + 1, p2);
		}
		//create childNodes of every workIntervall;
		//Order has to be sorted!
		for (auto& i : workIntervall)
		{
			addNode(std::make_shared<Aabb>(depth + 1, i.primitiveBegin, i.primitiveEnd));
		}

		//just a debug output to get warned abount eventual loop
		if (depth >= 35)
		{
			std::cout << depth << std::endl;
		}

		primitiveBegin = primitiveEnd;

		//constructs bvh of all children:
		Node::recursiveBvh(branchingFactor, leafTarget, bucketCount);
	}

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
	void Node::sweepRight() override
	{
		primitiveEnd = primitiveEnd + 1;

		glm::vec3 minp, maxp;
		(*primitiveEnd)->getBounds(minp, maxp);
		boundMin = glm::min(boundMin, minp);
		boundMax = glm::max(boundMax, maxp);
	}
	void Node::sweepLeft()override
	{
		primitiveBegin = primitiveBegin - 1;

		glm::vec3 minp, maxp;
		(*primitiveBegin)->getBounds(minp, maxp);
		boundMin = glm::min(boundMin, minp);
		boundMax = glm::max(boundMax, maxp);
	}

	void Node::sweepRight(Node* n) override
	{
		Aabb* aabb = (Aabb*)n;
		primitiveEnd = aabb->primitiveEnd;

		boundMin = glm::min(boundMin, aabb->boundMin);
		boundMax = glm::max(boundMax, aabb->boundMax);
	}
	void Node::sweepLeft(Node* n) override
	{
		Aabb* aabb = (Aabb*)n;
		primitiveBegin = aabb->primitiveBegin;

		boundMin = glm::min(boundMin, aabb->boundMin);
		boundMax = glm::max(boundMax, aabb->boundMax);
	}
};
