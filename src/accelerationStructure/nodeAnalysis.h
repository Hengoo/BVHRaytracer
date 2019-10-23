#pragma once
#include <iostream>
#include <algorithm>
#include <vector>

#include "aabb.h"
#include "node.h"
#include "../color.h"
#include "../primitives/triangle.h"
#include "../glmUtil.h"

// this is to count bits in integers
//#include <intrin.h>

//class is only there to analyse bvhs
//it saves additional information about nodes that are not needed for raytracing, like parent
//can be discared when bvh analysis is finshed
class  NodeAnalysis
{

public:
	unsigned int depth;
	Node* node;
	NodeAnalysis* parent;
	std::vector<std::unique_ptr<NodeAnalysis>> children;
	unsigned int primitiveCount;

	unsigned int branchingFactor;
	unsigned int targetPrimitiveCount;
	float volume;
	float surfaceArea;
	float sah;
	//x -> -1 for non leafs, for leafs its the id
	int x;

	//node id
	int id;

	//just a copy of the node boundmin and boundmax -> its faster for the epo calculation since the data is more compact
	glm::vec3 boundMin;
	glm::vec3 boundMax;

	uint32_t allPrimitiveBeginId;
	uint32_t allPrimitiveEndId;

	NodeAnalysis(Node* node, int branchingFactor, int targetPrimitiveCount,
		primPointVector* primitives, float triangleCostFactor, float nodeCostFactor)
		:node(node), branchingFactor(branchingFactor), targetPrimitiveCount(targetPrimitiveCount)
	{
		parent = nullptr;
		depth = node->depth;
		primitiveCount = node->getPrimCount();
		volume = node->getVolume();
		surfaceArea = node->getSurfaceArea();
		float invSurfaceAreaRoot = 1 / surfaceArea;
		sah = 0;
		allPrimitiveBeginId = std::distance(primitives->begin(), node->allPrimitiveBegin);
		allPrimitiveEndId = std::distance(primitives->begin(), node->allPrimitiveEnd);
		auto aabb = static_cast<Aabb*>(node);
		boundMin = aabb->boundMin;
		boundMax = aabb->boundMax;
		for (auto& n : node->children)
		{
			children.push_back(std::make_unique<NodeAnalysis>(n.get(), this, branchingFactor, targetPrimitiveCount,
				primitives, invSurfaceAreaRoot, triangleCostFactor, nodeCostFactor));
		}
		x = -1;
	}
	NodeAnalysis(Node* node, NodeAnalysis* parent, int branchingFactor, int targetPrimitiveCount,
		primPointVector* primitives, float invSurfaceAreaRoot, float triangleCostFactor, float nodeCostFactor)
		:node(node), parent(parent), branchingFactor(branchingFactor), targetPrimitiveCount(targetPrimitiveCount)
	{
		depth = node->depth;
		primitiveCount = node->getPrimCount();
		volume = node->getVolume();
		surfaceArea = node->getSurfaceArea();

		auto aabb = static_cast<Aabb*>(node);
		boundMin = aabb->boundMin;
		boundMax = aabb->boundMax;

		//calc sah according to https://users.aalto.fi/~laines9/publications/aila2013hpg_paper.pdf
		//so its  surface area / surface area of root * cost of node
		//cost of node:
		//		for triangle: triCount * costFactorTri
		//		for node: costFactorNode
		if (primitiveCount == 0)
		{
			sah = surfaceArea * invSurfaceAreaRoot * nodeCostFactor;
		}
		else
		{
			sah = primitiveCount * surfaceArea * invSurfaceAreaRoot * triangleCostFactor;
		}
		allPrimitiveBeginId = std::distance(primitives->begin(), node->allPrimitiveBegin);
		allPrimitiveEndId = std::distance(primitives->begin(), node->allPrimitiveEnd);
		for (auto& n : node->children)
		{
			children.push_back(std::make_unique<NodeAnalysis>(n.get(), this, branchingFactor, targetPrimitiveCount,
				primitives, invSurfaceAreaRoot, triangleCostFactor, nodeCostFactor));
		}
		x = -1;
	}

	//traverses tree from left to right
	void analysis(std::vector<NodeAnalysis*>& leafNodes, std::vector<uint32_t>& treeDepth, std::vector<uint32_t>& childCount,
		std::vector<uint32_t>& primCount, float& allNodeSah)
	{
		size_t cc = children.size();
		size_t pc = primitiveCount;
		if (pc != 0 && cc == 0)
		{
			if (treeDepth.size() < depth + 1)
			{
				treeDepth.resize(depth + 1);
			}
			treeDepth[depth]++;
			leafNodes.push_back(this);
		}
		if (childCount.size() < cc + 1)
		{
			childCount.resize(cc + 1);
		}
		childCount[cc]++;
		if (primCount.size() < pc + 1)
		{
			primCount.resize(pc + 1);
		}
		primCount[pc]++;
		allNodeSah += sah;

		for (size_t i = 0; i < children.size(); i++)
		{
			children[i]->analysis(leafNodes, treeDepth, childCount, primCount, allNodeSah);
		}
	}

	//used to print bvh image
	void printLeaf(float& factor, std::vector<NodeAnalysis*>& parents, int& x, int& y)
	{
		parents.push_back(parent);
		y = depth;
		this->x = x;
		factor = 1 - primitiveCount / (float)targetPrimitiveCount;
	}

	//used to print bvh image
	bool printNode(float& factor, std::vector<NodeAnalysis*>& parents, int& x, int& y)
	{
		if (depth != 0)
		{
			parents.push_back(parent);
		}
		y = depth;
		x = 0;
		for (auto& c : children)
		{
			if (c->x == -1)
			{
				return false;
			}
			x += c->x;
		}
		x = x / children.size();
		this->x = x;
		factor = children.size() / (float)branchingFactor;
		return true;
	}
};