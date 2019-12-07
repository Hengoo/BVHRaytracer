#pragma once
#include <iostream>
#include <algorithm>
#include <vector>
#include <numeric>

#include "bvh.h"

class FastRay;

template  <size_t nodeMemory>
struct alignas(32) FastNode
{

	//aabbs
	std::array<float, nodeMemory * 6> bounds;

	//sorting
	std::array<std::array<int8_t, nodeMemory>, 4> traverseOrderEachAxis;

	//TODO padding and size improvements
	union
	{
		uint32_t childIdBegin;
		uint32_t primIdBegin;
	};



	//could put this bool inside something ?
	bool hasChildren;



	FastNode(uint32_t childIdBegin, uint32_t childCount, uint32_t primIdBegin, uint32_t primCount, std::array<float, nodeMemory * 6> bounds
		, std::array<std::vector<int8_t>, 4> traverseOrderEachAxis)
		: bounds(bounds)
	{
		if (childCount != 0)
		{
			this->childIdBegin = childIdBegin;
			hasChildren = true;
		}
		else if (primCount != 0)
		{
			this->primIdBegin = primIdBegin;
			hasChildren = false;
		}
		else
		{
			std::cerr << "ERROR assigning both child and prim to fast compact node" << std::endl;
		}

		for (int j = 0; j < 4; j++)
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

template <size_t gangSize, size_t nodeMemory>
class FastNodeManager
{
	std::vector<FastNode<nodeMemory>>compactNodes;

	//(SoA order) -> first p0.x p0.y p0.z -> p1.x ....   (this for each triangle so its lieafsize * p0.x)
	std::vector<float> trianglePoints;

	//padto is the number of elements to pad to
	inline void pad(int padTo, std::vector<float>& vector)
	{
		//lazy, could be faster but doesnt matter
		while (vector.size() % padTo != 0)
		{
			vector.push_back(0);
		}
	}

public:
	int leafSize;
	int leafMemory;
	int branchingFactor;
	bool intersect(FastRay& ray, double& timeTriangleTest) const;
	bool intersectWide(FastRay& ray, double& timeTriangleTest) const;
	bool intersectSecondary(FastRay& ray, double& timeTriangleTest) const;
	bool intersectSecondaryWide(FastRay& ray, double& timeTriangleTest) const;

	float averageBvhDepth;
	uint32_t triangleCount;
	uint32_t nodeCount;

	//copies a bvh and rearanges it into a single vector of compact nodes
	FastNodeManager(Bvh& bvh, int leafMemory);

	//first add all children of node, then recusion for each child
	void customTreeOrder(NodeAnalysis* n, std::vector<NodeAnalysis*>& nodeVector);

	//calculates the surface normalof the triangle
	inline void getSurfaceNormalTri(FastRay& ray, glm::vec3& surfaceNormal) const;
	//calculates the surface normal and position
	inline void getSurfaceNormalPosition(FastRay& ray, glm::vec3& surfaceNormal, glm::vec3& surfacePosition) const;
};