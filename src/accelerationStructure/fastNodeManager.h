#pragma once
#include <iostream>
#include <algorithm>
#include <vector>
#include <numeric>
#include <bitset>

#include "bvh.h"
#include "../timing.h"

#include "../glmUtil.h"

//activates or deactives the timers for each ray
#define doTimer false;


class FastRay;

#define workGroupSquare (workGroupSize * workGroupSize)

//padding. padding of 1 means no padding, 2 doubles everything, and so on
#define nodeLeafPadding 1

template  <unsigned nodeMemory>
struct alignas(64) FastNode
{
	//aabbs
	std::array< float, nodeMemory * 6 * nodeLeafPadding> bounds;

	union
	{
		uint32_t childIdBegin;
		uint32_t primIdBegin;
	};

	//sorting
	std::array<std::array<int8_t, nodeMemory>, 3> traverseOrderEachAxis;

	//0 = leaf, 1 = node
	std::bitset<nodeMemory> childType;


	FastNode(uint32_t childIdBegin, uint32_t childCount, uint32_t primIdBegin, uint32_t primCount, std::array<float, nodeMemory * 6> bounds
		, std::array<std::vector<int8_t>, 3> traverseOrderEachAxis, std::bitset<nodeMemory> childType)
		: childType(childType)
	{
		if (childCount != 0)
		{
			this->childIdBegin = childIdBegin;
		}
		else if (primCount != 0)
		{
			this->primIdBegin = primIdBegin;
		}
		else
		{
			std::cerr << "ERROR assigning both child and prim to fast compact node" << std::endl;
		}

		for (int j = 0; j < 3; j++)
		{
			for (int i = 0; i < this->traverseOrderEachAxis[0].size(); i++)
			{
				if (traverseOrderEachAxis[0].size() > i)
				{
					this->traverseOrderEachAxis[j][i] = traverseOrderEachAxis[j][i];
				}
				else
				{
					//no childs anymore so fill with 0
					this->traverseOrderEachAxis[j][i] = 0;
				}
			}
		}
#if nodeLeafPadding != 1
		//fix bounds array
		for (int i = nodeMemory * 6 - 1; i >= 0; i--)
		{
			this->bounds[i * nodeLeafPadding] = bounds[i];
		}
#else
		this->bounds = bounds;
#endif
	}
};

template <unsigned nodeMemory, uint16_t workGroupSize >
struct RefillStructure
{
	//this struct stores everything needed to start a wide intersection.
	//the main purpose is to allow refilling the ray work list.

	std::array<FastRay, workGroupSquare> rays;
	//the pixel id of the rays in the "rays" array
	std::array<uint32_t, workGroupSquare> rayId;
	//result storage: triIndex is negative if nothing is hit
	std::array<uint32_t, workGroupSquare> leafIndex;
	std::array<int8_t, workGroupSquare> triIndex;

	nanoSec timeTriangleTest;

	//next nodeId for each ray. 40 is current max stack size. Negative id means leaf
	//id 0 is root node
	std::array< std::array<int32_t, workGroupSquare>, 40> stack;
	//id of the current element in the stack we have to work on. 0 means we are finished
	std::array< uint8_t, workGroupSquare>stackIndex;

	//ray id list to keep track of what rays we need to do.
	std::array<uint16_t, workGroupSquare> work1;
	std::array<uint16_t, workGroupSquare> work2;

	//precalculate the code for node traversal.
	//the first bit signales if its reverse or not, and the number resulting from (code >> 1) is the axis
	std::array<uint8_t, workGroupSquare > rayMajorAxis;

	//number of noderays and leafrays so we how much to read in nodeWork and leafWork	
	uint16_t nodeRays = workGroupSquare;
	uint16_t leafRays = 0;
	//same as above but for the next iteration.
	uint16_t nodeRaysNext = 0;
	uint16_t leafRaysNext = 0;

