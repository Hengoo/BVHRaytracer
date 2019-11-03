#include "compactNode.h"
#include "nodeAnalysis.h"
#include "aabb.h"

template class CompactNodeManager<CompactNodeV0>;
template class CompactNodeManager<CompactNodeV1>;
template class CompactNodeManager<CompactNodeV2>;
template class CompactNodeManager<CompactNodeV3>;

template<typename T>
CompactNodeManager<T>::CompactNodeManager(Bvh bvh, int nodeOrder)
	:branchingFactor(bvh.branchingFactor)
{
	//general appraoch: save all analysisNodes in a vector -> then write the id (the position in the vector)
	//Then we now the ids of all nodes and the ids of those children -> write them into compactNodesVector

	//tmp node vector
	std::vector<NodeAnalysis*> nodeVector;
	//LevelOrder

	switch (nodeOrder)
	{
	case 0:
		//custom order: (TODO: need to seach if it exists / what name it has?)
		//benefits it has: consecutive children + consecutive cachelines when the first child is tested
		nodeVector.push_back(bvh.getAnalysisRoot());
		customTreeOrder(bvh.getAnalysisRoot(), nodeVector);
		break;
	case 1:
		//not the most efficient thing but that doesnt matter.
		for (size_t i = 0; i < bvh.bvhDepth; i++)
		{
			levelTreeOrder(bvh.getAnalysisRoot(), nodeVector, i);
		}
		break;
	case 2:
		depthFirstTreeOrder(bvh.getAnalysisRoot(), nodeVector);
		break;
	default:
		nodeVector.push_back(bvh.getAnalysisRoot());
		customTreeOrder(bvh.getAnalysisRoot(), nodeVector);
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
			std::vector<uint32_t> childs;
			uint32_t pBegin = 0;
			uint32_t pEnd = 0;

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
	else if constexpr (std::is_same<T, CompactNodeV1>::value || std::is_same<T, CompactNodeV2>::value || std::is_same<T, CompactNodeV3>::value)
	{
		for (auto& n : nodeVector)
		{
			//begin and end the same -> empty
			uint32_t cBegin = 0;
			uint32_t cEnd = 0;
			uint32_t pBegin = 0;
			uint32_t pEnd = 0;
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
			if constexpr (std::is_same<T, CompactNodeV1>::value || std::is_same<T, CompactNodeV2>::value)
			{
				compactNodes.push_back(T(cBegin, cEnd, pBegin, pEnd, aabb->boundMin, aabb->boundMax, n->node->sortAxis));
			}
			else if constexpr (std::is_same<T, CompactNodeV3>::value)
			{
				compactNodes.push_back(T(cBegin, cEnd, pBegin, pEnd, aabb->boundMin, aabb->boundMax, n->node->traverseOrderEachAxis));
			}
		}
	}
	primitives = bvh.primitives;

	//debug: test if compact nodes are fully traversable
	//std::cout << nodeVector.size() << std::endl;
	//std::cout << "Node count: " << compactNodes.size() << std::endl;
	//std::cout << "traverse count: " << fullTraverse() << std::endl;
	//std::cout << "primCount: " << primitives->size() << std::endl;
	//std::cout << sizeof(CompactNodeV0) << std::endl;
	//std::cout << sizeof(CompactNodeV1) << std::endl;
	//std::cout << sizeof(CompactNodeV2) << std::endl;
	//std::cout << sizeof(glm::vec3) << std::endl;
}

template<typename T>
bool CompactNodeManager<T>::intersectImmediately(Ray& ray, bool useDistance)
{
	//traverse compact node vector

	//check root node
	T* node = &compactNodes[0];
	if (!aabbCheck(ray, 0))
	{
		return false;
	}

	//ids of nodes that are already tested but didnt test the children jet:
	std::vector<uint32_t> queue;
	queue.reserve(50);
	queue.push_back(0);
	std::vector<uint8_t> depths;
	depths.reserve(50);
	depths.push_back(0);
	std::vector<float> distances;
	if (useDistance)
	{
		distances.reserve(50);
		distances.push_back(0);
	}

	bool result = false;

	while (queue.size() != 0)
	{
		//get current id (most recently added because we do depth first)
		uint32_t id = queue.back();
		queue.pop_back();
		uint8_t depth = depths.back();
		depths.pop_back();
		float distance = 0;
		if (useDistance)
		{
			distance = distances.back();
			distances.pop_back();
			if (distance > ray.tMax)
			{
				continue;
			}
		}

		//check intersection with id
		node = &compactNodes[id];

		//intersection happened, test primitives
		if (node->hasPrimitive())
		{
			uint32_t primCount = node->primIdEndOffset;
			//logging: primitive fullness:
			ray.primitiveFullness[primCount] ++;
			ray.leafIntersectionCount[depth]++;

			if (!ray.shadowRay)
			{
				std::for_each(primitives->begin() + node->primIdBegin, primitives->begin() + node->primIdEndOffset + node->primIdBegin,
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
				auto b = std::any_of(primitives->begin() + node->primIdBegin, primitives->begin() + node->primIdEndOffset + node->primIdBegin,
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
		if (node->hasChildren())
		{
			//update counters
			uint16_t childCount = node->getChildCount();

			ray.childFullness[childCount] ++;
			ray.nodeIntersectionCount[depth]++;

			if constexpr (std::is_same<T, CompactNodeV0>::value)
			{
				if (ray.direction[node->sortAxis] > 0)
				{
					std::for_each(std::execution::seq, node->childrenIds.rbegin(), node->childrenIds.rend(),
						[&](auto& i)
						{
							if (!aabbCheck(ray, i, distance))
							{
								return;
							}
							queue.push_back(i);
							depths.push_back(depth + 1);
							distances.push_back(distance);
						});
				}
				else
				{
					std::for_each(std::execution::seq, node->childrenIds.begin(), node->childrenIds.end(),
						[&](auto& i)
						{
							if (!aabbCheck(ray, i, distance))
							{
								return;
							}
							queue.push_back(i);
							depths.push_back(depth + 1);
							distances.push_back(distance);
						});
				}
			}
			else if constexpr (std::is_same<T, CompactNodeV1>::value || std::is_same<T, CompactNodeV2>::value)
			{
				if (ray.direction[node->sortAxis] > 0)
				{
					//version in comments is slower
					//int insertPoint = queue.size();
					//for (uint32_t i = node->childIdBegin; i <= node->childIdEndOffset + node->childIdBegin; i++)
					for (uint32_t i = node->childIdEndOffset + node->childIdBegin; i >= node->childIdBegin; i--)
					{
						if (!aabbCheck(ray, i, distance))
						{
							continue;
						}
						//queue.insert(queue.begin() + insertPoint, i);
						//depths.insert(depths.begin() + insertPoint, depth + 1);

						queue.push_back(i);
						depths.push_back(depth + 1);
						distances.push_back(distance);
					}
				}
				else
				{
					for (uint32_t i = node->childIdBegin; i <= node->childIdEndOffset + node->childIdBegin; i++)
					{
						if (!aabbCheck(ray, i, distance))
						{
							continue;
						}
						queue.push_back(i);
						depths.push_back(depth + 1);
						distances.push_back(distance);
					}
				}
			}
			else if (std::is_same<T, CompactNodeV3>::value)
			{
				//traverse nodes with children that can have arbitrary sorting
				//new version that saves traverse order
				uint8_t code = 0;
				code = code | (ray.direction[0] <= 0);
				code = code | ((ray.direction[1] <= 0) << 1);
				bool reverse = ray.direction[2] <= 0;
				if (reverse)
				{
					code = code ^ 3;
					std::for_each(std::execution::seq, node->traverseOrderEachAxis[code].begin(), node->traverseOrderEachAxis[code].end(),
						[&](auto& cId)
						{
							if (aabbCheck(ray, node->childIdBegin + cId, distance))
							{
								queue.push_back(node->childIdBegin + cId);
								depths.push_back(depth + 1);
								distances.push_back(distance);
							}
						});
				}
				else
				{
					std::for_each(std::execution::seq, node->traverseOrderEachAxis[code].rbegin(), node->traverseOrderEachAxis[code].rend(),
						[&](auto& cId)
						{
							if (aabbCheck(ray, node->childIdBegin + cId, distance))
							{
								queue.push_back(node->childIdBegin + cId);
								depths.push_back(depth + 1);
								distances.push_back(distance);
							}
						});
				}
			}
		}
	}
	return result;
}

template<typename T>
bool CompactNodeManager<T>::intersect(Ray& ray)
{
	//traverse compact node vector

	//ids of ndodes that we still need to test:
	std::vector<uint32_t> queue;
	queue.reserve(20);
	queue.push_back(0);
	std::vector<uint8_t> depths;
	depths.reserve(20);
	depths.push_back(0);

	bool result = false;

	while (queue.size() != 0)
	{
		//get current id (most recently added because we do depth first)
		uint32_t id = queue.back();
		queue.pop_back();
		uint8_t depth = depths.back();
		depths.pop_back();

		//check intersection with id
		T* node = &compactNodes[id];

		if (!aabbCheck(ray, id))
		{
			continue;
		}

		//intersection happened, test primitives
		if (node->hasPrimitive())
		{
			uint16_t primCount = node->primIdEndOffset;
			//logging: primitive fullness:
			ray.primitiveFullness[primCount] ++;
			ray.leafIntersectionCount[depth]++;
			if (!ray.shadowRay)
			{
				std::for_each(primitives->begin() + node->primIdBegin, primitives->begin() + node->primIdEndOffset + node->primIdBegin,
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
				auto b = std::any_of(primitives->begin() + node->primIdBegin, primitives->begin() + node->primIdEndOffset + node->primIdBegin,
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
		if (node->hasChildren())
		{
			//update child Fullness:
			uint16_t childCount = node->getChildCount();
			ray.childFullness[childCount] ++;
			ray.nodeIntersectionCount[depth]++;

			if constexpr (std::is_same<T, CompactNodeV0>::value)
			{
				//increment node intersection counter
				if (ray.direction[node->sortAxis] > 0)
				{
					std::for_each(std::execution::seq, node->childrenIds.rbegin(), node->childrenIds.rend(),
						[&](auto& i)
						{
							queue.push_back(i);
							depths.push_back(depth + 1);
						});
				}
				else
				{
					std::for_each(std::execution::seq, node->childrenIds.begin(), node->childrenIds.end(),
						[&](auto& i)
						{
							queue.push_back(i);
							depths.push_back(depth + 1);
						});
				}
			}
			else if constexpr (std::is_same<T, CompactNodeV1>::value || std::is_same<T, CompactNodeV2>::value)
			{
				if (ray.direction[node->sortAxis] > 0)
				{
					for (uint32_t i = node->childIdEndOffset + node->childIdBegin; i >= node->childIdBegin; i--)
					{
						queue.push_back(i);
						depths.push_back(depth + 1);
					}
				}
				else
				{
					for (uint32_t i = node->childIdBegin; i <= node->childIdEndOffset + node->childIdBegin; i++)
					{
						queue.push_back(i);
						depths.push_back(depth + 1);
					}
				}
			}
			else if constexpr (std::is_same<T, CompactNodeV3>::value)
			{
				//traverse nodes with children that can have arbitrary sorting

				//new version that saves traverse order
				uint8_t code = 0;
				code = code | (ray.direction[0] <= 0);
				code = code | ((ray.direction[1] <= 0) << 1);
				bool reverse = ray.direction[2] <= 0;
				if (reverse)
				{
					code = code ^ 3;
					std::for_each(std::execution::seq, node->traverseOrderEachAxis[code].begin(), node->traverseOrderEachAxis[code].end(),
						[&](auto& cId)
						{
							queue.push_back(node->childIdBegin + cId);
							depths.push_back(depth + 1);
						});
				}
				else
				{
					std::for_each(std::execution::seq, node->traverseOrderEachAxis[code].rbegin(), node->traverseOrderEachAxis[code].rend(),
						[&](auto& cId)
						{
							queue.push_back(node->childIdBegin + cId);
							depths.push_back(depth + 1);
						});
				}
			}
		}
	}
	return result;
}

template<typename T>
int CompactNodeManager<T>::fullTraverse()
{
	int result = 0;

	std::vector<uint32_t> queue;
	queue.push_back(0);
	while (queue.size() != 0)
	{
		//get current id (most recently added because we do depth first)
		uint32_t id = queue.back();
		queue.pop_back();

		//check intersection with id
		T* node = &compactNodes[id];
		result++;

		if (node->hasChildren())
		{
			if constexpr (std::is_same<T, CompactNodeV0>::value)
			{

				std::for_each(std::execution::seq, node->childrenIds.begin(), node->childrenIds.end(),
					[&](auto& i)
					{
						queue.push_back(i);
					});

			}
			else if constexpr (std::is_same<T, CompactNodeV1>::value || std::is_same<T, CompactNodeV2>::value)
			{

				for (uint32_t i = node->childIdBegin; i <= node->childIdEndOffset + node->childIdBegin; i++)
				{
					queue.push_back(i);
				}
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
		nodeVector.push_back(c.get());
	}
	for (auto& c : n->children)
	{
		customTreeOrder(c.get(), nodeVector);
	}
}

template<typename T>
void CompactNodeManager<T>::depthFirstTreeOrder(NodeAnalysis* n, std::vector<NodeAnalysis*>& nodeVector)
{
	nodeVector.push_back(n);
	for (auto& c : n->children)
	{
		depthFirstTreeOrder(c.get(), nodeVector);
	}
}

template<typename T>
void CompactNodeManager<T>::levelTreeOrder(NodeAnalysis* n, std::vector<NodeAnalysis*>& nodeVector, int depth)
{
	if (n->depth == depth)
	{
		nodeVector.push_back(n);
		//for (auto& c : n->children)
		//{
		//	nodeVector.push_back(c.get());
		//}
	}
	else
	{
		for (auto& c : n->children)
		{
			levelTreeOrder(c.get(), nodeVector, depth);
		}
	}
}