#include "fastNodeManager.h"
#include "nodeAnalysis.h"
#include "..\primitives\triangle.h"
#include "..\util.h"
#include "..\glmUtil.h"
#include <algorithm>

#include "..\timing.h"
#include "..\fastRay.h"

// Include the header file that the ispc compiler generates
#include "..\ISPC/ISPCBuild/rayTracer_ISPC.h"
using namespace::ispc;

//macros to tell cpp what template this class is used for.

//we need all version of gangsize, branchmemory, workGroupSize
#define macro1() macro2(8) macro2(16) macro2(32)

#define macro2(wGS) macro3(4 ,wGS) macro4(8, wGS)

//macro3 does gangisze 4
#define macro3(gS, wGS) macro5(gS, 4, wGS)\
macro5(gS, 8, wGS)\
macro5(gS, 12, wGS)\
macro5(gS, 16, wGS)\
//macro5(gS, 20, wGS)\
//macro5(gS, 24, wGS)\
//macro5(gS, 28, wGS)\
//macro5(gS, 32, wGS)\
//macro5(gS, 36, wGS)\
//macro5(gS, 40, wGS)\
//macro5(gS, 44, wGS)\
//macro5(gS, 48, wGS)\
//macro5(gS, 52, wGS)\
//macro5(gS, 56, wGS)\
//macro5(gS, 60, wGS)\
//macro5(gS, 64, wGS)\

//macro4 does gangsize 8
#define macro4(gS, wGS) macro5(gS, 8, wGS)\
macro5(gS, 16, wGS)\
//macro5(gS, 24, wGS)\
//macro5(gS, 32, wGS)\
//macro5(gS, 40, wGS)\
//macro5(gS, 48, wGS)\
//macro5(gS, 56, wGS)\
//macro5(gS, 64, wGS)

#define macro5(gS, mS, wGS) template class FastNodeManager<gS, mS, wGS>;

//its difficulter than expected to call the templated ispc functions
//current approach would be to generate some const exp if to call the correct method