	std::array<uint16_t, workGroupSquare>* currentWork = &work1;
	std::array<uint16_t, workGroupSquare>* nextWork = &work2;

	RefillStructure()
	{
		triIndex.fill(-1);
		timeTriangleTest = nanoSec(0);
		stack[0].fill(0);
		stackIndex.fill(1);
		std::iota(work1.begin(), work1.end(), 0);
	}

	void fillRayMajorAxis()
	{
		for (int i = 0; i < workGroupSquare; i++)
		{
			uint8_t code = maxAbsDimension(rays[i].direction);
			if (rays[i].direction[code] <= 0)
			{
				code = (code << 1) + 1;
			}
			else
			{
				code = (code << 1);
			}
			rayMajorAxis[i] = code;
		}
	}
};

template <unsigned gangSize, unsigned nodeMemory, unsigned workGroupSize >
class FastNodeManager
{
	std::vector<FastNode<nodeMemory>>compactNodes;

	//(SoA order) -> first p0.x p0.y p0.z -> p1.x ....   (this for each triangle so its lieafsize * p0.x)
	std::vector<float> trianglePoints;

public:
	int leafSize;
	int leafMemory;
	int branchingFactor;
	bool intersectSaveDistance(FastRay& ray, uint32_t& leafIndex, uint8_t& triIndex, nanoSec& timeTriangleTest) const;
	bool intersect(FastRay& ray, uint32_t& leafIndex, uint8_t& triIndex, nanoSec& timeTriangleTest) const;
	bool intersectSecondary(FastRay& ray, nanoSec& timeTriangleTest) const;
	void intersectWide(std::array<FastRay, workGroupSquare>& rays, std::array<uint32_t, workGroupSquare>& leafIndex,
		std::array<int8_t, workGroupSquare>& triIndex, nanoSec& timeTriangleTest) const;
	void  intersectSecondaryWide(std::array<FastRay, workGroupSquare>& rays, std::array<uint8_t,
		workGroupSquare>& result, nanoSec& timeTriangleTest) const;

	void intersectWideAlternative(std::array<FastRay, workGroupSquare>& rays, std::array<uint32_t, workGroupSquare>& leafIndex,
		std::array<int8_t, workGroupSquare>& triIndex, nanoSec& timeTriangleTest) const;
	void intersectSecondaryWideAlternative(std::array<FastRay, workGroupSquare>& rays, std::array<uint8_t,
		workGroupSquare>& result, nanoSec& timeTriangleTest) const;

	void intersectRefillWideAlternative(RefillStructure<nodeMemory, workGroupSize>& data, bool lastRun) const;
	//void intersectRefillSecondaryWideAlternative(RefillStructure<nodeMemory, workGroupSquare>& data) const;

	float averageBvhDepth;
	uint32_t triangleCount;
	uint32_t nodeCount;

	//copies a bvh and rearanges it into a single vector of compact nodes
	FastNodeManager(Bvh& bvh, int leafMemory);

	//first add all children of node, then recusion for each child
	void customTreeOrder(NodeAnalysis* n, std::vector<NodeAnalysis*>& nodeVector);

	//calculates the surface normalof the triangle
	inline void getSurfaceNormalTri(const FastRay& ray, glm::vec3& surfaceNormal, const uint32_t leafIndex, const uint8_t triIndex) const;

	//calculates the surface normal and position
	inline void getSurfaceNormalPosition(const FastRay& ray, glm::vec3& surfaceNormal, glm::vec3& surfacePosition, const uint32_t leafIndex, const uint8_t triIndex) const;

	inline int getLeafLoopCount(const uint32_t leafIndex) const;

	inline int getNodeLoopCount(const std::array<float, nodeMemory * 6 * nodeLeafPadding>& bounds) const;

	inline void getTriPoints(glm::vec3& p0, glm::vec3& p1, glm::vec3& p2, const uint32_t leafIndex, const uint8_t triIndex) const;
};