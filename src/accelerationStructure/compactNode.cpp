#include "compactNode.h"
#include "nodeAnalysis.h"
#include "aabb.h"
#include <set>

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
			compactNodes.push_back(T(childs, pBegin, pEnd, aabb->boundMin, aabb->boundMax, n->node->sortAxis, n->depth));
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
				compactNodes.push_back(T(cBegin, cEnd, pBegin, pEnd, aabb->boundMin, aabb->boundMax, n->node->sortAxis, n->depth));
			}
			else if constexpr (std::is_same<T, CompactNodeV3>::value)
			{
				compactNodes.push_back(T(cBegin, cEnd, pBegin, pEnd, aabb->boundMin, aabb->boundMax, n->node->traverseOrderEachAxis, n->depth));
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
void CompactNodeManager<T>::intersectWide(std::vector<Ray>& rays, std::vector<uint32_t>& nodeWorkPerStep, std::vector<uint32_t>& leafWorkPerStep,
	std::vector<uint32_t>& uniqueNodesPerStep, std::vector<uint32_t>& uniqueLeafsPerStep, std::vector<uint32_t>& terminationsPerStep)
{
	//reserve vector size for performance. Most rays are finished after 100 steps.
	int reserveSize = 100;
	nodeWorkPerStep.reserve(reserveSize);
	leafWorkPerStep.reserve(reserveSize);
	uniqueNodesPerStep.reserve(reserveSize);
	uniqueLeafsPerStep.reserve(reserveSize);
	terminationsPerStep.reserve(reserveSize);


	if constexpr (std::is_same<T, CompactNodeV3>::value)
	{
		//this is basically the slow version of fastNodeManager intersectWide that collects extra data
		int wideSize = rays.size();

		//ray id list to keep track of what rays we need to do.
		std::vector<uint16_t> nodeWork(wideSize);
		std::vector<uint16_t> leafWork(wideSize);

		//stack for each ray. 48 is current max stack size
		std::vector<std::array<int32_t, 48>>stack(wideSize);
		std::vector< uint8_t>stackIndex(wideSize);

		for (int i = 0; i < wideSize; i++)
		{
			stack[i][0] = 0;
			stackIndex[i] = 1;
		}

		int counter = 0;
		if (!rays[0].shadowRay)
		{
			std::iota(nodeWork.begin(), nodeWork.end(), 0);
			counter = wideSize;

		}
		else
		{
			//check secondary rays and only traverse those where the primary ray hit something
			for (int i = 0; i < wideSize; i++)
			{
				if (!isnan(rays[i].pos.x))
				{
					nodeWork[counter++] = i;
				}
			}
		}



		//number of noderays and leafrays so we know what ids to read.
		uint16_t nodeRays = counter;
		uint16_t leafRays = 0;

		uint16_t nodeRaysNext = 0;
		uint16_t leafRaysNext = 0;

		std::set<uint16_t> uniqueNumbers;

		//state "before" we start loop
		terminationsPerStep.push_back(wideSize - counter);
		nodeWorkPerStep.push_back(nodeRays);
		uniqueNodesPerStep.push_back(1);
		leafWorkPerStep.push_back(0);
		uniqueLeafsPerStep.push_back(0);

		while (nodeRays != 0 || leafRays != 0)
		{
			int terminationsThisStep = 0;

			//collect data before node loop
			nodeWorkPerStep.push_back(nodeRays);
			uniqueNumbers.clear();

			//loop over nodes
			if (nodeRays != 0)
			{
				for (int i = 0; i < nodeRays; i++)
				{
					auto rayId = nodeWork[i];
					auto& ray = rays[rayId];
					auto nodeId = stack[rayId][--stackIndex[rayId]];
					T* node = &compactNodes[nodeId];
					uniqueNumbers.insert(nodeId);

					//test ray against NodeId and write result in correct array

					//update counters
					uint16_t childCount = node->getChildCount();
					ray.childFullness[childCount] ++;

					ray.nodeIntersectionCount[node->depth]++;

					//traverse nodes with children that can have arbitrary sorting
					int code = maxAbsDimension(ray.direction);
					bool reverse = ray.direction[code] <= 0;
					if (reverse)
					{
						std::for_each(std::execution::seq, node->traverseOrderEachAxis[code].begin(), node->traverseOrderEachAxis[code].end(),
							[&](auto& cId)
							{
								int nodeId = node->childIdBegin + cId;
								if (aabbCheck(ray, nodeId))
								{
									//test if hit is a node or leaf
									if (compactNodes[nodeId].hasPrimitive())
									{
										stack[rayId][stackIndex[rayId]++] = -(int32_t)(node->childIdBegin + cId);
									}
									else
									{
										stack[rayId][stackIndex[rayId]++] = (int32_t)node->childIdBegin + cId;
									}
								}
							});
					}
					else
					{
						std::for_each(std::execution::seq, node->traverseOrderEachAxis[code].rbegin(), node->traverseOrderEachAxis[code].rend(),
							[&](auto& cId)
							{
								int nodeId = node->childIdBegin + cId;
								if (aabbCheck(ray, nodeId))
								{
									//test if hit is a node or leaf
									if (compactNodes[nodeId].hasPrimitive())
									{
										stack[rayId][stackIndex[rayId]++] = -(int32_t)(node->childIdBegin + cId);
									}
									else
									{
										stack[rayId][stackIndex[rayId]++] = (int32_t)node->childIdBegin + cId;
									}
								}
							});
					}

					//depending on next element in stack. put this ray in node or leaf
					if (stackIndex[rayId] != 0)
					{
						if (stack[rayId][stackIndex[rayId] - 1] >= 0)
						{
							//node
							nodeWork[nodeRaysNext++] = rayId;
						}
						else
						{
							//leaf
							leafWork[leafRays++] = rayId;
						}
					}
					else
					{
						terminationsThisStep++;
					}
				}
				nodeRays = nodeRaysNext;
				nodeRaysNext = 0;
			}

			uniqueNodesPerStep.push_back(uniqueNumbers.size());
			uniqueNumbers.clear();

			//collect data before leaf loop
			leafWorkPerStep.push_back(leafRays);

			//loop over triangles
			if (leafRays != 0)
				//if (leafRays >= 8 || nodeRays <= 8)
			{
				for (int i = 0; i < leafRays; i++)
				{
					auto rayId = leafWork[i];
					auto& ray = rays[rayId];
					auto leafId = -stack[rayId][--stackIndex[rayId]];
					//get current id (most recently added because we do depth first)
					T* node = &compactNodes[leafId];
					//test ray against NodeId and write result in correct array
					uniqueNumbers.insert(leafId);

					uint32_t primCount = node->primIdEndOffset;
					//logging: primitive fullness:
					ray.primitiveFullness[primCount] ++;
					ray.leafIntersectionCount[node->depth]++;

					bool anyHit = false;
					//In theory i need to save the clostest distance and only apply it to the ray at the end?
					std::for_each(primitives->begin() + node->primIdBegin, primitives->begin() + node->primIdEndOffset + node->primIdBegin,
						[&](auto& p)
						{
							if (p->intersect(ray))
							{
								anyHit = true;
							}
						});

					if (ray.shadowRay && anyHit)
					{
						//Stop this ray if shadowray and it hit something.
						ray.tMax = NAN;
						terminationsThisStep++;
						continue;
					}
					//depending on next element in stack. put this ray in node or leaf
					if (stackIndex[rayId] != 0)
					{
						if (stack[rayId][stackIndex[rayId] - 1] >= 0)
						{
							//node
							nodeWork[nodeRays++] = rayId;
						}
						else
						{
							//leaf
							leafWork[leafRaysNext++] = rayId;
						}
					}
					else
					{
						terminationsThisStep++;
					}
				}
				leafRays = leafRaysNext;
				leafRaysNext = 0;
			}
			uniqueLeafsPerStep.push_back(uniqueNumbers.size());
			terminationsPerStep.push_back(terminationsThisStep);
		}
	}
	else
	{
		std::cerr << "unsupported node type for wideIntersect data collection" << std::endl;
	}
	return;
}

template<typename T>
void CompactNodeManager<T>::intersectWideAlternative(std::vector<Ray>& rays, std::vector<uint32_t>& nodeWorkPerStep, std::vector<uint32_t>& leafWorkPerStep,
	std::vector<uint32_t>& uniqueNodesPerStep, std::vector<uint32_t>& uniqueLeafsPerStep, std::vector<uint32_t>& terminationsPerStep)
{
	//reserve vector size for performance. Most rays are finished after 100 steps.
	int reserveSize = 100;
	nodeWorkPerStep.reserve(reserveSize);
	leafWorkPerStep.reserve(reserveSize);
	uniqueNodesPerStep.reserve(reserveSize);
	uniqueLeafsPerStep.reserve(reserveSize);
	terminationsPerStep.reserve(reserveSize);


	if constexpr (std::is_same<T, CompactNodeV3>::value)
	{
		//this is basically the slow version of fastNodeManager intersectWide that collects extra data
		int wideSize = rays.size();

		//ray id list to keep track of what rays we need to do.
		std::vector<uint16_t> work1(wideSize);
		std::vector<uint16_t> work2(wideSize);

		std::vector<uint16_t>* currentWork = &work1;
		std::vector<uint16_t>* nextWork = &work2;

		//stack for each ray. 48 is current max stack size
		std::vector<std::array<int32_t, 48>>stack(wideSize);
		std::vector< uint8_t>stackIndex(wideSize);

		for (int i = 0; i < wideSize; i++)
		{
			stack[i][0] = 0;
			stackIndex[i] = 1;
		}

		int counter = 0;
		if (!rays[0].shadowRay)
		{
			std::iota(work1.begin(), work1.end(), 0);
			counter = wideSize;

		}
		else
		{
			//check secondary rays and only traverse those where the primary ray hit something
			for (int i = 0; i < wideSize; i++)
			{
				if (!isnan(rays[i].pos.x))
				{
					work1[counter++] = i;
				}
			}
		}

		//number of noderays and leafrays so we know what ids to read.
		uint16_t nodeRays = counter;
		uint16_t leafRays = 0;

		uint16_t nodeRaysNext = 0;
		uint16_t leafRaysNext = 0;

		std::set<uint16_t> uniqueNumbers;

		//state "before" we start loop
		terminationsPerStep.push_back(wideSize - counter);
		nodeWorkPerStep.push_back(nodeRays);
		uniqueNodesPerStep.push_back(1);
		leafWorkPerStep.push_back(0);
		uniqueLeafsPerStep.push_back(0);

		while (nodeRays != 0 || leafRays != 0)
		{
			int terminationsThisStep = 0;

			//collect data before node loop
			nodeWorkPerStep.push_back(nodeRays);
			uniqueNumbers.clear();

			//loop over nodes
			if (nodeRays != 0)
			{
				for (int i = 0; i < nodeRays; i++)
				{
					auto rayId = (*currentWork)[i];
					auto& ray = rays[rayId];
					auto nodeId = stack[rayId][--stackIndex[rayId]];
					T* node = &compactNodes[nodeId];
					uniqueNumbers.insert(nodeId);

					//test ray against NodeId and write result in correct array

					//update counters
					uint16_t childCount = node->getChildCount();
					ray.childFullness[childCount] ++;

					ray.nodeIntersectionCount[node->depth]++;

					//traverse nodes with children that can have arbitrary sorting
					int code = maxAbsDimension(ray.direction);
					bool reverse = ray.direction[code] <= 0;
					if (reverse)
					{
						std::for_each(std::execution::seq, node->traverseOrderEachAxis[code].begin(), node->traverseOrderEachAxis[code].end(),
							[&](auto& cId)
							{
								int nodeId = node->childIdBegin + cId;
								if (aabbCheck(ray, nodeId))
								{
									//test if hit is a node or leaf
									if (compactNodes[nodeId].hasPrimitive())
									{
										stack[rayId][stackIndex[rayId]++] = -(int32_t)(node->childIdBegin + cId);
									}
									else
									{
										stack[rayId][stackIndex[rayId]++] = (int32_t)node->childIdBegin + cId;
									}
								}
							});
					}
					else
					{
						std::for_each(std::execution::seq, node->traverseOrderEachAxis[code].rbegin(), node->traverseOrderEachAxis[code].rend(),
							[&](auto& cId)
							{
								int nodeId = node->childIdBegin + cId;
								if (aabbCheck(ray, nodeId))
								{
									//test if hit is a node or leaf
									if (compactNodes[nodeId].hasPrimitive())
									{
										stack[rayId][stackIndex[rayId]++] = -(int32_t)(node->childIdBegin + cId);
									}
									else
									{
										stack[rayId][stackIndex[rayId]++] = (int32_t)node->childIdBegin + cId;
									}
								}
							});
					}

					//depending on next element in stack. put this ray in node or leaf
					if (stackIndex[rayId] != 0)
					{
						if (stack[rayId][stackIndex[rayId] - 1] >= 0)
						{
							//node
							(*nextWork)[nodeRaysNext++] = rayId;
						}
						else
						{
							//leaf
							(*nextWork)[wideSize - 1 - (leafRaysNext++)] = rayId;
						}
					}
					else
					{
						terminationsThisStep++;
					}
				}
			}

			uniqueNodesPerStep.push_back(uniqueNumbers.size());
			uniqueNumbers.clear();

			//collect data before leaf loop
			leafWorkPerStep.push_back(leafRays);

			//loop over triangles
			if (leafRays != 0)
				//if (leafRays >= 8 || nodeRays <= 8)
			{
				for (int i = 0; i < leafRays; i++)
				{
					auto rayId = (*currentWork)[wideSize - 1 - i];
					auto& ray = rays[rayId];

					auto leafId = -stack[rayId][--stackIndex[rayId]];
					//get current id (most recently added because we do depth first)
					T* node = &compactNodes[leafId];
					//test ray against NodeId and write result in correct array
					uniqueNumbers.insert(leafId);

					uint32_t primCount = node->primIdEndOffset;
					//logging: primitive fullness:
					ray.primitiveFullness[primCount] ++;
					ray.leafIntersectionCount[node->depth]++;

					bool anyHit = false;
					//In theory i need to save the clostest distance and only apply it to the ray at the end?
					std::for_each(primitives->begin() + node->primIdBegin, primitives->begin() + node->primIdEndOffset + node->primIdBegin,
						[&](auto& p)
						{
							if (p->intersect(ray))
							{
								anyHit = true;
							}
						});

					if (ray.shadowRay && anyHit)
					{
						//Stop this ray if shadowray and it hit something.
						ray.tMax = NAN;
						terminationsThisStep++;
						continue;
					}
					//depending on next element in stack. put this ray in node or leaf
					if (stackIndex[rayId] != 0)
					{
						if (stack[rayId][stackIndex[rayId] - 1] >= 0)
						{
							//node
							(*nextWork)[nodeRaysNext++] = rayId;
						}
						else
						{
							//leaf
							(*nextWork)[wideSize - 1 - (leafRaysNext++)] = rayId;
						}
					}
					else
					{
						terminationsThisStep++;
					}
				}
			}
			uniqueLeafsPerStep.push_back(uniqueNumbers.size());
			terminationsPerStep.push_back(terminationsThisStep);

			//prepare next loop:
			leafRays = leafRaysNext;
			leafRaysNext = 0;

			nodeRays = nodeRaysNext;
			nodeRaysNext = 0;

			std::swap(currentWork, nextWork);
		}
	}
	else
	{
		std::cerr << "unsupported node type for wideIntersect data collection" << std::endl;
	}
	return;
}


template<typename T>
bool CompactNodeManager<T>::intersectImmediately(Ray& ray, bool useDistance)
{
	//traverse compact node vector

	//check root node
	T* node = &compactNodes[0];

	//first root aabb test is not neccessary
	//if (!aabbCheck(ray, 0))
	//{
	//	return false;
	//}

	//ids of nodes that are already tested but didnt test the children jet:
	std::vector<uint32_t> queue;
	queue.reserve(50);
	queue.push_back(0);
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
			ray.leafIntersectionCount[node->depth]++;

			if (!ray.shadowRay)
			{
				std::for_each(primitives->begin() + node->primIdBegin, primitives->begin() + node->primIdEndOffset + node->primIdBegin,
					[&](auto& p)
					{
						if (p->intersect(ray))
						{
							result = true;
						}
					});
			}
			else
			{
				//version that stops after hitting first triangle. This is NOT how avx sse does it..
				/*
				auto b = std::any_of(primitives->begin() + node->primIdBegin, primitives->begin() + node->primIdEndOffset + node->primIdBegin,
					[&](auto& p)
					{
						if (p->intersect(ray))
						{
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
				}*/

				//correct version that loops over all triangles
				bool anyHit = false;
				//In theory i need to save the clostest distance and only apply it to the ray at the end?
				std::for_each(primitives->begin() + node->primIdBegin, primitives->begin() + node->primIdEndOffset + node->primIdBegin,
					[&](auto& p)
					{
						if (p->intersect(ray))
						{
							anyHit = true;
						}
					});

				if (ray.shadowRay && anyHit)
				{
					//Stop this ray if shadowray and it hit something.
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
			ray.nodeIntersectionCount[node->depth]++;

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
						distances.push_back(distance);
					}
				}
			}
			else if (std::is_same<T, CompactNodeV3>::value)
			{
				//traverse nodes with children that can have arbitrary sorting
				int code = maxAbsDimension(ray.direction);
				bool reverse = ray.direction[code] <= 0;
				if (reverse)
				{
					std::for_each(std::execution::seq, node->traverseOrderEachAxis[code].begin(), node->traverseOrderEachAxis[code].end(),
						[&](auto& cId)
						{
							if (aabbCheck(ray, node->childIdBegin + cId, distance))
							{
								queue.push_back(node->childIdBegin + cId);
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

	bool result = false;

	while (queue.size() != 0)
	{
		//get current id (most recently added because we do depth first)
		uint32_t id = queue.back();
		queue.pop_back();

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
			ray.leafIntersectionCount[node->depth]++;
			if (!ray.shadowRay)
			{
				std::for_each(primitives->begin() + node->primIdBegin, primitives->begin() + node->primIdEndOffset + node->primIdBegin,
					[&](auto& p)
					{
						if (p->intersect(ray))
						{
							result = true;
						}
					});
			}
			else
			{
				auto b = std::any_of(primitives->begin() + node->primIdBegin, primitives->begin() + node->primIdEndOffset + node->primIdBegin,
					[&](auto& p)
					{
						if (p->intersect(ray))
						{
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
			ray.nodeIntersectionCount[node->depth]++;

			if constexpr (std::is_same<T, CompactNodeV0>::value)
			{
				//increment node intersection counter
				if (ray.direction[node->sortAxis] > 0)
				{
					std::for_each(std::execution::seq, node->childrenIds.rbegin(), node->childrenIds.rend(),
						[&](auto& i)
						{
							queue.push_back(i);
						});
				}
				else
				{
					std::for_each(std::execution::seq, node->childrenIds.begin(), node->childrenIds.end(),
						[&](auto& i)
						{
							queue.push_back(i);
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
					}
				}
				else
				{
					for (uint32_t i = node->childIdBegin; i <= node->childIdEndOffset + node->childIdBegin; i++)
					{
						queue.push_back(i);
					}
				}
			}
			else if constexpr (std::is_same<T, CompactNodeV3>::value)
			{
				//traverse nodes with children that can have arbitrary sorting

				//new version that saves traverse order
				int code = maxAbsDimension(ray.direction);
				bool reverse = ray.direction[code] <= 0;
				if (reverse)
				{
					std::for_each(std::execution::seq, node->traverseOrderEachAxis[code].begin(), node->traverseOrderEachAxis[code].end(),
						[&](auto& cId)
						{
							queue.push_back(node->childIdBegin + cId);
						});
				}
				else
				{
					std::for_each(std::execution::seq, node->traverseOrderEachAxis[code].rbegin(), node->traverseOrderEachAxis[code].rend(),
						[&](auto& cId)
						{
							queue.push_back(node->childIdBegin + cId);
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