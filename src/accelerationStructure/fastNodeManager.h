#pragma once
#include <iostream>
#include <algorithm>
#include <vector>
#include <numeric>

#include "bvh.h"
#include "../fastRay.h"

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
		uint8_t childIdEndOffset;
		uint8_t primIdEndOffset;
	};

	//TODO could put this bool inside something
	bool hasChildren;

	//sorting:
	std::array<std::vector<int8_t>, 4> traverseOrderEachAxis;

	//array of bounds of children.
	std::vector<glm::vec3> bounds;

	FastNode(uint32_t childIdBegin, uint32_t childIdEnd, uint32_t primIdBegin, uint32_t primIdEnd, std::vector<glm::vec3> bounds, std::array<std::vector<int8_t>, 4> traverseOrderEachAxis)
		: traverseOrderEachAxis(traverseOrderEachAxis), bounds(bounds)
	{
		if (childIdBegin != childIdEnd)
		{
			this->childIdBegin = childIdBegin;
			childIdEndOffset = childIdEnd - childIdBegin;
			hasChildren = true;
		}
		else if (primIdBegin != primIdEnd)
		{
			this->primIdBegin = primIdBegin;
			primIdEndOffset = primIdEnd - primIdBegin;
			hasChildren = false;
		}
		else
		{
			std::cerr << "ERROR assigning both child and prim to fast compact node" << std::endl;
		}
	}

	inline uint16_t getChildCount()
	{
		return childIdEndOffset + 1;
	}
};

struct FastTriangle
{
	//transformed point coordinates
	std::array<glm::vec3, 3> points = {};

	//padding doesnt seem to change performance
	uint64_t pad0;
	uint64_t pad1;
	uint64_t pad2;

	FastTriangle(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2)
	{
		points = { p0,p1,p2 };
	}
};


class FastNodeManager
{
	std::vector<FastNode> compactNodes;
	std::vector<FastTriangle> triangles;
public:
	bool intersect(FastRay& ray);
	//copies a bvh and rearanges it into a single vector of compact nodes
	FastNodeManager(Bvh& bvh);

	//first add all children of node, then recusion for each child
	void customTreeOrder(NodeAnalysis* n, std::vector<NodeAnalysis*>& nodeVector);

	inline bool aabbCheck(FastRay& ray, glm::vec3& boundMin, glm::vec3& boundMax, float& distance);

	inline bool triangleCheck(FastRay& ray, uint32_t& id);

};