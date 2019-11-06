#pragma once
#include <iostream>
#include <algorithm>
#include <vector>
#include <numeric>

#include "bvh.h"
#include "../fastRay.h"

// Include the header file that the ispc compiler generates
#include "..\ISPC/ISPCBuild/rayTracer_ISPC.h"

struct FastNode
{
	//TODO padding and size improvements
	union
	{
		uint32_t childIdBegin;
		uint32_t primIdBegin;
	};
	union
	{
		uint8_t childCount;
		uint8_t primCount;
	};

	uint32_t boundsId;

	//TODO could put this bool inside something
	bool hasChildren;

	//sorting:
	//std::array<std::vector<int8_t>, 4> traverseOrderEachAxis;
	std::array<std::array<int8_t, 16>, 4> traverseOrderEachAxis;

	//array of bounds of children.
	//std::vector<glm::vec3> bounds;



	FastNode(uint32_t childIdBegin, uint32_t childCount, uint32_t primIdBegin, uint32_t primCount
		, uint32_t boundsId, std::array<std::vector<int8_t>, 4> traverseOrderEachAxis)
		:  boundsId(boundsId)
	{
		if (childCount != 0)
		{
			this->childIdBegin = childIdBegin;
			this->childCount = childCount;
			hasChildren = true;
		}
		else if (primCount != 0)
		{
			this->primIdBegin = primIdBegin;
			this->primCount = primCount;
			hasChildren = false;
		}
		else
		{
			std::cerr << "ERROR assigning both child and prim to fast compact node" << std::endl;
		}

		for (int j = 0; j < 4; j++)
		{
			for (int i = 0; i < traverseOrderEachAxis[0].size(); i++)
			{
				this->traverseOrderEachAxis[j][i] = traverseOrderEachAxis[j][i];
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


class FastNodeManager
{
	std::vector<FastNode> compactNodes;
	std::vector<FastTriangle> triangles;

	//(SoA order) -> first p0.x p0.y p0.z -> p1.x ....   (this for each triangle so its lieafsize * p0.x)
	std::vector<float> trianglePoints;

	//same soa order but for aabb
	std::vector<float> boundsSoA;

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
	int branchingFactor;
	int leafSize;
	bool intersect(FastRay& ray, double& timeTriangleTest);
	//copies a bvh and rearanges it into a single vector of compact nodes
	FastNodeManager(Bvh& bvh);

	//first add all children of node, then recusion for each child
	void customTreeOrder(NodeAnalysis* n, std::vector<NodeAnalysis*>& nodeVector);

	inline bool aabbCheck(FastRay& ray, glm::vec3& boundMin, glm::vec3& boundMax, float& distance);

	inline bool triangleCheck(FastRay& ray, uint32_t& id);

};