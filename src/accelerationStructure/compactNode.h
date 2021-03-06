#pragma once
#include <iostream>
#include <algorithm>
#include <vector>
#include <numeric>

#include "bvh.h"
#include "../ray.h"

/* possible nodes
struct Node32Bit
{
	glm::vec3 min1;
	glm::vec3 min2;

	uint16_t childIdBegin;
	uint16_t primIdBegin;
	uint8_t childIdEndOffset;
	uint8_t primIdEndOffset;
	uint8_t sortAxis;
};

struct Node64Bit
{
	glm::vec3 min1;
	glm::vec3 min2;
	glm::vec3 min3;
	glm::vec3 min4;

	uint32_t childIdBegin;
	uint32_t primIdBegin;
	uint8_t childIdEndOffset;
	uint8_t primIdEndOffset;
	uint8_t sortAxis;
	uint8_t pad1;
	uint16_t pad2;
};
*/
//compact node with random child ids
struct CompactNodeV0
{
	std::vector<uint32_t> childrenIds;
	uint32_t primIdBegin;
	uint8_t primIdEndOffset;
	uint8_t sortAxis;
	glm::vec3 boundMin;
	glm::vec3 boundMax;

	//just for data collection
	int depth;

	CompactNodeV0(std::vector<uint32_t> childrenIds, uint32_t primIdBegin,
		uint32_t primIdEnd, glm::vec3 boundMin, glm::vec3 boundMax, uint8_t sortAxis, int depth)
		:childrenIds(childrenIds), primIdBegin(primIdBegin),
		boundMin(boundMin), boundMax(boundMax), sortAxis(sortAxis), depth(depth)
	{
		primIdEndOffset = primIdEnd - primIdBegin;
	}

	inline uint16_t getChildCount()
	{
		return childrenIds.size();
	}

	inline bool hasChildren()
	{
		return childrenIds.size() != 0;
	}

	inline bool hasPrimitive()
	{
		return primIdEndOffset != 0;
	}
};

//compact node with consecutive child ids
struct CompactNodeV1
{
	uint32_t childIdBegin;
	uint32_t primIdBegin;
	uint8_t childIdEndOffset;
	uint8_t primIdEndOffset;
	uint8_t sortAxis;
	glm::vec3 boundMin;
	glm::vec3 boundMax;

	//just for data collection
	int depth;

	CompactNodeV1(uint32_t childIdBegin, uint32_t childIdEnd, uint32_t primIdBegin, uint32_t primIdEnd,
		glm::vec3 boundMin, glm::vec3 boundMax, uint8_t sortAxis, int depth)
		: childIdBegin(childIdBegin), primIdBegin(primIdBegin), boundMin(boundMin), boundMax(boundMax), sortAxis(sortAxis), depth(depth)
	{
		childIdEndOffset = childIdEnd - childIdBegin;
		primIdEndOffset = primIdEnd - primIdBegin;
	}

	inline uint16_t getChildCount()
	{
		return childIdEndOffset + 1;
	}

	inline bool hasChildren()
	{
		return childIdEndOffset != 0;
	}

	inline bool hasPrimitive()
	{
		return primIdEndOffset != 0;
	}
};

//even more compact node with consecutive child ids (can only support either childs or primitives)
struct CompactNodeV2
{
	union
	{
		uint32_t childIdBegin;
		uint32_t primIdBegin;
	};
	union
	{
		uint8_t childIdEndOffset;
		uint8_t primIdEndOffset;
	};
	uint8_t sortAxis;
	glm::vec3 boundMin;
	glm::vec3 boundMax;

	//just for data collection
	int depth;

	CompactNodeV2(uint32_t childIdBegin, uint32_t childIdEnd, uint32_t primIdBegin, uint32_t primIdEnd,
		glm::vec3 boundMin, glm::vec3 boundMax, uint8_t sortAxis, int depth)
		: boundMin(boundMin), boundMax(boundMax), depth(depth)
	{
		if (childIdBegin != childIdEnd)
		{
			this->childIdBegin = childIdBegin;
			childIdEndOffset = childIdEnd - childIdBegin;
			this->sortAxis = sortAxis;
		}
		else if (primIdBegin != primIdEnd)
		{
			this->primIdBegin = primIdBegin;
			primIdEndOffset = primIdEnd - primIdBegin;
			this->sortAxis = 16;
		}
		else
		{
			std::cerr << "ERROR assigning both child and prim to compact node v2" << std::endl;
		}
	}

	inline uint16_t getChildCount()
	{
		return childIdEndOffset + 1;
	}

	inline bool hasChildren()
	{
		return sortAxis != 16;
	}

	inline bool hasPrimitive()
	{
		return sortAxis == 16;
	}
};

//compact node with consecutive child ids and a sorting axis for each split
struct CompactNodeV3
{
	uint32_t childIdBegin;
	uint32_t primIdBegin;
	uint8_t childIdEndOffset;
	uint8_t primIdEndOffset;
	std::array<std::vector<int8_t>, 3> traverseOrderEachAxis;
	glm::vec3 boundMin;
	glm::vec3 boundMax;

	//just for data collection
	int depth;

