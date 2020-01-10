#pragma once
#include <iostream>
#include <algorithm>
#include <vector>
#include <numeric>
#include <bitset>

#include "bvh.h"
#include "../timing.h"


class FastRay;

#define workGroupSquare (workGroupSize * workGroupSize)

template  <unsigned nodeMemory>
struct alignas(32) FastNode
{

	//aabbs
	std::array<float, nodeMemory * 6> bounds;

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
		: bounds(bounds), childType(childType)
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
	}
};

struct FastTriangle
{
	//transformed point coordinates. I
	//TODO: shoud try vec4 to get 16 align (and respective thing on ispc
	std::array<glm::vec4, 3> points = {};

	//pad to 64 byte
	//padding doesnt seem to change performance?
	uint32_t pad3[4];

	FastTriangle(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2)
	{
		points = { glm::vec4(p0,0),glm::vec4(p1,0) ,glm::vec4(p2,0) };
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
	void  intersectSecondaryWideAlternative(std::array<FastRay, workGroupSquare>& rays, std::array<uint8_t,
		workGroupSquare>& result, nanoSec& timeTriangleTest) const;

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

	inline int getLeafLoopCount(const uint32_t leafIndex) const
	{
		//return leafMemory
		int loopCount = leafMemory;
		if (isnan(trianglePoints[leafIndex + leafMemory * 9 - 1]))
		{
			loopCount = trianglePoints[leafIndex + leafMemory * 9 - 2];
		}
		return loopCount;
	}

	inline int getNodeLoopCount(const std::array<float, nodeMemory * 6>& bounds) const
	{
		//return nodeMemory;
		int loopCount = nodeMemory;
		if (isnan(bounds[nodeMemory * 6 - 1]))
		{
			loopCount = bounds[nodeMemory * 6 - 2];
		}
		return loopCount;
	}
};