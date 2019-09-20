#pragma once
#include <iostream>
#include <algorithm>
#include <vector>

#include "node.h"
#include "../color.h"

//class is only there to analyse bvhs
//it saves additional information about nodes that are not needed for raytracing, like parent
//can be discared when bvh analysis is finshed
class  NodeAnalysis
{
public:
	unsigned int depth;
	NodeAnalysis* parent;
	std::vector<std::unique_ptr<NodeAnalysis>> children;
	unsigned int primitiveCount;

	unsigned int branchingFactor;
	unsigned int targetPrimitiveCount;
	int x;

	NodeAnalysis(Node* node, int branchingFactor, int targetPrimitiveCount)
		:branchingFactor(branchingFactor), targetPrimitiveCount(targetPrimitiveCount)
	{
		parent = nullptr;
		depth = node->depth;
		primitiveCount = node->getPrimCount();
		for (auto& n : node->children)
		{
			children.push_back(std::make_unique<NodeAnalysis>(&*n, this, branchingFactor, targetPrimitiveCount));
		}
		x = -1;
	}
	NodeAnalysis(Node* node, NodeAnalysis* parent, int branchingFactor, int targetPrimitiveCount)
		:parent(parent), branchingFactor(branchingFactor), targetPrimitiveCount(targetPrimitiveCount)
	{
		depth = node->depth;
		primitiveCount = node->getPrimCount();
		for (auto& n : node->children)
		{
			children.push_back(std::make_unique<NodeAnalysis>(&*n, this, branchingFactor, targetPrimitiveCount));
		}
		x = -1;
	}

	//traverses tree from left to right
	void analysis(std::vector<NodeAnalysis*>& leafNodes, std::vector<unsigned int>& treeDepth, std::vector<unsigned int>& childCount, std::vector<unsigned int>& primCount)
	{
		unsigned int cc = children.size();
		unsigned int pc = primitiveCount;
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
		for (size_t i = 0; i < children.size(); i++)
		{
			children[i]->analysis(leafNodes, treeDepth, childCount, primCount);
		}
	}

	void printLeaf(float& factor, std::vector<NodeAnalysis*>& parents, int& x, int& y)
	{
		parents.push_back(parent);
		y = depth;
		this->x = x;
		factor = 1 - primitiveCount / (float)targetPrimitiveCount;
	}

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