	CompactNodeV3(uint32_t childIdBegin, uint32_t childIdEnd, uint32_t primIdBegin, uint32_t primIdEnd,
		glm::vec3 boundMin, glm::vec3 boundMax, std::array<std::vector<int8_t>, 3> traverseOrderEachAxis, int depth)
		: childIdBegin(childIdBegin), primIdBegin(primIdBegin), boundMin(boundMin),
		boundMax(boundMax), traverseOrderEachAxis(traverseOrderEachAxis), depth(depth)
	{
		childIdEndOffset = childIdEnd - childIdBegin;
		primIdEndOffset = primIdEnd - primIdBegin;
	}

	inline uint16_t getChildCount()
	{
		return childIdEndOffset + 1;
	}

	inline bool hasChildren()
	{
		return childIdEndOffset != 0;
	}

	inline bool hasPrimitive()
	{
		return primIdEndOffset != 0;
	}
};

//only for aabb!
template<typename T>
class CompactNodeManager
{
	//depending on the node order we can choose the consecutive V1 or have to use V0
	std::vector<T> compactNodes;
	std::shared_ptr<primPointVector> primitives;
	const unsigned branchingFactor;
public:

	//copies a bvh and rearanges it into a single vector of compact nodes
	CompactNodeManager(Bvh bvh, int nodeOrder);

	//compact node intersect with the same results as the bvh intersect
	bool intersect(Ray& ray);

	//similar to normal intersect but instantly tests all child aabbs immediately (instead of only testing the closest)
	bool intersectImmediately(Ray& ray, bool useDistance);

	void intersectWide(std::vector<Ray>& rays, std::vector<uint32_t>& nodeWorkPerStep, std::vector<uint32_t>& leafWorkPerStep,
		std::vector<uint32_t>& uniqueNodesPerStep, std::vector<uint32_t>& uniqueLeafsPerStep, std::vector<uint32_t>& terminationsPerStep);
	void intersectWideAlternative(std::vector<Ray>& rays, std::vector<uint32_t>& nodeWorkPerStep, std::vector<uint32_t>& leafWorkPerStep,
		std::vector<uint32_t>& uniqueNodesPerStep, std::vector<uint32_t>& uniqueLeafsPerStep, std::vector<uint32_t>& terminationsPerStep);


	//first add all children of node, then rekusion for each child
	void customTreeOrder(NodeAnalysis* n, std::vector<NodeAnalysis*>& nodeVector);

	//for level order. add all children of depth (from left to right)
	void levelTreeOrder(NodeAnalysis* n, std::vector<NodeAnalysis*>& nodeVector, int depth);

	void depthFirstTreeOrder(NodeAnalysis* n, std::vector<NodeAnalysis*>& nodeVector);

	//debug method returns number of nodes that are connected to root
	int fullTraverse();

	inline bool aabbCheck(Ray& ray, int id)
	{
		ray.aabbIntersectionCount++;
		//faster version -> from ISPC ratracer exmaple https://github.com/ispc/ispc/blob/master/examples/rt/rt_serial.cpp
		float t0 = 0, t1 = ray.tMax;
		glm::fvec3 tNear = (compactNodes[id].boundMin - ray.pos) * ray.invDirection;
		glm::fvec3 tFar = (compactNodes[id].boundMax - ray.pos) * ray.invDirection;
		if (tNear.x > tFar.x)
		{
			std::swap(tNear.x, tFar.x);
		}
		t0 = std::max(tNear.x, t0);
		t1 = std::min(tFar.x, t1);

		if (tNear.y > tFar.y)
		{
			std::swap(tNear.y, tFar.y);
		}
		t0 = std::max(tNear.y, t0);
		t1 = std::min(tFar.y, t1);

		if (tNear.z > tFar.z)
		{
			std::swap(tNear.z, tFar.z);
		}
		t0 = std::max(tNear.z, t0);
		t1 = std::min(tFar.z, t1);

		bool result = t0 <= t1;
		ray.successfulAabbIntersectionCount += result;
		return result;
	}

	inline bool aabbCheck(Ray& ray, int id, float& distance)
	{
		ray.aabbIntersectionCount++;

		//faster version -> from ISPC ratracer exmaple https://github.com/ispc/ispc/blob/master/examples/rt/rt_serial.cpp
		float t0 = 0, t1 = ray.tMax;
		glm::fvec3 tNear = (compactNodes[id].boundMin - ray.pos) * ray.invDirection;
		glm::fvec3 tFar = (compactNodes[id].boundMax - ray.pos) * ray.invDirection;
		if (tNear.x > tFar.x)
		{
			std::swap(tNear.x, tFar.x);
		}
		t0 = std::max(tNear.x, t0);
		t1 = std::min(tFar.x, t1);

		if (tNear.y > tFar.y)
		{
			std::swap(tNear.y, tFar.y);
		}
		t0 = std::max(tNear.y, t0);
		t1 = std::min(tFar.y, t1);

		if (tNear.z > tFar.z)
		{
			std::swap(tNear.z, tFar.z);
		}
		t0 = std::max(tNear.z, t0);
		t1 = std::min(tFar.z, t1);
		distance = t0;

		bool result = t0 <= t1;
		ray.successfulAabbIntersectionCount += result;
		return result;
	}
};