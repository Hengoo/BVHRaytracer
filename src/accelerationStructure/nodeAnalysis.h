#pragma once
#include <iostream>
#include <algorithm>
#include <vector>

#include "aabb.h"
#include "node.h"
#include "../color.h"
#include "../primitives/triangle.h"

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

	float traverseAnalysisBvh(Triangle* tri, glm::vec3 triMin, glm::vec3 triMax, float triSurfaceArea, uint16_t primId)
	{
		float epo = 0;

		//look if tri is part of this subtree:
		if (allPrimitiveBeginId <= primId && allPrimitiveEndId > primId)
		{
			for (auto& c : children)
			{
				epo += c->traverseAnalysisBvh(tri, triMin, triMax, triSurfaceArea, primId);
			}
		}
		else
		{
			auto aabb = dynamic_cast<Aabb*>(node);
			bool intersect = aabb->boundMin[0] <= triMin[0] && aabb->boundMax[0] >= triMax[0] &&
				aabb->boundMin[1] <= triMin[1] && aabb->boundMax[1] >= triMax[1] &&
				aabb->boundMin[2] <= triMin[2] && aabb->boundMax[2] >= triMax[2];

			if (intersect)
			{
				//no "collision" guaranteed
				bool allInside = true;
				//check if all vertices are inside: 
				if (allInside)
				{
					epo += triSurfaceArea;
				}
				else
				{
					//yeay triangle clipping -> this can be really complex 
					//-> search for some solution (need surface area of triangle inside aabb)
				}

				//locking to sum up surface area for each node.


				//only continue when it contributes area (this also removes triangles without surface)
				if (epo >= 0)
				{
					for (auto& c : children)
					{
						epo += c->traverseAnalysisBvh(tri, triMin, triMax, triSurfaceArea, primId);
					}
				}
			}
		}
		return epo;
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