#pragma once
#include <iostream>
#include <algorithm>
#include <vector>

#include "bvh.h"

//compact node with random child ids
struct CompactNodeV0
{
	std::vector<size_t> childrenIds;
	size_t primIdBegin;
	size_t primIdEnd;
	CompactNodeV0(std::vector<size_t> childrenIds, size_t primIdBegin, size_t primIdEnd)
		:childrenIds(childrenIds), primIdBegin(primIdBegin), primIdEnd(primIdEnd)
	{
	}
};

//compact node with consecutive child ids (use if possible)
struct CompactNodeV1
{
	//in theory the end part only needs to be really small (could be offset to begin) -> right now its not
	size_t childIdBegin;
	size_t childIdEnd;
	size_t primIdBegin;
	size_t primIdEnd;

	CompactNodeV1(size_t childIdBegin, size_t childIdEnd, size_t primIdBegin, size_t primIdEnd)
		: childIdBegin(childIdBegin), childIdEnd(childIdEnd), primIdBegin(primIdBegin), primIdEnd(primIdEnd)
	{
	}
};

class CompactNodeManager
{
	int usedNodeType;
	//depending on the node order we can choose the consecutive V1 or have to use V0
	std::vector<CompactNodeV0> compactNodesV0;
	std::vector<CompactNodeV1> compactNodesV1;
public:

	//copies a bvh and rearanges it into a single vector of compact nodes
	CompactNodeManager(Bvh bvh)
	{
		//general appraoch: save all analysisNodes in a vector -> then write the id (the position in the vector)
		//Then we now the ids of all nodes and the ids of those children -> write them into compactNodesVector

		std::vector<NodeAnalysis*> nodeVector;
		//LevelOrder

		//custom order: (TODO: need to seach if it exists / what name it has?)
		//benefits it has: consecutive children + consecutive cachelines when the first child is tested
		usedNodeType = 1;
		nodeVector.push_back(bvh.getAnalysisRoot());
		customTreeOrder(bvh.getAnalysisRoot(), nodeVector);



		//set ids:
		for (size_t i = 0; i < nodeVector.size(); i++)
		{
			nodeVector[i]->id = i;
		}

		//create compact form:
		if (usedNodeType == 0)
		{
			for (auto& n : nodeVector)
			{
				//begin and end the same -> empty
				std::vector<size_t> childs;
				size_t pBegin = 0;
				size_t pEnd = 0;

				for (auto& c : n->children)
				{
					childs.push_back(c->id);
				}
				if (n->node->getPrimCount() > 0)
				{
					pBegin = std::distance(bvh.primitives->begin(), n->node->primitiveBegin);
					pEnd = std::distance(bvh.primitives->begin(), n->node->primitiveEnd);
				}
				compactNodesV0.push_back(CompactNodeV0(childs, pBegin, pEnd));
			}
		}
		if (usedNodeType == 1)
		{
			for (auto& n : nodeVector)
			{
				//begin and end the same -> empty
				size_t cBegin = 0;
				size_t cEnd = 0;
				size_t pBegin = 0;
				size_t pEnd = 0;
				if (n->node->getChildCount() > 0)
				{
					cBegin = (*n->children.begin())->id;
					cEnd = (*(n->children.end() - 1))->id;
				}
				if (n->node->getPrimCount() > 0)
				{
					pBegin = std::distance(bvh.primitives->begin(), n->node->primitiveBegin);
					pEnd = std::distance(bvh.primitives->begin(), n->node->primitiveEnd);
				}
				compactNodesV1.push_back(CompactNodeV1(cBegin, cEnd, pBegin, pEnd));
			}
		}
		int deb = 0;
	}

	//first first add all children of node, then rekusion for each child
	void customTreeOrder(NodeAnalysis* n, std::vector<NodeAnalysis*>& nodeVector)
	{
		for (auto& c : n->children)
		{
			nodeVector.push_back(&*c);
		}
		for (auto& c : n->children)
		{
			customTreeOrder(&*c, nodeVector);
		}
	}
};