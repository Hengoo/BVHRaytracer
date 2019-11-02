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

//dont like it static but better than code duplication
static inline uint8_t chooseAxisAndSort(primPointVector::iterator primitiveBegin, primPointVector::iterator primitiveEnd)
{
	//version to sort each intervall itself -> mixed result, (without implementing correct traversal)
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
	uint8_t sortAxis = maxDimension(centerDistance);

	//sort primitive array along axis:
	//its faster to first check if its sorted
	if (!std::is_sorted(primitiveBegin, primitiveEnd, std::bind(sortPrimitive, std::placeholders::_1, std::placeholders::_2, sortAxis)))
	{
		//TODO test what parallel stuff like std::execution::seq or unseqpar does here
		std::sort(primitiveBegin, primitiveEnd, std::bind(sortPrimitive, std::placeholders::_1, std::placeholders::_2, sortAxis));
	}
	return sortAxis;
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

		//compute sah of every possible split and
		primPointVector::iterator computerBestSplit(float invSurfaceArea, int leafTarget);
		primPointVector::iterator computerBestSplitSort(float invSurfaceArea, int leafTarget, int8_t& sortAxis);
		primPointVector::iterator computerBestSplit(float invSurfaceArea, int leafTarget,
			int bucketCount, glm::vec3& boundMin, glm::vec3& boundMax, uint16_t sortAxis);


		inline size_t getPrimCount()
		{
			return std::distance(primitiveBegin, primitiveEnd);
		}
	};
protected:
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

	inline void calculateBounds()
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

	inline float getSurfaceArea() override
	{
		auto d = boundMax - boundMin;
		return 2 * (d.x * d.y + d.x * d.z + d.y * d.z);
	}

	inline float getVolume() override
	{
		auto d = boundMax - boundMin;
		return d.x * d.y * d.z;
	}

	virtual void recursiveBvh(const unsigned branchingFactor, const unsigned leafTarget, const int bucketCount,const bool sortEachSplit);


	inline virtual bool intersectNode(Ray& ray, float& distance) override
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

	inline void Node::sweepRight() override
	{
		primitiveEnd = primitiveEnd + 1;

		glm::vec3 minp, maxp;
		(*primitiveEnd)->getBounds(minp, maxp);
		boundMin = glm::min(boundMin, minp);
		boundMax = glm::max(boundMax, maxp);
	}
	inline void Node::sweepLeft()override
	{
		primitiveBegin = primitiveBegin - 1;

		glm::vec3 minp, maxp;
		(*primitiveBegin)->getBounds(minp, maxp);
		boundMin = glm::min(boundMin, minp);
		boundMax = glm::max(boundMax, maxp);
	}

	inline void Node::sweepRight(Node* n) override
	{
		Aabb* aabb = static_cast<Aabb*>(n);
		primitiveEnd = aabb->primitiveEnd;

		boundMin = glm::min(boundMin, aabb->boundMin);
		boundMax = glm::max(boundMax, aabb->boundMax);
	}
	inline void Node::sweepLeft(Node* n) override
	{
		Aabb* aabb = static_cast<Aabb*>(n);
		primitiveBegin = aabb->primitiveBegin;

		boundMin = glm::min(boundMin, aabb->boundMin);
		boundMax = glm::max(boundMax, aabb->boundMax);
	}
};
