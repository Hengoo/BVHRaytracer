#include "compactNode.h"
#include "nodeAnalysis.h"
#include "aabb.h"

template class CompactNodeManager<CompactNodeV0>;
template class CompactNodeManager<CompactNodeV1>;

template<typename T>
CompactNodeManager<T>::CompactNodeManager(Bvh bvh, int nodeOrder)
{
	//general appraoch: save all analysisNodes in a vector -> then write the id (the position in the vector)
	//Then we now the ids of all nodes and the ids of those children -> write them into compactNodesVector

	std::vector<NodeAnalysis*> nodeVector;
	//LevelOrder

	//custom order: (TODO: need to seach if it exists / what name it has?)
	//benefits it has: consecutive children + consecutive cachelines when the first child is tested

	switch (nodeOrder)
	{
	case 0:
		nodeVector.push_back(bvh.getAnalysisRoot());
		customTreeOrder(bvh.getAnalysisRoot(), nodeVector);
		break;
	default:
		break;
	}

	//set ids:
	for (size_t i = 0; i < nodeVector.size(); i++)
	{
		nodeVector[i]->id = i;
	}

	//create compact form depending on type:
	if constexpr (std::is_same<T, CompactNodeV0>::value)
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
			Aabb* aabb = static_cast<Aabb*>(n->node);
			compactNodes.push_back(T(childs, pBegin, pEnd, aabb->boundMin, aabb->boundMax, n->node->sortAxis));
		}
	}
	if constexpr (std::is_same<T, CompactNodeV1>::value)
	{
		std::cout << "compactNodeV1" << std::endl;
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
			Aabb* aabb = static_cast<Aabb*>(n->node);
			compactNodes.push_back(T(cBegin, cEnd, pBegin, pEnd, aabb->boundMin, aabb->boundMax, n->node->sortAxis));
		}
	}
	//int test = testTraverse();
	//int deb = 0;
	primitives = bvh.primitives;
}

template<typename T>
bool CompactNodeManager<T>::intersectImmediately(Ray& ray)
{
	//traverse compact node vector

	//check root node
	T node = compactNodes[0];
	if (!aabbCheck(ray, 0))
	{
		return false;
	}

	//ids of nodes that are already tested but didnt test the children jet:
	std::vector<size_t> queue;
	queue.reserve(50);
	queue.push_back(0);
	std::vector<size_t> depths;
	depths.reserve(50);
	depths.push_back(0);

	bool result = false;

	while (queue.size() != 0)
	{
		//get current id (most recently added because we do depth first)
		size_t id = queue.back();
		queue.pop_back();
		size_t d = depths.back();
		depths.pop_back();

		//check intersection with id
		node = compactNodes[id];

		//intersection happened, test primitives
		if (node.primIdBegin != node.primIdEnd)
		{
			int primCount = node.primIdEnd - node.primIdBegin;
			//logging: primitive fullness:
			ray.primitiveFullness[primCount] ++;
			ray.leafIntersectionCount[d]++;

			if (!ray.shadowRay)
			{
				std::for_each(primitives->begin() + node.primIdBegin, primitives->begin() + node.primIdEnd,
					[&](auto& p)
					{
						ray.primitiveIntersectionCount++;
						if (p->intersect(ray))
						{
							ray.successfulPrimitiveIntersectionCount++;
							result = true;
						}
					});
			}
			else
			{
				auto b = std::any_of(primitives->begin() + node.primIdBegin, primitives->begin() + node.primIdEnd,
					[&](auto& p)
					{
						ray.primitiveIntersectionCount++;
						if (p->intersect(ray))
						{
							ray.successfulPrimitiveIntersectionCount++;
							if (ray.shadowRay)
							{
								return true;
							}
						}
						return false;
					});
				if (b)
				{
					return true;
				}
			}
		}

		//test nodes (and add positive ones to queue)
		if (!node.noChildren())
		{
			//update counters
			int childCount = node.getChildCount();

			ray.childFullness[childCount] ++;
			ray.nodeIntersectionCount[d]++;

			auto children = node.getChildVector();
			if (ray.direction[node.sortAxis] > 0)
			{
				//need to check what order to read
				//int insertPoint = queue.size();
				std::for_each(std::execution::seq, children.rbegin(), children.rend(),
					[&](auto& i)
					{
						if (!aabbCheck(ray, i))
						{
							return;
						}
						queue.push_back(i);
						depths.push_back(d + 1);
						//queue.insert(queue.begin() + insertPoint, i);
						//depths.insert(depths.begin() + insertPoint, d + 1);
					});
			}
			else
			{
				std::for_each(std::execution::seq, children.begin(), children.end(),
					[&](auto& i)
					{
						if (!aabbCheck(ray, i))
						{
							return;
						}
						queue.push_back(i);
						depths.push_back(d + 1);
					});
			}
		}
	}
	return result;
}

template<typename T>
bool CompactNodeManager<T>::intersect(Ray& ray)
{
	//traverse compact node vector

	//most of the logging can also be done here, but we do not know the depth of

	//ids of ndodes that we still need to test:
	std::vector<size_t> queue;
	queue.reserve(20);
	queue.push_back(0);
	std::vector<size_t> depths;
	depths.reserve(20);
	depths.push_back(0);

	bool result = false;

	while (queue.size() != 0)
	{
		//get current id (most recently added because we do depth first)
		size_t id = queue.back();
		queue.pop_back();
		size_t d = depths.back();
		depths.pop_back();

		//check intersection with id
		T node = compactNodes[id];

		if (!aabbCheck(ray, id))
		{
			continue;
		}

		//intersection happened, test primitives
		if (node.primIdBegin != node.primIdEnd)
		{
			int primCount = node.primIdEnd - node.primIdBegin;
			//logging: primitive fullness:
			ray.primitiveFullness[primCount] ++;
			ray.leafIntersectionCount[d]++;
			if (!ray.shadowRay)
			{
				std::for_each(primitives->begin() + node.primIdBegin, primitives->begin() + node.primIdEnd,
					[&](auto& p)
					{
						ray.primitiveIntersectionCount++;
						if (p->intersect(ray))
						{
							ray.successfulPrimitiveIntersectionCount++;
							result = true;
						}
					});
			}
			else
			{
				auto b = std::any_of(primitives->begin() + node.primIdBegin, primitives->begin() + node.primIdEnd,
					[&](auto& p)
					{
						ray.primitiveIntersectionCount++;
						if (p->intersect(ray))
						{
							ray.successfulPrimitiveIntersectionCount++;
							if (ray.shadowRay)
							{
								return true;
							}
						}
						return false;
					});
				if (b)
				{
					return true;
				}
			}
		}

		//add nodes to todo list
		if (!node.noChildren())
		{
			//update child FUllness:
			int childCount = node.getChildCount();
			ray.childFullness[childCount + 1] ++;

			//increment node intersection counter
			ray.nodeIntersectionCount[d]++;

			auto children = node.getChildVector();
			if (ray.direction[node.sortAxis] > 0)
			{
				std::for_each(std::execution::seq, children.rbegin(), children.rend(),
					[&](auto& i)
					{
						queue.push_back(i);
						depths.push_back(d + 1);
					});
			}
			else
			{
				std::for_each(std::execution::seq, children.begin(), children.end(),
					[&](auto& i)
					{
						queue.push_back(i);
						depths.push_back(d + 1);
					});
			}
		}
	}
	return result;
}

template<typename T>
void CompactNodeManager<T>::customTreeOrder(NodeAnalysis* n, std::vector<NodeAnalysis*>& nodeVector)
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