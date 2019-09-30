#pragma once
#include <iostream>
#include <algorithm>
#include <vector>
#include <numeric>

#include "bvh.h"
#include "../ray.h"

//compact node with random child ids
struct CompactNodeV0
{
	std::vector<uint32_t> childrenIds;
	uint32_t primIdBegin;
	uint8_t primIdEndOffset;
	uint8_t sortAxis;
	glm::vec3 boundMin;
	glm::vec3 boundMax;



	CompactNodeV0(std::vector<uint32_t> childrenIds, uint32_t primIdBegin, uint32_t primIdEnd, glm::vec3 boundMin, glm::vec3 boundMax, uint8_t sortAxis)
		:childrenIds(childrenIds), primIdBegin(primIdBegin), boundMin(boundMin), boundMax(boundMax), sortAxis(sortAxis)
	{
		primIdEndOffset = primIdEnd - primIdBegin;
	}

	inline uint16_t getChildCount()
	{
		return childrenIds.size();
	}

	inline bool noChildren()
	{
		return childrenIds.size() == 0;
	}
};

//compact node with consecutive child ids (use if possible)
struct CompactNodeV1
{
	//in theory the end part only needs to be really small (could be offset to begin) -> right now its not
	uint32_t childIdBegin;
	uint32_t primIdBegin;
	uint8_t childIdEndOffset;
	uint8_t primIdEndOffset;
	uint8_t sortAxis;
	glm::vec3 boundMin;
	glm::vec3 boundMax;

	CompactNodeV1(uint32_t childIdBegin, uint32_t childIdEnd, uint32_t primIdBegin, uint32_t primIdEnd, glm::vec3 boundMin, glm::vec3 boundMax, uint8_t sortAxis)
		: childIdBegin(childIdBegin), primIdBegin(primIdBegin), boundMin(boundMin), boundMax(boundMax), sortAxis(sortAxis)
	{
		childIdEndOffset = childIdEnd - childIdBegin;
		primIdEndOffset = primIdEnd - primIdBegin;
	}

	inline uint16_t getChildCount()
	{
		return childIdEndOffset + 1;
	}

	inline bool noChildren()
	{
		return childIdEndOffset == 0;
	}
};

//only for aabb!
template<typename T>
class CompactNodeManager
{
	//depending on the node order we can choose the consecutive V1 or have to use V0
	std::vector<T> compactNodes;

	std::shared_ptr<primPointVector> primitives;
public:

	//copies a bvh and rearanges it into a single vector of compact nodes
	CompactNodeManager(Bvh bvh, int nodeOrder);

	//compact node intersect with the same results as the bvh intersect
	bool intersect(Ray& ray);

	//similar to normal intersect but instantly tests all child aabbs immediately (instead of only testing the closest)
	bool intersectImmediately(Ray& ray);

	//first add all children of node, then rekusion for each child
	void customTreeOrder(NodeAnalysis* n, std::vector<NodeAnalysis*>& nodeVector);

	//for level order. add all children of depth (from left to right)
	void levelTreeOrder(NodeAnalysis* n, std::vector<NodeAnalysis*>& nodeVector, int depth);

	inline bool aabbCheck(Ray& ray, int id)// CompactNodeV1& node)
	{
		ray.aabbIntersectionCount++;
		//aabb intersection (same as in Aabb)
		//code modified from here : https://gamedev.stackexchange.com/questions/18436/most-efficient-aabb-vs-ray-collision-algorithms
		float t;

		glm::fvec3 t1 = (compactNodes[id].boundMin - ray.pos) * ray.invDirection;
		glm::fvec3 t2 = (compactNodes[id].boundMax - ray.pos) * ray.invDirection;
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
		//stop when current ray distance is closer than minimum possible distance of the aabb
		if (ray.tMax < tmin)
		{
			return false;
		}
		ray.successfulAabbIntersectionCount++;
		return true;
	}

	//debug method returns number of nodes that are connected to root
	int fullTraverse();
};