//calls the function with a number behind, depending on the memoryName.
#define callIspcTemplate(functionName, memoryName, ...)								\
if constexpr (memoryName == 4 ) {ispcResult = functionName##4 (__VA_ARGS__);}		\
if constexpr (memoryName == 8 ) {ispcResult = functionName##8 (__VA_ARGS__);}		\
if constexpr (memoryName == 12) {ispcResult = functionName##12(__VA_ARGS__);}		\
if constexpr (memoryName == 16) {ispcResult = functionName##16(__VA_ARGS__);}		\

//switch might be better?
//calls the function with a number behind, depending on the memoryName.
#define callIspcTemplateNotConst(functionName, memoryName, ...)			\
if (memoryName == 4 ) {ispcResult = functionName##4 (__VA_ARGS__);}		\
if (memoryName == 8 ) {ispcResult = functionName##8 (__VA_ARGS__);}		\
if (memoryName == 12) {ispcResult = functionName##12(__VA_ARGS__);}		\
if (memoryName == 16) {ispcResult = functionName##16(__VA_ARGS__);}		\


macro1()

template <unsigned gangSize, unsigned nodeMemory, unsigned  workGroupSize>
void FastNodeManager<gangSize, nodeMemory, workGroupSize>::intersectWide(std::array<FastRay, workGroupSquare>& rays,
	std::array<uint32_t, workGroupSquare>& leafIndex, std::array<int8_t, workGroupSquare>& triIndex,
	nanoSec& timeTriangleTest) const
{
	//stack for each ray. 32 is current max stack size
	std::vector< std::array<int32_t, workGroupSquare>> stack(32);
	std::array< uint8_t, workGroupSquare>stackIndex;
	for (int i = 0; i < workGroupSquare; i++)
	{
		stack[0][i] = 0;
		stackIndex[i] = 1;
	}

	//ray id list to keep track of what rays we need to do.
	//TODO this could be uint8 for workGroupSize of 16 and smaller
	std::array<uint16_t, workGroupSquare> nodeWork;
	std::iota(nodeWork.begin(), nodeWork.end(), 0);
	std::array<uint16_t, workGroupSquare> leafWork;


	//number of noderays and leafrays so we know what ids to read.
	uint16_t nodeRays = workGroupSquare;
	uint16_t leafRays = 0;

	uint16_t nodeRaysNext = 0;
	uint16_t leafRaysNext = 0;

	//memory for aabb result
	std::array<float, nodeMemory> aabbDistances;
	aabbDistances.fill(-100000);

	while (nodeRays != 0 || leafRays != 0)
	{
		//loop over nodes
		if (nodeRays != 0)
		{
			for (int i = 0; i < nodeRays; i++)
			{
				auto rayId = nodeWork[i];
				auto& ray = rays[rayId];

				//get current id (most recently added because we do depth first)
				const FastNode<nodeMemory>* node = &compactNodes[stack[--stackIndex[rayId]][rayId]];
				//test ray against NodeId and write result in correct array

				bool ispcResult;
				int loopCount = nodeMemory;
				if (isnan(node->bounds[nodeMemory * 6 - 1]))
				{
					loopCount = node->bounds[nodeMemory * 6 - 2];
				}
				callIspcTemplateNotConst(aabbIntersect, loopCount, (float*)node->bounds.data(), (float*)aabbDistances.data(), reinterpret_cast<float*>(&ray));
				//callIspcTemplate(aabbIntersect, nodeMemory, (float*)node->bounds.data(), (float*)aabbDistances.data(), reinterpret_cast<float*>(&ray));
				if (ispcResult)
				{
					int code = maxAbsDimension(ray.direction);
					bool reverse = ray.direction[code] <= 0;
					if (reverse)
					{
						std::for_each(std::execution::seq, node->traverseOrderEachAxis[code].begin(), node->traverseOrderEachAxis[code].end(),
							[&](auto& cId)
							{
								if (aabbDistances[cId] != -100000)
								{
									if (node->childType[cId])
									{
										//next is node
										stack[stackIndex[rayId]++][rayId] = (int32_t)node->childIdBegin + cId;
									}
									else
									{
										//next is leaf
										stack[stackIndex[rayId]++][rayId] = -(int32_t)(node->childIdBegin + cId);
									}
									aabbDistances[cId] = -100000;
								}
							});
					}
					else
					{
						//reverse order
						std::for_each(std::execution::seq, node->traverseOrderEachAxis[code].rbegin(), node->traverseOrderEachAxis[code].rend(),
							[&](auto& cId)
							{
								if (aabbDistances[cId] != -100000)
								{
									if (node->childType[cId])
									{
										//next is node
										stack[stackIndex[rayId]++][rayId] = node->childIdBegin + cId;
									}
									else
									{
										//next is leaf
										stack[stackIndex[rayId]++][rayId] = -(int32_t)(node->childIdBegin + cId);
									}
									aabbDistances[cId] = -100000;
								}
							});
					}
				}
				//depending on next element in stack. put this ray in node or leaf
				if (stackIndex[rayId] != 0)
				{
					if (stack[stackIndex[rayId] - 1][rayId] >= 0)
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
			}
			nodeRays = nodeRaysNext;
			nodeRaysNext = 0;
		}

		//loop over triangles
		if (leafRays != 0)
			//if (leafRays >= 8 || nodeRays <= 8)
		{
			auto timeBeforeTriangleTest = getTime();

			for (int i = 0; i < leafRays; i++)
			{
				auto rayId = leafWork[i];
				auto& ray = rays[rayId];

				//get current id (most recently added because we do depth first)
				const FastNode<nodeMemory>* node = &compactNodes[-stack[--stackIndex[rayId]][rayId]];
				//test ray against NodeId and write result in correct array

				int ispcResult;
				int loopCount = leafMemory;
				if (isnan(trianglePoints[node->primIdBegin + leafMemory * 9 - 1]))
				{
					loopCount = trianglePoints[node->primIdBegin + leafMemory * 9 - 2];
				}
				callIspcTemplateNotConst(triIntersect, loopCount, trianglePoints.data(), node->primIdBegin, reinterpret_cast<float*>(&ray));
				//callIspcTemplateNotConst(triIntersect, leafMemory, trianglePoints.data(), node->primIdBegin, reinterpret_cast<float*>(&ray));
				//it returns the hit id -> -1 = no hit
				if (ispcResult != -1)
				{
					leafIndex[rayId] = node->primIdBegin;
					triIndex[rayId] = ispcResult;
				}

				//depending on next element in stack. put this ray in node or leaf
				if (stackIndex[rayId] != 0)
				{
					if (stack[stackIndex[rayId] - 1][rayId] >= 0)
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
			}
			timeTriangleTest += getTimeSpan(timeBeforeTriangleTest);
			leafRays = leafRaysNext;
			leafRaysNext = 0;
		}
	}
}

template <unsigned gangSize, unsigned nodeMemory, unsigned  workGroupSize>
void FastNodeManager<gangSize, nodeMemory, workGroupSize>::intersectSecondaryWide(
	std::array<FastRay, workGroupSquare>& rays, std::array<uint8_t, workGroupSquare>& result, nanoSec& timeTriangleTest) const
{
	//next nodeId for each ray. 32 is current max stack size. Negative id means leaf
	std::vector< std::array<int32_t, workGroupSquare>> stack(32);
	stack[0].fill(0);
	//id of the current element in the stack we have to work on. 0 means we are finished
	std::array< uint8_t, workGroupSquare>stackIndex;
	stackIndex.fill(1);

	//ray id list to keep track of what rays we need to do.
	//TODO this could be uint8 for workGroupSize of 16 and smaller
	std::array<uint16_t, workGroupSquare> nodeWork;
	std::iota(nodeWork.begin(), nodeWork.end(), 0);
	std::array<uint16_t, workGroupSquare> leafWork;

	//number of noderays and leafrays so we how much to read in nodeWork and leafWork
	uint16_t nodeRays = workGroupSquare;
	uint16_t leafRays = 0;

	//same as above but for the next iteration.
	uint16_t nodeRaysNext = 0;
	uint16_t leafRaysNext = 0;

	//memory for aabb result
	std::array<float, nodeMemory> aabbDistances;
	aabbDistances.fill(-100000);

	while (nodeRays != 0 || leafRays != 0)
	{
		//loop over nodes
		if (nodeRays != 0)
		{
			for (int i = 0; i < nodeRays; i++)
			{
				auto rayId = nodeWork[i];
				auto& ray = rays[rayId];

				//get current id (most recently added because we do depth first)
				const FastNode<nodeMemory>* node = &compactNodes[stack[--stackIndex[rayId]][rayId]];
				//test ray against NodeId and write result in correct array

				bool ispcResult;
				int loopCount = nodeMemory;
				if (isnan(node->bounds[nodeMemory * 6 - 1]))
				{
					loopCount = node->bounds[nodeMemory * 6 - 2];
				}
				callIspcTemplateNotConst(aabbIntersect, loopCount, (float*)node->bounds.data(), (float*)aabbDistances.data(), reinterpret_cast<float*>(&ray));
				//callIspcTemplate(aabbIntersect, nodeMemory, (float*)node->bounds.data(), (float*)aabbDistances.data(), reinterpret_cast<float*>(&ray));
				if (ispcResult)
				{
					//this loop is faster with constant (nodememory) than with branching factor for N4L4.
					for (int cId = 0; cId < nodeMemory; cId++)
					{
						if (aabbDistances[cId] != -100000)
						{
							if (node->childType[cId])
							{
								//next is node
								stack[stackIndex[rayId]++][rayId] = node->childIdBegin + cId;
							}
							else
							{
								//next is leaf
								stack[stackIndex[rayId]++][rayId] = -(int32_t)(node->childIdBegin + cId);
							}
							aabbDistances[cId] = -100000;
						}
					}
				}
				//depending on next element in stack. put this ray in node or leaf
				if (stackIndex[rayId] != 0)
				{
					if (stack[stackIndex[rayId] - 1][rayId] >= 0)
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
			}
			nodeRays = nodeRaysNext;
			nodeRaysNext = 0;
		}
		//loop over triangles
		if (leafRays != 0)
			//if (leafRays >= 8 || nodeRays <= 8)
		{
			auto timeBeforeTriangleTest = getTime();

			for (int i = 0; i < leafRays; i++)
			{
				auto rayId = leafWork[i];
				auto& ray = rays[rayId];

				//get current id (most recently added because we do depth first)
				const FastNode<nodeMemory>* node = &compactNodes[-stack[--stackIndex[rayId]][rayId]];
				//test ray against NodeId and write result in correct array

				bool ispcResult;
				int loopCount = leafMemory;
				if (isnan(trianglePoints[node->primIdBegin + leafMemory * 9 - 1]))
				{
					loopCount = trianglePoints[node->primIdBegin + leafMemory * 9 - 2];
				}
				callIspcTemplateNotConst(triAnyHit, loopCount, trianglePoints.data(), node->primIdBegin, reinterpret_cast<float*>(&ray));
				//callIspcTemplateNotConst(triAnyHit, leafMemory, trianglePoints.data(), node->primIdBegin, reinterpret_cast<float*>(&ray));
				//it returns the hit id -> -1 = no hit
				if (ispcResult)
				{
					result[rayId] ++;
					//this ray is finished. Cancel and dont add it to the stack
					continue;
				}

				//depending on next element in stack. put this ray in node or leaf
				if (stackIndex[rayId] != 0)
				{
					if (stack[stackIndex[rayId] - 1][rayId] >= 0)
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
			}
			timeTriangleTest += getTimeSpan(timeBeforeTriangleTest);
			leafRays = leafRaysNext;
			leafRaysNext = 0;
		}
	}
}

template <unsigned gangSize, unsigned nodeMemory, unsigned  workGroupSize>
bool FastNodeManager<gangSize, nodeMemory, workGroupSize>::intersectSaveDistance(FastRay& ray, uint32_t& leafIndex, uint8_t& triIndex, nanoSec& timeTriangleTest) const
{
	//ids of ndodes that we still need to test:
	std::array<std::tuple<uint32_t, float>, 32> nodeStack;
	nodeStack[0] = std::make_tuple<uint32_t, float>(0, 0);
	uint8_t stackIndex = 1;

	bool result = false;

	std::array<float, nodeMemory> aabbDistances;
	aabbDistances.fill(-100000);

	//precalculate the code for node traversal.
	int code = maxAbsDimension(ray.direction);
	bool reverse = ray.direction[code] <= 0;

	while (stackIndex != 0)
	{
		//get current id (most recently added because we do depth first)
		auto& tup = nodeStack[--stackIndex];
		if (std::get<1>(tup) > ray.tMax)
		{
			continue;
		}
		const FastNode<nodeMemory>* node = &compactNodes[std::get<0>(tup)];

		if (!node->hasChildren)
		{
			auto timeBeforeTriangleTest = getTime();

			int ispcResult;
			callIspcTemplateNotConst(triIntersect, leafMemory, trianglePoints.data(), node->primIdBegin, reinterpret_cast<float*>(&ray));
			//it returns the hit id -> -1 = no hit
			if (ispcResult != -1)
			{
				result = true;
				leafIndex = node->primIdBegin;
				triIndex = ispcResult;
			}
			timeTriangleTest += getTimeSpan(timeBeforeTriangleTest);
		}
		else
		{
			//child tests: 
			//ispc version:

			//call ispc method that returns an array of min distances. if minDist = 0 -> no hit otherwise hit.
			//go trough traverseOrder and add those childs with distance != 0 to queue (in right order)
			bool ispcResult;
			callIspcTemplate(aabbIntersect, nodeMemory, (float*)node->bounds.data(), (float*)aabbDistances.data(), reinterpret_cast<float*>(&ray));
			if (ispcResult)
			{
				if (reverse)
				{
					std::for_each(std::execution::seq, node->traverseOrderEachAxis[code].begin(), node->traverseOrderEachAxis[code].end(),
						[&](auto& cId)
						{
							if (aabbDistances[cId] != -100000)
							{
								nodeStack[stackIndex++] = std::make_tuple(node->childIdBegin + cId, aabbDistances[cId]);
								aabbDistances[cId] = -100000;
							}
						});
				}
				else
				{
					std::for_each(std::execution::seq, node->traverseOrderEachAxis[code].rbegin(), node->traverseOrderEachAxis[code].rend(),
						[&](auto& cId)
						{
							if (aabbDistances[cId] != -100000)
							{
								nodeStack[stackIndex++] = std::make_tuple(node->childIdBegin + cId, aabbDistances[cId]);
								aabbDistances[cId] = -100000;
							}
						});
				}
			}
		}
	}
	return result;
}

template <unsigned gangSize, unsigned nodeMemory, unsigned  workGroupSize>
bool FastNodeManager<gangSize, nodeMemory, workGroupSize>::intersect(FastRay& ray, uint32_t& leafIndex, uint8_t& triIndex, nanoSec& timeTriangleTest) const
{
	//ids of ndodes that we still need to test:
	std::array<uint32_t, 32> nodeStack;
	nodeStack[0] = 0;
	uint8_t stackIndex = 1;

	bool result = false;

	std::array<float, nodeMemory> aabbDistances;
	aabbDistances.fill(-100000);

	//precalculate the code for node traversal.
	int code = maxAbsDimension(ray.direction);
	bool reverse = ray.direction[code] <= 0;

	while (stackIndex != 0)
	{
		//get current id (most recently added because we do depth first)
		const FastNode<nodeMemory>* node = &compactNodes[nodeStack[--stackIndex]];

		if (!node->hasChildren)
		{
			auto timeBeforeTriangleTest = getTime();

			int ispcResult;
			callIspcTemplateNotConst(triIntersect, leafMemory, trianglePoints.data(), node->primIdBegin, reinterpret_cast<float*>(&ray));
			//it returns the hit id -> -1 = no hit
			if (ispcResult != -1)
			{
				result = true;
				leafIndex = node->primIdBegin;
				triIndex = ispcResult;
			}
			timeTriangleTest += getTimeSpan(timeBeforeTriangleTest);
		}
		else
		{
			//child tests: 
			//ispc version:

			//call ispc method that returns an array of min distances. if minDist = 0 -> no hit otherwise hit.
			//go trough traverseOrder and add those childs with distance != 0 to queue (in right order)
			bool ispcResult;
			callIspcTemplate(aabbIntersect, nodeMemory, (float*)node->bounds.data(), (float*)aabbDistances.data(), reinterpret_cast<float*>(&ray));
			if (ispcResult)
				//if (aabbIntersect((float*)node->bounds.data(), (float*)aabbDistances.data(), reinterpret_cast<float*>(&ray), nodeMemory, branchingFactor))
			{
				if (reverse)
				{
					std::for_each(std::execution::seq, node->traverseOrderEachAxis[code].begin(), node->traverseOrderEachAxis[code].end(),
						[&](auto& cId)
						{
							if (aabbDistances[cId] != -100000)
							{
								nodeStack[stackIndex++] = node->childIdBegin + cId;
								aabbDistances[cId] = -100000;
							}
						});
				}
				else
				{
					std::for_each(std::execution::seq, node->traverseOrderEachAxis[code].rbegin(), node->traverseOrderEachAxis[code].rend(),
						[&](auto& cId)
						{
							if (aabbDistances[cId] != -100000)
							{
								nodeStack[stackIndex++] = node->childIdBegin + cId;
								aabbDistances[cId] = -100000;
							}
						});
				}
			}
		}
	}
	return result;
}


template <unsigned gangSize, unsigned nodeMemory, unsigned  workGroupSize>
bool FastNodeManager<gangSize, nodeMemory, workGroupSize>::intersectSecondary(FastRay& ray, nanoSec& timeTriangleTest) const
{
	//bvh intersection for secondary hits. -> anyhit
	//differences: no return for triangles. No aabb distance stop (no saving of distance of aabb intersections since we stop after first hit)

	//ids of ndodes that we still need to test:
	std::array<uint32_t, 32> nodeStack;
	nodeStack[0] = 0;
	uint8_t stackIndex = 1;
	bool result = false;

	std::array<float, nodeMemory> aabbDistances;
	aabbDistances.fill(-100000);

	while (stackIndex != 0)
	{
		//get current id (most recently added because we do depth first)
		uint32_t id = nodeStack[--stackIndex];

		const FastNode<nodeMemory>* node = &compactNodes[id];

		if (!node->hasChildren)
		{
			auto timeBeforeTriangleTest = getTime();
			bool ispcResult;
			callIspcTemplateNotConst(triAnyHit, leafMemory, trianglePoints.data(), node->primIdBegin, reinterpret_cast<float*>(&ray));
			//it returns the hit id -> -1 = no hit
			if (ispcResult)
			{
				//we dont care about exact result for shadowrays (could do other ispc intersection for it
				timeTriangleTest += getTimeSpan(timeBeforeTriangleTest);
				return true;
			}
			timeTriangleTest += getTimeSpan(timeBeforeTriangleTest);
		}
		else
		{
			//child tests: 
			//ispc version:

			//call ispc method that returns an array of min distances. if minDist = 0 -> no hit otherwise hit.
			//go trough traverseOrder and add those childs with distance != 0 to queue (in right order)

			bool ispcResult;
			callIspcTemplate(aabbIntersect, nodeMemory, (float*)node->bounds.data(), (float*)aabbDistances.data(), reinterpret_cast<float*>(&ray));
			if (ispcResult)
				//if (aabbIntersect((float*)node->bounds.data(), (float*)aabbDistances.data(), reinterpret_cast<float*>(&ray), nodeMemory, branchingFactor))
			{
				for (int i = 0; i < nodeMemory; i++)
				{
					if (aabbDistances[i] != -100000)
					{
						nodeStack[stackIndex++] = node->childIdBegin + i;
						aabbDistances[i] = -100000;
					}
				}
			}
		}
	}
	return result;
}

template <unsigned gangSize, unsigned nodeMemory, unsigned  workGroupSize>
FastNodeManager<gangSize, nodeMemory, workGroupSize>::FastNodeManager(Bvh& bvh, int leafMemory)
	:leafMemory(leafMemory)
{

	//very similar to compactNodeManager

	//general appraoch: save all analysisNodes in a vector -> then write the id (the position in the vector)
	//Then we now the ids of all nodes and the ids of those children -> write them into compactNodesVector

	//tmp node vector
	std::vector<NodeAnalysis*> nodeVector;
	nodeVector.push_back(bvh.getAnalysisRoot());
	customTreeOrder(bvh.getAnalysisRoot(), nodeVector);


	leafSize = bvh.leafSize;
	branchingFactor = bvh.branchingFactor;

	//std::cerr << " " << leafSize << " " << leafMemory << " " << branchingFactor << " " << gangSize << " " << nodeMemory << " " << workGroupSize << std::endl;
	if (branchingFactor > nodeMemory)
	{
		std::cerr << "branching factor is larger than node memory!!" << std::endl;
		throw 10;
	}
	if (leafSize > leafMemory)
	{
		std::cerr << "leafsize is larger than leafmemory!!" << std::endl;
		throw 10;
	}

	//both pad options are only used for triangles!!!
	int padTo = 8;
	bool padAfter = false;
	//pads block inside soa to gangsize
	bool padInside = true;

	//set ids:
	for (size_t i = 0; i < nodeVector.size(); i++)
	{
		nodeVector[i]->id = i;
	}

	//reserve space for triangle and compact nodes:
	trianglePoints.reserve(bvh.totalLeafCount * leafMemory * 3 * 3);
	compactNodes.reserve(nodeVector.size());
	triangleCount = bvh.totalTriCount;
	nodeCount = nodeVector.size();
	averageBvhDepth = bvh.averageBvhDepth;

	for (auto& n : nodeVector)
	{
		//begin and end the same -> empty
		uint32_t cBegin = 0;
		uint32_t cCount = 0;
		uint32_t pBegin = 0;
		uint32_t pCount = 0;
		std::array<float, nodeMemory * 6> bounds = {};

		//types of the childs
		std::bitset<nodeMemory> childType;

		if (n->node->getChildCount() > 0)
		{
			cBegin = (*n->children.begin())->id;
			cCount = n->node->getChildCount();

			//now fill aabb (will fill it here and then just copy in in the constructor. performance isnt that important here)

			//soa order aabb
			for (int i = 0; i < 3; i++)
			{
				for (size_t j = 0; j < nodeMemory; j++)
				{
					if (j < n->children.size())
					{
						bounds[i * nodeMemory + j] = n->children[j]->boundMin[i];
					}
					else
					{
						bounds[i * nodeMemory + j] = 0;
						//approach to have the first float of empty nodes to NAN so we can check it in ispc to skip them
						if (j % gangSize == 0 && i == 0)
						{
							//bounds[i * nodeMemory + j] = NAN;
						}
					}
				}
			}
			for (int i = 0; i < 3; i++)
			{
				for (size_t j = 0; j < nodeMemory; j++)
				{
					if (j < n->children.size())
					{
						bounds[nodeMemory * 3 + i * nodeMemory + j] = n->children[j]->boundMax[i];
					}
					else
					{
						bounds[nodeMemory * 3 + i * nodeMemory + j] = 0;
					}
				}
			}
			if (((cCount - 1) / gangSize + 1) * gangSize != nodeMemory)
			{
				bounds[bounds.size() - 2] = ((cCount - 1) / gangSize + 1) * gangSize;
				bounds[bounds.size() - 1] = NAN;
			}
			for (size_t j = 0; j < n->children.size(); j++)
			{
				if (n->children[j]->node->getChildCount() == 0)
				{
					childType[j] = 0;
				}
				else
				{
					childType[j] = 1;
				}
			}
		}
		if (n->node->getPrimCount() > 0)
		{
			//if leaf fill the triangle array and get the right ids / length.
			pCount = n->node->getPrimCount();
			pBegin = trianglePoints.size();
			//idea is to restucture the floats inside the primitives
			//i want to have all x of p0, then all y of p0, then all z of p0 -> all x of p1 ...
			for (int i = 0; i < 3; i++)
			{
				std::for_each(std::execution::seq, n->node->primitiveBegin, n->node->primitiveEnd,
					[&](auto& p)
					{
						Triangle* tri = static_cast<Triangle*>(p.get());
						glm::vec3 p0, p1, p2;
						tri->getVertexPositions(p0, p1, p2);
						trianglePoints.push_back(p0[i]);
					});
				if (padInside)
					pad(leafMemory, trianglePoints);
			}
			for (int i = 0; i < 3; i++)
			{
				std::for_each(std::execution::seq, n->node->primitiveBegin, n->node->primitiveEnd,
					[&](auto& p)
					{
						Triangle* tri = static_cast<Triangle*>(p.get());
						glm::vec3 p0, p1, p2;
						tri->getVertexPositions(p0, p1, p2);
						trianglePoints.push_back(p1[i]);
					});
				if (padInside)
					pad(leafMemory, trianglePoints);
			}
			for (int i = 0; i < 3; i++)
			{
				std::for_each(std::execution::seq, n->node->primitiveBegin, n->node->primitiveEnd,
					[&](auto& p)
					{
						Triangle* tri = static_cast<Triangle*>(p.get());
						glm::vec3 p0, p1, p2;
						tri->getVertexPositions(p0, p1, p2);
						trianglePoints.push_back(p2[i]);
					});
				if (padInside)
					pad(leafMemory, trianglePoints);
			}
			if (padAfter)
				pad(padTo, trianglePoints);
			if (((pCount - 1) / gangSize + 1) * gangSize != leafMemory)
			{
				trianglePoints[pBegin + leafMemory * 9 - 2] = ((pCount - 1) / gangSize + 1) * gangSize;
				trianglePoints[pBegin + leafMemory * 9 - 1] = NAN;
			}
		}
		//Aabb* aabb = static_cast<Aabb*>(n->node);



		compactNodes.push_back(FastNode<nodeMemory>(cBegin, cCount, pBegin, pCount, bounds, n->node->traverseOrderEachAxis, childType));
	}
}

//first add all children of node, then recusion for each child
template <unsigned gangSize, unsigned nodeMemory, unsigned  workGroupSize>
void FastNodeManager<gangSize, nodeMemory, workGroupSize>::customTreeOrder(NodeAnalysis* n, std::vector<NodeAnalysis*>& nodeVector)
{
	//same as in nodeManager
	for (auto& c : n->children)
	{
		nodeVector.push_back(c.get());
	}
	for (auto& c : n->children)
	{
		customTreeOrder(c.get(), nodeVector);
	}
}

//calculates the surface normalof the triangle
template <unsigned gangSize, unsigned nodeMemory, unsigned  workGroupSize>
inline void FastNodeManager<gangSize, nodeMemory, workGroupSize>::getSurfaceNormalTri(const FastRay& ray, glm::vec3& surfaceNormal, const uint32_t leafIndex, const uint8_t triIndex) const
{
	//we need the 3 positions of the triangle
	glm::vec3 p0(trianglePoints[triIndex + leafIndex], trianglePoints[triIndex + leafIndex + leafMemory], trianglePoints[triIndex + leafIndex + leafMemory * 2]);
	glm::vec3 p1(trianglePoints[triIndex + leafIndex + leafMemory * 3], trianglePoints[triIndex + leafIndex + leafMemory * 4], trianglePoints[triIndex + leafIndex + leafMemory * 5]);
	glm::vec3 p2(trianglePoints[triIndex + leafIndex + leafMemory * 6], trianglePoints[triIndex + leafIndex + leafMemory * 7], trianglePoints[triIndex + leafIndex + leafMemory * 8]);
	surfaceNormal = computeNormal(p0, p1, p2);
}

//calculates the surface normal and position
template <unsigned gangSize, unsigned nodeMemory, unsigned  workGroupSize>
inline void FastNodeManager<gangSize, nodeMemory, workGroupSize>::getSurfaceNormalPosition(const FastRay& ray, glm::vec3& surfaceNormal, glm::vec3& surfacePosition, const uint32_t leafIndex, const uint8_t triIndex) const
{
	//we need the 3 positions of the triangle
	glm::vec3 p0(trianglePoints[triIndex + leafIndex], trianglePoints[triIndex + leafIndex + leafMemory], trianglePoints[triIndex + leafIndex + leafMemory * 2]);
	glm::vec3 p1(trianglePoints[triIndex + leafIndex + leafMemory * 3], trianglePoints[triIndex + leafIndex + leafMemory * 4], trianglePoints[triIndex + leafIndex + leafMemory * 5]);
	glm::vec3 p2(trianglePoints[triIndex + leafIndex + leafMemory * 6], trianglePoints[triIndex + leafIndex + leafMemory * 7], trianglePoints[triIndex + leafIndex + leafMemory * 8]);
	surfaceNormal = computeNormal(p0, p1, p2);

	//calculating t a bit more accurate. (im not sure why the calcualtion in ispc is that wrong?)
	float denom = glm::dot(surfaceNormal, ray.direction);
	glm::vec3 p0l0 = p0 - ray.pos;
	float t = glm::dot(p0l0, surfaceNormal) / denom;
	surfacePosition = ray.pos + ray.direction * t;
}