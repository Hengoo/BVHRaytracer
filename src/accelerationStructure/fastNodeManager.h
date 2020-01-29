#pragma once

#include "bvh.h"
#include "../timing.h"
#include "../glmUtil.h"
#include "cacheSimulator.h"

#include <iostream>
#include <algorithm>
#include <vector>
#include <array>
#include <numeric>
#include <bitset>

//activates or deactives the timers for each ray
#define doTimer false;

//if true: tests all leafs of the stack
#define doAllLeafs false

class FastRay;

#define workGroupSquare (workGroupSize * workGroupSize)

//padding. padding of 1 means no padding, 2 doubles everything, and so on
//In order for it to work the padding also has to be the same in the jinja script
#define nodeLeafPadding 1

template  <unsigned nodeMemory>
struct alignas(64) FastNode
{
	//aabbs
	std::array< float, nodeMemory * 6 * nodeLeafPadding> bounds;

	union
	{
		uint32_t childIdBegin;
		uint32_t primIdBegin;
	};

	//sorting
	std::array<std::array<int8_t, nodeMemory>, 3> traverseOrderEachAxis;

	//0 = leaf, 1 = node
	std::bitset<nodeMemory> childType;


	FastNode(uint32_t childIdBegin, uint32_t childCount, uint32_t primIdBegin, uint32_t primCount, std::array<float, nodeMemory * 6> bounds
		, std::array<std::vector<int8_t>, 3> traverseOrderEachAxis, std::bitset<nodeMemory> childType)
		: childType(childType)
	{
		if (childCount != 0)
		{
			this->childIdBegin = childIdBegin;
		}
		else if (primCount != 0)
		{
			this->primIdBegin = primIdBegin;
		}
		else
		{
			std::cerr << "ERROR assigning both child and prim to fast compact node" << std::endl;
		}

		for (int j = 0; j < 3; j++)
		{
			for (int i = 0; i < this->traverseOrderEachAxis[0].size(); i++)
			{
				if (traverseOrderEachAxis[0].size() > i)
				{
					this->traverseOrderEachAxis[j][i] = traverseOrderEachAxis[j][i];
				}
				else
				{
					//no childs anymore so fill with 0
					this->traverseOrderEachAxis[j][i] = 0;
				}
			}
		}
#if nodeLeafPadding != 1
		//fix bounds array
		for (int i = nodeMemory * 6 - 1; i >= 0; i--)
		{
			this->bounds[i * nodeLeafPadding] = bounds[i];
		}
#else
		this->bounds = bounds;
#endif
	}
};

template <unsigned nodeMemory, uint16_t workGroupSize>
struct RefillStructure
{
	//this struct stores everything needed to start a wide intersection.
	//the main purpose is to allow refilling the ray work list.

	std::array<FastRay, workGroupSquare> rays;
	//the pixel id of the rays in the "rays" array
	std::array<uint32_t, workGroupSquare> rayId;
	//result storage: triIndex is negative if nothing is hit
	std::array<uint32_t, workGroupSquare> leafIndex;
	std::array<int8_t, workGroupSquare> triIndex;

	nanoSec timeTriangleTest;

	//next nodeId for each ray. 40 is current max stack size. Negative id means leaf
	//id 0 is root node
	std::array< std::array<int32_t, workGroupSquare>, 40> stack;
	//id of the current element in the stack we have to work on. 0 means we are finished
	std::array< uint8_t, workGroupSquare>stackIndex;

	//ray id list to keep track of what rays we need to do.
	std::array<uint16_t, workGroupSquare> work1;
	std::array<uint16_t, workGroupSquare> work2;

	//precalculate the code for node traversal.
	//the first bit signales if its reverse or not, and the number resulting from (code >> 1) is the axis
	std::array<uint8_t, workGroupSquare > rayMajorAxis;

	//number of noderays and leafrays so we how much to read in nodeWork and leafWork	
	uint16_t nodeRays = workGroupSquare;
	uint16_t leafRays = 0;
	//same as above but for the next iteration.
	uint16_t nodeRaysNext = 0;
	uint16_t leafRaysNext = 0;

	std::array<uint16_t, workGroupSquare>* currentWork = &work1;
	std::array<uint16_t, workGroupSquare>* nextWork = &work2;

	RefillStructure()
	{
		triIndex.fill(-1);
		timeTriangleTest = nanoSec(0);
		stack[0].fill(0);
		stackIndex.fill(1);
		std::iota(work1.begin(), work1.end(), 0);
	}

	void fillRayMajorAxis()
	{
		for (int i = 0; i < workGroupSquare; i++)
		{
			uint8_t code = maxAbsDimension(rays[i].direction);
			if (rays[i].direction[code] <= 0)
			{
				code = (code << 1) + 1;
			}
			else
			{
				code = (code << 1);
			}
			rayMajorAxis[i] = code;
		}
	}
};

template <unsigned gangSize, unsigned nodeMemory, unsigned workGroupSize >
class FastNodeManager
{
	alignas(64) std::vector<FastNode<nodeMemory>>compactNodes;

	//(SoA order) -> first p0.x p0.y p0.z -> p1.x ....   (this for each triangle so its lieafsize * p0.x)
	std::vector<float> trianglePoints;

public:
	CacheSimulator cache;
	int leafSize;
	int leafMemory;
	int branchingFactor;

	template<bool doCache>
	bool intersectSaveDistance(FastRay& ray, uint32_t& leafIndex, uint8_t& triIndex, nanoSec& timeTriangleTest);
	template<bool doCache>
	bool intersect(FastRay& ray, uint32_t& leafIndex, uint8_t& triIndex, nanoSec& timeTriangleTest);
	template<bool doCache>
	bool intersectSecondary(FastRay& ray, nanoSec& timeTriangleTest);
	template<bool doCache>
	void intersectWide(std::array<FastRay, workGroupSquare>& rays, std::array<uint32_t, workGroupSquare>& leafIndex,
		std::array<int8_t, workGroupSquare>& triIndex, nanoSec& timeTriangleTest);
	template<bool doCache>
	void  intersectSecondaryWide(std::array<FastRay, workGroupSquare>& rays, std::array<uint16_t,
		workGroupSquare>& result, nanoSec& timeTriangleTest);
	template<bool doCache>
	void intersectWideAlternative(std::array<FastRay, workGroupSquare>& rays, std::array<uint32_t, workGroupSquare>& leafIndex,
		std::array<int8_t, workGroupSquare>& triIndex, nanoSec& timeTriangleTest);
	template<bool doCache>
	void intersectSecondaryWideAlternative(std::array<FastRay, workGroupSquare>& rays, std::array<uint16_t,
		workGroupSquare>& result, nanoSec& timeTriangleTest);

	template<bool doCache>
	void intersectRefillWideAlternative(RefillStructure<nodeMemory, workGroupSize>& data, bool lastRun);
	//void intersectRefillSecondaryWideAlternative(RefillStructure<nodeMemory, workGroupSquare>& data);

	float averageBvhDepth;
	uint32_t triangleCount;
	uint32_t nodeCount;

	//copies a bvh and rearanges it into a single vector of compact nodes
	FastNodeManager(Bvh& bvh, int leafMemory, CacheSimulator cache);

	//first add all children of node, then recusion for each child
	void customTreeOrder(NodeAnalysis* n, std::vector<NodeAnalysis*>& nodeVector);

	//calculates the surface normalof the triangle
	inline void getSurfaceNormalTri(const FastRay& ray, glm::vec3& surfaceNormal, const uint32_t leafIndex, const uint8_t triIndex);

	//calculates the surface normal and position
	inline void getSurfaceNormalPosition(const FastRay& ray, glm::vec3& surfaceNormal, glm::vec3& surfacePosition, const uint32_t leafIndex, const uint8_t triIndex);

	inline int getLeafLoopCount(const uint32_t leafIndex);

	inline int getNodeLoopCount(const std::array<float, nodeMemory * 6 * nodeLeafPadding>& bounds);

	inline void getTriPoints(glm::vec3& p0, glm::vec3& p1, glm::vec3& p2, const uint32_t leafIndex, const uint8_t triIndex);

	inline void nodeCacheLoad(void* pointer)
	{
		//we know the size of the node, so the begin pointer is enought
		int cacheLineCount = sizeof(FastNode<nodeMemory>) / 64;
		for (int i = 0; i < cacheLineCount; i++)
		{
			cache.loadHeap((pointerType)pointer + i * 64);
		}
	}
	inline void leafCacheLoad(void* leafPointer, void* trianglePointer)
	{
		//we know the size of the node, so the begin pointer is enought
		int cacheLineCount = sizeof(FastNode<nodeMemory>) / 64;
		for (int i = 0; i < cacheLineCount; i++)
		{
			cache.loadHeap((pointerType)leafPointer + i * 64);
		}
		cacheLineCount = (9 * 4 * leafMemory) / 64;
		for (int i = 0; i < cacheLineCount; i++)
		{
			cache.loadHeap((pointerType)trianglePointer + i * 64);
		}
	}
};

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


//macros to call ispc methods (it adds the number after the function name to call the code generated function)

//calls the function with a number behind, depending on the memoryName.
#define callIspcTemplate(functionName, memoryName, ...)								\
if constexpr (memoryName == 4 ) {ispcResult = functionName##4 (__VA_ARGS__);}		\
if constexpr (memoryName == 8 ) {ispcResult = functionName##8 (__VA_ARGS__);}		\
if constexpr (memoryName == 12) {ispcResult = functionName##12(__VA_ARGS__);}		\
if constexpr (memoryName == 16) {ispcResult = functionName##16(__VA_ARGS__);}		\	

//switch might be better?
//calls the function with a number behind, depending on the memoryName.
#define callIspcTemplateNotConst(functionName, memoryName, ...)					\
if (memoryName == 4 ) {ispcResult = functionName##4 (__VA_ARGS__);}				\
else if (memoryName == 8 ) {ispcResult = functionName##8 (__VA_ARGS__);}		\
else if (memoryName == 12) {ispcResult = functionName##12(__VA_ARGS__);}		\
else {ispcResult = functionName##16(__VA_ARGS__);}			\

template <unsigned gangSize, unsigned nodeMemory, unsigned workGroupSize>
template <bool doCache>
void FastNodeManager<gangSize, nodeMemory, workGroupSize>::intersectRefillWideAlternative(RefillStructure<nodeMemory, workGroupSize>& data, bool lastRun)
{
	//TODO doCache is not implemented for this method
	//TODO only does primary rays

	//do normal intersection until we only have 50% of our rays left. Then return to refill

	//memory for aabb result (NAN if no hit
	std::array<float, nodeMemory> aabbDistances;
	aabbDistances.fill(NAN);

	while (data.nodeRays != 0 || data.leafRays != 0)
	{
		//loop over nodes
		if (data.nodeRays != 0)
		{
			for (int i = 0; i < data.nodeRays; i++)
			{
				auto rayId = (*data.currentWork)[i];
				auto& ray = data.rays[rayId];

				//get current id (most recently added because we do depth first)
				const FastNode<nodeMemory>& node = compactNodes[data.stack[--data.stackIndex[rayId]][rayId]];
				//test ray against NodeId and write result in correct array

				bool ispcResult;
				int	loopCount = getNodeLoopCount(node.bounds);
				callIspcTemplateNotConst(aabbIntersect, loopCount, node.bounds.data(), aabbDistances.data(), reinterpret_cast<float*>(&ray));
				if constexpr (doCache)
					nodeCacheLoad((void*)&node);
				if (ispcResult)
				{
					if (data.rayMajorAxis[rayId] & 1)
					{
						std::for_each(std::execution::seq, node.traverseOrderEachAxis[data.rayMajorAxis[i] >> 1].begin(), node.traverseOrderEachAxis[data.rayMajorAxis[i] >> 1].end(),
							[&](auto& cId)
							{
								if (!isnan(aabbDistances[cId]))
								{
									if (node.childType[cId])
									{
										//next is node
										data.stack[data.stackIndex[rayId]++][rayId] = (int32_t)node.childIdBegin + cId;
									}
									else
									{
										//next is leaf
										data.stack[data.stackIndex[rayId]++][rayId] = -(int32_t)(node.childIdBegin + cId);
									}
									aabbDistances[cId] = NAN;
								}
							});
					}
					else
					{
						//reverse order
						std::for_each(std::execution::seq, node.traverseOrderEachAxis[data.rayMajorAxis[i] >> 1].rbegin(), node.traverseOrderEachAxis[data.rayMajorAxis[i] >> 1].rend(),
							[&](auto& cId)
							{
								if (!isnan(aabbDistances[cId]))
								{
									if (node.childType[cId])
									{
										//next is node
										data.stack[data.stackIndex[rayId]++][rayId] = node.childIdBegin + cId;
									}
									else
									{
										//next is leaf
										data.stack[data.stackIndex[rayId]++][rayId] = -(int32_t)(node.childIdBegin + cId);
									}
									aabbDistances[cId] = NAN;
								}
							});
					}
				}
				//depending on next element in stack. put this ray in node or leaf
				if (data.stackIndex[rayId] != 0)
				{
					if (data.stack[data.stackIndex[rayId] - 1][rayId] >= 0)
					{
						//node
						(*data.nextWork)[data.nodeRaysNext++] = rayId;
					}
					else
					{
						//leaf
						(*data.nextWork)[workGroupSquare - 1 - (data.leafRaysNext++)] = rayId;
					}
				}
				else
				{
					data.rays[rayId].tMax = NAN;
				}
			}

		}

		//loop over triangles
		if (data.leafRays != 0)
			//if (leafRays >= 8 || nodeRays <= 8)
		{
#if doTimer
			auto timeBeforeTriangleTest = getTime();
#endif

			for (int i = 0; i < data.leafRays; i++)
			{
				auto rayId = (*data.currentWork)[workGroupSquare - 1 - i];
				auto& ray = data.rays[rayId];

				//get current id (most recently added because we do depth first)
				const FastNode<nodeMemory>& node = compactNodes[-data.stack[--data.stackIndex[rayId]][rayId]];
				//test ray against NodeId and write result in correct array

				int ispcResult;
				int loopCount = getLeafLoopCount(node.primIdBegin);
				callIspcTemplateNotConst(triIntersect, loopCount, trianglePoints.data(), node.primIdBegin, reinterpret_cast<float*>(&ray));
				if constexpr (doCache)
					leafCacheLoad((void*)&node, (void*)&trianglePoints[node.primIdBegin]);
				//it returns the hit id -> -1 = no hit
				if (ispcResult != -1)
				{
					data.leafIndex[rayId] = node.primIdBegin;
					data.triIndex[rayId] = ispcResult;
				}

				//depending on next element in stack. put this ray in node or leaf
				if (data.stackIndex[rayId] != 0)
				{
					if (data.stack[data.stackIndex[rayId] - 1][rayId] >= 0)
					{
						//node
						(*data.nextWork)[data.nodeRaysNext++] = rayId;
					}
					else
					{
						//leaf
#if doAllLeafs
						i--;
#else
						(*data.nextWork)[workGroupSquare - 1 - (data.leafRaysNext++)] = rayId;
#endif
					}
				}
				else
				{
					data.rays[rayId].tMax = NAN;
				}
			}
#if doTimer
			timeTriangleTest += getTimeSpan(timeBeforeTriangleTest);
#endif
		}
		//prepare next loop:
		data.leafRays = data.leafRaysNext;
		data.leafRaysNext = 0;

		data.nodeRays = data.nodeRaysNext;
		data.nodeRaysNext = 0;

		std::swap(data.currentWork, data.nextWork);

		if (data.nodeRays + data.leafRays < workGroupSquare / 2)
		{
			if (!lastRun)
			{
				return;
			}
		}
	}
}

template <unsigned gangSize, unsigned nodeMemory, unsigned workGroupSize>
template <bool doCache>
void FastNodeManager<gangSize, nodeMemory, workGroupSize>::intersectWide(std::array<FastRay, workGroupSquare>& rays,
	std::array<uint32_t, workGroupSquare>& leafIndex, std::array<int8_t, workGroupSquare>& triIndex,
	nanoSec& timeTriangleTest)
{
	//stack for each ray. 40 is current max stack size
	std::array< std::array<int32_t, workGroupSquare>, 40> stack;
	std::array< uint8_t, workGroupSquare>stackIndex;

	stack[0].fill(0);
	stackIndex.fill(1);

	//ray id list to keep track of what rays we need to do.
	std::array<uint16_t, workGroupSquare> nodeWork;
	std::iota(nodeWork.begin(), nodeWork.end(), 0);
	std::array<uint16_t, workGroupSquare> leafWork;

	//precalculate the code for node traversal.
	//the first bit signales if its reverse or not, and the number resulting from (code >> 1) is the axis
	std::array<uint8_t, workGroupSquare > rayMajorAxis;
	for (int i = 0; i < workGroupSquare; i++)
	{
		uint8_t code = maxAbsDimension(rays[i].direction);
		if (rays[i].direction[code] <= 0)
		{
			code = (code << 1) + 1;
		}
		else
		{
			code = (code << 1);
		}
		rayMajorAxis[i] = code;
	}

	//number of noderays and leafrays so we know what ids to read.
	uint16_t nodeRays = workGroupSquare;
	uint16_t leafRays = 0;

	uint16_t nodeRaysNext = 0;
	uint16_t leafRaysNext = 0;

	//memory for aabb result
	std::array<float, nodeMemory> aabbDistances;
	aabbDistances.fill(NAN);

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
				const FastNode<nodeMemory>& node = compactNodes[stack[--stackIndex[rayId]][rayId]];
				//test ray against NodeId and write result in correct array

				bool ispcResult;
				int loopCount = getNodeLoopCount(node.bounds);
				callIspcTemplateNotConst(aabbIntersect, loopCount, node.bounds.data(), aabbDistances.data(), reinterpret_cast<float*>(&ray));
				if constexpr (doCache)
				{
					//cache.loadStack((void*)&nodeWork[i]);
					cache.loadStack((void*)&ray);
					//cache.loadStack((void*)&stackIndex[rayId]);

					cache.loadStack((void*)&stack[stackIndex[rayId] + 1]);

					nodeCacheLoad((void*)&node);
				}
				if (ispcResult)
				{
					if (rayMajorAxis[rayId] & 1)
					{
						std::for_each(std::execution::seq, node.traverseOrderEachAxis[rayMajorAxis[i] >> 1].begin(), node.traverseOrderEachAxis[rayMajorAxis[i] >> 1].end(),
							[&](auto& cId)
							{
								if (!isnan(aabbDistances[cId]))
								{
									if (node.childType[cId])
									{
										//next is node
										stack[stackIndex[rayId]++][rayId] = (int32_t)node.childIdBegin + cId;
									}
									else
									{
										//next is leaf
										stack[stackIndex[rayId]++][rayId] = -(int32_t)(node.childIdBegin + cId);
									}
									aabbDistances[cId] = NAN;
								}
							});
					}
					else
					{
						//reverse order
						std::for_each(std::execution::seq, node.traverseOrderEachAxis[rayMajorAxis[i] >> 1].rbegin(), node.traverseOrderEachAxis[rayMajorAxis[i] >> 1].rend(),
							[&](auto& cId)
							{
								if (!isnan(aabbDistances[cId]))
								{
									if (node.childType[cId])
									{
										//next is node
										stack[stackIndex[rayId]++][rayId] = node.childIdBegin + cId;
									}
									else
									{
										//next is leaf
										stack[stackIndex[rayId]++][rayId] = -(int32_t)(node.childIdBegin + cId);
									}
									aabbDistances[cId] = NAN;
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
#if doTimer
			auto timeBeforeTriangleTest = getTime();
#endif
			for (int i = 0; i < leafRays; i++)
			{
				auto rayId = leafWork[i];
				auto& ray = rays[rayId];

				//get current id (most recently added because we do depth first)
				const FastNode<nodeMemory>& node = compactNodes[-stack[--stackIndex[rayId]][rayId]];
				//test ray against NodeId and write result in correct array

				int ispcResult;
				int loopCount = getLeafLoopCount(node.primIdBegin);
				callIspcTemplateNotConst(triIntersect, loopCount, trianglePoints.data(), node.primIdBegin, reinterpret_cast<float*>(&ray));
				if constexpr (doCache)
				{
					//cache.loadStack((void*)&leafWork[i]);
					cache.loadStack((void*)&ray);
					//cache.loadStack((void*)&stackIndex[rayId]);
					cache.loadStack((void*)&stack[stackIndex[rayId] + 1]);

					leafCacheLoad((void*)&node, (void*)&trianglePoints[node.primIdBegin]);
				}
				//it returns the hit id -> -1 = no hit
				if (ispcResult != -1)
				{
					leafIndex[rayId] = node.primIdBegin;
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
#if doAllLeafs
						i--;
#else
						leafWork[leafRaysNext++] = rayId;
#endif
					}
				}
			}
#if doTimer
			timeTriangleTest += getTimeSpan(timeBeforeTriangleTest);
#endif
			leafRays = leafRaysNext;
			leafRaysNext = 0;
			}
	}
}

template <unsigned gangSize, unsigned nodeMemory, unsigned workGroupSize>
template <bool doCache>
void FastNodeManager<gangSize, nodeMemory, workGroupSize>::intersectSecondaryWide(
	std::array<FastRay, workGroupSquare>& rays, std::array<uint16_t, workGroupSquare>& result, nanoSec& timeTriangleTest)
{
	//next nodeId for each ray. 40 is current max stack size. Negative id means leaf
	//id 0 is root node
	std::array< std::array<int32_t, workGroupSquare>, 40> stack;
	//id of the current element in the stack we have to work on. 0 means we are finished
	std::array< uint8_t, workGroupSquare>stackIndex;

	stack[0].fill(0);
	stackIndex.fill(1);

	//ray id list to keep track of what rays we need to do.
	//this could be uint8 for workGroupSize of 16 and smaller
	std::array<uint16_t, workGroupSquare> nodeWork;
	std::array<uint16_t, workGroupSquare> leafWork;

	int counter = 0;
	for (int i = 0; i < workGroupSquare; i++)
	{
		if (!isnan(rays[i].pos.x))
		{
			nodeWork[counter++] = i;
		}
	}

	//number of noderays and leafrays so we how much to read in nodeWork and leafWork
	uint16_t nodeRays = counter;
	uint16_t leafRays = 0;

	//same as above but for the next iteration.
	uint16_t nodeRaysNext = 0;
	uint16_t leafRaysNext = 0;

	//memory for aabb result
	std::array<float, nodeMemory> aabbDistances;
	aabbDistances.fill(NAN);

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
				const FastNode<nodeMemory>& node = compactNodes[stack[--stackIndex[rayId]][rayId]];
				//test ray against NodeId and write result in correct array

				bool ispcResult;
				int loopCount = getNodeLoopCount(node.bounds);
				callIspcTemplateNotConst(aabbIntersect, loopCount, node.bounds.data(), aabbDistances.data(), reinterpret_cast<float*>(&ray));
				if constexpr (doCache)
				{
					//cache.loadStack((void*)&leafWork[i]);
					cache.loadStack((void*)&ray);
					//cache.loadStack((void*)&stackIndex[rayId]);
					cache.loadStack((void*)&stack[stackIndex[rayId] + 1]);

					nodeCacheLoad((void*)&node);
				}
				if (ispcResult)
				{
					//this loop is faster with constant (nodememory) than with branching factor for N4L4.
					for (int cId = 0; cId < nodeMemory; cId++)
					{
						if (!isnan(aabbDistances[cId]))
						{
							if (node.childType[cId])
							{
								//next is node
								stack[stackIndex[rayId]++][rayId] = node.childIdBegin + cId;
							}
							else
							{
								//next is leaf
								stack[stackIndex[rayId]++][rayId] = -(int32_t)(node.childIdBegin + cId);
							}
							aabbDistances[cId] = NAN;
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
#if doTimer
			auto timeBeforeTriangleTest = getTime();
#endif
			for (int i = 0; i < leafRays; i++)
			{
				auto rayId = leafWork[i];
				auto& ray = rays[rayId];

				//get current id (most recently added because we do depth first)
				const FastNode<nodeMemory>& node = compactNodes[-stack[--stackIndex[rayId]][rayId]];
				//test ray against NodeId and write result in correct array

				bool ispcResult;
				int loopCount = getLeafLoopCount(node.primIdBegin);
				callIspcTemplateNotConst(triAnyHit, loopCount, trianglePoints.data(), node.primIdBegin, reinterpret_cast<float*>(&ray));
				if constexpr (doCache)
				{
					//cache.loadStack((void*)&leafWork[i]);
					cache.loadStack((void*)&ray);
					//cache.loadStack((void*)&stackIndex[rayId]);
					cache.loadStack((void*)&stack[stackIndex[rayId] + 1]);

					leafCacheLoad((void*)&node, (void*)&trianglePoints[node.primIdBegin]);
				}
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
#if doAllLeafs
						i--;
#else
						leafWork[leafRaysNext++] = rayId;
#endif
					}
				}
			}
#if doTimer
			timeTriangleTest += getTimeSpan(timeBeforeTriangleTest);
#endif
			leafRays = leafRaysNext;
			leafRaysNext = 0;
			}
	}
}


template <unsigned gangSize, unsigned nodeMemory, unsigned workGroupSize>
template <bool doCache>
void FastNodeManager<gangSize, nodeMemory, workGroupSize>::intersectWideAlternative(std::array<FastRay, workGroupSquare>& rays,
	std::array<uint32_t, workGroupSquare>& leafIndex, std::array<int8_t, workGroupSquare>& triIndex,
	nanoSec& timeTriangleTest)
{
	//stack for each ray. 40 is current max stack size
	std::array< std::array<int32_t, workGroupSquare>, 40> stack;
	std::array< uint8_t, workGroupSquare>stackIndex;
	stack[0].fill(0);
	stackIndex.fill(1);

	//ray id list to keep track of what rays we need to do.
	std::array<uint16_t, workGroupSquare> work1;
	std::iota(work1.begin(), work1.end(), 0);
	std::array<uint16_t, workGroupSquare> work2;

	//precalculate the code for node traversal.
	//the first bit signales if its reverse or not, and the number resulting from (code >> 1) is the axis
	std::array<uint8_t, workGroupSquare > rayMajorAxis;
	for (int i = 0; i < workGroupSquare; i++)
	{
		uint8_t code = maxAbsDimension(rays[i].direction);
		if (rays[i].direction[code] <= 0)
		{
			code = (code << 1) + 1;
		}
		else
		{
			code = (code << 1);
		}
		rayMajorAxis[i] = code;
	}

	std::array<uint16_t, workGroupSquare>* currentWork = &work1;
	std::array<uint16_t, workGroupSquare>* nextWork = &work2;

	//number of noderays and leafrays so we know what ids to read.
	uint16_t nodeRays = workGroupSquare;
	uint16_t leafRays = 0;

	uint16_t nodeRaysNext = 0;
	uint16_t leafRaysNext = 0;

	//memory for aabb result
	std::array<float, nodeMemory> aabbDistances;
	aabbDistances.fill(NAN);

	while (nodeRays != 0 || leafRays != 0)
	{
		//loop over nodes
		if (nodeRays != 0)
		{
			for (int i = 0; i < nodeRays; i++)
			{
				auto rayId = (*currentWork)[i];
				auto& ray = rays[rayId];

				//get current id (most recently added because we do depth first)
				const FastNode<nodeMemory>& node = compactNodes[stack[--stackIndex[rayId]][rayId]];
				//test ray against NodeId and write result in correct array

				bool ispcResult;
				int	loopCount = getNodeLoopCount(node.bounds);
				callIspcTemplateNotConst(aabbIntersect, loopCount, node.bounds.data(), aabbDistances.data(), reinterpret_cast<float*>(&ray));
				if constexpr (doCache)
				{
					//cache.loadStack((void*)&leafWork[i]);
					cache.loadStack((void*)&ray);
					//cache.loadStack((void*)&stackIndex[rayId]);
					cache.loadStack((void*)&stack[stackIndex[rayId] + 1]);

					nodeCacheLoad((void*)&node);
				}
				if (ispcResult)
				{
					if (rayMajorAxis[rayId] & 1)
					{
						std::for_each(std::execution::seq, node.traverseOrderEachAxis[rayMajorAxis[i] >> 1].begin(), node.traverseOrderEachAxis[rayMajorAxis[i] >> 1].end(),
							[&](auto& cId)
							{
								if (!isnan(aabbDistances[cId]))
								{
									if (node.childType[cId])
									{
										//next is node
										stack[stackIndex[rayId]++][rayId] = (int32_t)node.childIdBegin + cId;
									}
									else
									{
										//next is leaf
										stack[stackIndex[rayId]++][rayId] = -(int32_t)(node.childIdBegin + cId);
									}
									aabbDistances[cId] = NAN;
								}
							});
					}
					else
					{
						//reverse order
						std::for_each(std::execution::seq, node.traverseOrderEachAxis[rayMajorAxis[i] >> 1].rbegin(), node.traverseOrderEachAxis[rayMajorAxis[i] >> 1].rend(),
							[&](auto& cId)
							{
								if (!isnan(aabbDistances[cId]))
								{
									if (node.childType[cId])
									{
										//next is node
										stack[stackIndex[rayId]++][rayId] = node.childIdBegin + cId;
									}
									else
									{
										//next is leaf
										stack[stackIndex[rayId]++][rayId] = -(int32_t)(node.childIdBegin + cId);
									}
									aabbDistances[cId] = NAN;
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
						(*nextWork)[nodeRaysNext++] = rayId;
					}
					else
					{
						//leaf
						(*nextWork)[workGroupSquare - 1 - (leafRaysNext++)] = rayId;
					}
				}
			}

		}

		//loop over triangles
		if (leafRays != 0)
			//if (leafRays >= 8 || nodeRays <= 8)
		{
#if doTimer
			auto timeBeforeTriangleTest = getTime();
#endif

			for (int i = 0; i < leafRays; i++)
			{
				auto rayId = (*currentWork)[workGroupSquare - 1 - i];
				auto& ray = rays[rayId];

				//get current id (most recently added because we do depth first)
				const FastNode<nodeMemory>& node = compactNodes[-stack[--stackIndex[rayId]][rayId]];
				//test ray against NodeId and write result in correct array

				int ispcResult;
				int loopCount = getLeafLoopCount(node.primIdBegin);
				callIspcTemplateNotConst(triIntersect, loopCount, trianglePoints.data(), node.primIdBegin, reinterpret_cast<float*>(&ray));
				if constexpr (doCache)
				{
					//cache.loadStack((void*)&leafWork[i]);
					cache.loadStack((void*)&ray);
					//cache.loadStack((void*)&stackIndex[rayId]);
					cache.loadStack((void*)&stack[stackIndex[rayId] + 1]);

					leafCacheLoad((void*)&node, (void*)&trianglePoints[node.primIdBegin]);
				}
				//it returns the hit id -> -1 = no hit
				if (ispcResult != -1)
				{
					leafIndex[rayId] = node.primIdBegin;
					triIndex[rayId] = ispcResult;
				}

				//depending on next element in stack. put this ray in node or leaf
				if (stackIndex[rayId] != 0)
				{
					if (stack[stackIndex[rayId] - 1][rayId] >= 0)
					{
						//node
						(*nextWork)[nodeRaysNext++] = rayId;
					}
					else
					{
						//leaf
#if doAllLeafs
						i--;
#else
						(*nextWork)[workGroupSquare - 1 - (leafRaysNext++)] = rayId;
#endif
					}
				}
			}
#if doTimer
			timeTriangleTest += getTimeSpan(timeBeforeTriangleTest);
#endif
			}
		//prepare next loop:
		leafRays = leafRaysNext;
		leafRaysNext = 0;

		nodeRays = nodeRaysNext;
		nodeRaysNext = 0;

		std::swap(currentWork, nextWork);
	}
}

template <unsigned gangSize, unsigned nodeMemory, unsigned workGroupSize>
template <bool doCache>
void FastNodeManager<gangSize, nodeMemory, workGroupSize>::intersectSecondaryWideAlternative(
	std::array<FastRay, workGroupSquare>& rays, std::array<uint16_t, workGroupSquare>& result, nanoSec& timeTriangleTest)
{
	//next nodeId for each ray. 32 is current max stack size. Negative id means leaf
	//id 0 is root node
	std::array< std::array<int32_t, workGroupSquare>, 40> stack;

	//id of the current element in the stack we have to work on. 0 means we are finished
	std::array< uint8_t, workGroupSquare>stackIndex;

	stackIndex.fill(1);
	stack[0].fill(0);

	//ray id list to keep track of what rays we need to do.
	std::array<uint16_t, workGroupSquare> work1;
	std::array<uint16_t, workGroupSquare> work2;

	std::array<uint16_t, workGroupSquare>* currentWork = &work1;
	std::array<uint16_t, workGroupSquare>* nextWork = &work2;



	int counter = 0;
	for (int i = 0; i < workGroupSquare; i++)
	{
		if (!isnan(rays[i].pos.x))
		{
			work1[counter++] = i;
		}
	}

	//number of noderays and leafrays so we how much to read in nodeWork and leafWork
	uint16_t nodeRays = counter;
	uint16_t leafRays = 0;

	//same as above but for the next iteration.
	uint16_t nodeRaysNext = 0;
	uint16_t leafRaysNext = 0;

	//memory for aabb result
	std::array<float, nodeMemory> aabbDistances;
	aabbDistances.fill(NAN);

	while (nodeRays != 0 || leafRays != 0)
	{
		//loop over nodes
		if (nodeRays != 0)
		{
			for (int i = 0; i < nodeRays; i++)
			{
				auto rayId = (*currentWork)[i];
				auto& ray = rays[rayId];

				//get current id (most recently added because we do depth first)
				const FastNode<nodeMemory>& node = compactNodes[stack[--stackIndex[rayId]][rayId]];
				//test ray against NodeId and write result in correct array

				bool ispcResult;
				int loopCount = getNodeLoopCount(node.bounds);
				callIspcTemplateNotConst(aabbIntersect, loopCount, node.bounds.data(), aabbDistances.data(), reinterpret_cast<float*>(&ray));
				if constexpr (doCache)
				{
					//cache.loadStack((void*)&leafWork[i]);
					cache.loadStack((void*)&ray);
					//cache.loadStack((void*)&stackIndex[rayId]);
					cache.loadStack((void*)&stack[stackIndex[rayId] + 1]);

					nodeCacheLoad((void*)&node);
				}
				if (ispcResult)
				{
					//this loop is faster with constant (nodememory) than with branching factor for N4L4.
					for (int cId = 0; cId < nodeMemory; cId++)
					{
						if (!isnan(aabbDistances[cId]))
						{
							if (node.childType[cId])
							{
								//next is node
								stack[stackIndex[rayId]++][rayId] = node.childIdBegin + cId;
							}
							else
							{
								//next is leaf
								stack[stackIndex[rayId]++][rayId] = -(int32_t)(node.childIdBegin + cId);
							}
							aabbDistances[cId] = NAN;
						}
					}
				}
				//depending on next element in stack. put this ray in node or leaf
				if (stackIndex[rayId] != 0)
				{
					if (stack[stackIndex[rayId] - 1][rayId] >= 0)
					{
						//node
						(*nextWork)[nodeRaysNext++] = rayId;
					}
					else
					{
						//leaf
						(*nextWork)[workGroupSquare - 1 - (leafRaysNext++)] = rayId;
					}
				}
			}
		}
		//loop over triangles
		if (leafRays != 0)
			//if (leafRays >= 8 || nodeRays <= 8)
		{
#if doTimer
			auto timeBeforeTriangleTest = getTime();
#endif
			for (int i = 0; i < leafRays; i++)
			{
				auto rayId = (*currentWork)[workGroupSquare - 1 - i];
				auto& ray = rays[rayId];

				//get current id (most recently added because we do depth first)
				const FastNode<nodeMemory>& node = compactNodes[-stack[--stackIndex[rayId]][rayId]];
				//test ray against NodeId and write result in correct array

				bool ispcResult;
				int loopCount = getLeafLoopCount(node.primIdBegin);
				callIspcTemplateNotConst(triAnyHit, loopCount, trianglePoints.data(), node.primIdBegin, reinterpret_cast<float*>(&ray));
				if constexpr (doCache)
				{
					//cache.loadStack((void*)&leafWork[i]);
					cache.loadStack((void*)&ray);
					//cache.loadStack((void*)&stackIndex[rayId]);
					cache.loadStack((void*)&stack[stackIndex[rayId] + 1]);

					leafCacheLoad((void*)&node, (void*)&trianglePoints[node.primIdBegin]);
				}
				if (ispcResult)
				{
					result[rayId] ++;
					//this ray is finished. Cancel and dont add it to the stack
					continue;
				}

				//depending on next element in stack. put this ray in node or leaf stack
				if (stackIndex[rayId] != 0)
				{
					if (stack[stackIndex[rayId] - 1][rayId] >= 0)
					{
						//node
						(*nextWork)[nodeRaysNext++] = rayId;
					}
					else
					{
						//leaf
#if doAllLeafs
						i--;
#else
						(*nextWork)[workGroupSquare - 1 - (leafRaysNext++)] = rayId;
#endif
					}
				}
			}
#if doTimer
			timeTriangleTest += getTimeSpan(timeBeforeTriangleTest);
#endif
			}
		//prepare next loop:
		leafRays = leafRaysNext;
		leafRaysNext = 0;

		nodeRays = nodeRaysNext;
		nodeRaysNext = 0;

		std::swap(currentWork, nextWork);
	}
}

template <unsigned gangSize, unsigned nodeMemory, unsigned workGroupSize>
template <bool doCache>
bool FastNodeManager<gangSize, nodeMemory, workGroupSize>::intersectSaveDistance(FastRay& ray, uint32_t& leafIndex, uint8_t& triIndex, nanoSec& timeTriangleTest)
{
	//ids of ndodes that we still need to test:
	std::array<std::tuple<uint32_t, float>, 40> nodeStack;
	nodeStack[0] = std::make_tuple<uint32_t, float>(0, 0);
	uint8_t stackIndex = 1;

	bool result = false;

	std::array<float, nodeMemory> aabbDistances;
	aabbDistances.fill(NAN);

	//precalculate the code for node traversal.
	int code = maxAbsDimension(ray.direction);
	bool reverse = ray.direction[code] <= 0;

	while (stackIndex != 0)
	{
		//get current id (most recently added because we do depth first)
		auto& tup = nodeStack[--stackIndex];
		if constexpr (doCache)
		{
			cache.loadStack((void*)&tup);
		}
		if (std::get<1>(tup) > ray.tMax)
		{
			continue;
		}
		const FastNode<nodeMemory>& node = compactNodes[std::get<0>(tup)];

		if (isnan(node.bounds[0]))
		{
#if doTimer
			auto timeBeforeTriangleTest = getTime();
#endif
			int ispcResult;
			int loopCount = getLeafLoopCount(node.primIdBegin);
			callIspcTemplateNotConst(triIntersect, loopCount, trianglePoints.data(), node.primIdBegin, reinterpret_cast<float*>(&ray));
			if constexpr (doCache)
				leafCacheLoad((void*)&node, (void*)&trianglePoints[node.primIdBegin]);
			//it returns the hit id -> -1 = no hit
			if (ispcResult != -1)
			{
				result = true;
				leafIndex = node.primIdBegin;
				triIndex = ispcResult;
			}
#if doTimer
			timeTriangleTest += getTimeSpan(timeBeforeTriangleTest);
#endif
			}
		else
		{
			//child tests: 
			//ispc version:

			//call ispc method that returns an array of min distances. if minDist = Nan -> no hit otherwise hit.
			//go trough traverseOrder and add those childs with distance != Nan to queue (in right order)
			bool ispcResult;
			int loopCount = getNodeLoopCount(node.bounds);
			callIspcTemplateNotConst(aabbIntersect, loopCount, node.bounds.data(), aabbDistances.data(), reinterpret_cast<float*>(&ray));
			if constexpr (doCache)
				nodeCacheLoad((void*)&node);
			{
				if (reverse)
				{
					std::for_each(std::execution::seq, node.traverseOrderEachAxis[code].begin(), node.traverseOrderEachAxis[code].end(),
						[&](auto& cId)
						{
							if (!isnan(aabbDistances[cId]))
							{
								nodeStack[stackIndex++] = std::make_tuple(node.childIdBegin + cId, aabbDistances[cId]);
								aabbDistances[cId] = NAN;
							}
						});
				}
				else
				{
					std::for_each(std::execution::seq, node.traverseOrderEachAxis[code].rbegin(), node.traverseOrderEachAxis[code].rend(),
						[&](auto& cId)
						{
							if (!isnan(aabbDistances[cId]))
							{
								nodeStack[stackIndex++] = std::make_tuple(node.childIdBegin + cId, aabbDistances[cId]);
								aabbDistances[cId] = NAN;
							}
						});
				}
			}
		}
	}
	return result;
}

template <unsigned gangSize, unsigned nodeMemory, unsigned workGroupSize>
template <bool doCache>
bool FastNodeManager<gangSize, nodeMemory, workGroupSize>::intersect(FastRay& ray, uint32_t& leafIndex, uint8_t& triIndex, nanoSec& timeTriangleTest)
{
	//ids of ndodes that we still need to test:
	std::array<uint32_t, 40> nodeStack;
	nodeStack[0] = 0;
	uint8_t stackIndex = 1;

	bool result = false;

	std::array<float, nodeMemory> aabbDistances;
	aabbDistances.fill(NAN);

	//precalculate the code for node traversal.
	int code = maxAbsDimension(ray.direction);
	bool reverse = ray.direction[code] <= 0;

	while (stackIndex != 0)
	{
		//get current id (most recently added because we do depth first)
		const FastNode<nodeMemory>& node = compactNodes[nodeStack[--stackIndex]];
		if constexpr (doCache)
		{
			cache.loadStack((void*)&nodeStack[stackIndex + 1]);
		}
		if (isnan(node.bounds[0]))
		{
#if doTimer
			auto timeBeforeTriangleTest = getTime();
#endif

			int ispcResult;
			int loopCount = getLeafLoopCount(node.primIdBegin);
			callIspcTemplateNotConst(triIntersect, loopCount, trianglePoints.data(), node.primIdBegin, reinterpret_cast<float*>(&ray));
			if constexpr (doCache)
				leafCacheLoad((void*)&node, (void*)&trianglePoints[node.primIdBegin]);
			//it returns the hit id -> -1 = no hit
			if (ispcResult != -1)
			{
				result = true;
				leafIndex = node.primIdBegin;
				triIndex = ispcResult;
			}
#if doTimer
			timeTriangleTest += getTimeSpan(timeBeforeTriangleTest);
#endif
			}
		else
		{
			//child tests: 
			//ispc version:

			//call ispc method that returns an array of min distances. if minDist = Nan -> no hit otherwise hit.
			//go trough traverseOrder and add those childs with distance != Nan to queue (in right order)
			bool ispcResult;
			int loopCount = getNodeLoopCount(node.bounds);
			callIspcTemplateNotConst(aabbIntersect, loopCount, node.bounds.data(), aabbDistances.data(), reinterpret_cast<float*>(&ray));
			if constexpr (doCache)
				nodeCacheLoad((void*)&node);
			if (ispcResult)
			{
				if (reverse)
				{
					std::for_each(std::execution::seq, node.traverseOrderEachAxis[code].begin(), node.traverseOrderEachAxis[code].end(),
						[&](auto& cId)
						{
							if (!isnan(aabbDistances[cId]))
							{
								nodeStack[stackIndex++] = node.childIdBegin + cId;
								aabbDistances[cId] = NAN;
							}
						});
				}
				else
				{
					std::for_each(std::execution::seq, node.traverseOrderEachAxis[code].rbegin(), node.traverseOrderEachAxis[code].rend(),
						[&](auto& cId)
						{
							if (!isnan(aabbDistances[cId]))
							{
								nodeStack[stackIndex++] = node.childIdBegin + cId;
								aabbDistances[cId] = NAN;
							}
						});
				}
			}
		}
	}
	return result;
}

template <unsigned gangSize, unsigned nodeMemory, unsigned workGroupSize>
template <bool doCache>
bool FastNodeManager<gangSize, nodeMemory, workGroupSize>::intersectSecondary(FastRay& ray, nanoSec& timeTriangleTest)
{
	//bvh intersection for secondary hits. -> anyhit
	//differences: no return for triangles. No aabb distance stop (no saving of distance of aabb intersections since we stop after first hit)

	//ids of ndodes that we still need to test:
	std::array<uint32_t, 40> nodeStack;
	nodeStack[0] = 0;
	uint8_t stackIndex = 1;
	bool result = false;

	std::array<float, nodeMemory> aabbDistances;
	aabbDistances.fill(NAN);

	while (stackIndex != 0)
	{
		//get current id (most recently added because we do depth first)
		const FastNode<nodeMemory>& node = compactNodes[nodeStack[--stackIndex]];
		if constexpr (doCache)
		{
			cache.loadStack((void*)&nodeStack[stackIndex + 1]);
		}
		if (isnan(node.bounds[0]))
		{
#if doTimer
			auto timeBeforeTriangleTest = getTime();
#endif
			bool ispcResult;
			int loopCount = getLeafLoopCount(node.primIdBegin);
			callIspcTemplateNotConst(triAnyHit, loopCount, trianglePoints.data(), node.primIdBegin, reinterpret_cast<float*>(&ray));
			if constexpr (doCache)
				leafCacheLoad((void*)&node, (void*)&trianglePoints[node.primIdBegin]);
			if (ispcResult)
			{
				//we dont care about exact result for shadowrays (could do other ispc intersection for it
#if doTimer
				timeTriangleTest += getTimeSpan(timeBeforeTriangleTest);
#endif
				return true;
			}
#if doTimer
			timeTriangleTest += getTimeSpan(timeBeforeTriangleTest);
#endif
		}
		else
		{
			//child tests: 
			//ispc version:

			//call ispc method that returns an array of min distances. if minDist = Nan -> no hit otherwise hit.
			//go trough traverseOrder and add those childs with distance != Nan to queue (in right order)

			bool ispcResult;
			int loopCount = getNodeLoopCount(node.bounds);
			callIspcTemplateNotConst(aabbIntersect, loopCount, node.bounds.data(), aabbDistances.data(), reinterpret_cast<float*>(&ray));
			if constexpr (doCache)
				nodeCacheLoad((void*)&node);
			if (ispcResult)
			{
				for (int i = 0; i < nodeMemory; i++)
				{
					if (!isnan(aabbDistances[i]))
					{
						nodeStack[stackIndex++] = node.childIdBegin + i;
						aabbDistances[i] = NAN;
					}
				}
			}
		}
	}
	return result;
}

template <unsigned gangSize, unsigned nodeMemory, unsigned  workGroupSize>
FastNodeManager<gangSize, nodeMemory, workGroupSize>::FastNodeManager(Bvh& bvh, int leafMemory, CacheSimulator cache)
	:leafMemory(leafMemory), cache(cache)
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
		std::array<float, nodeMemory * 6> bounds;
		bounds.fill(0);

		//types of the childs
		std::bitset<nodeMemory> childType;
		int leafRealSize = 0;
		int nodeRealSize = 0;
		if (n->node->getChildCount() > 0)
		{
			cBegin = (*n->children.begin())->id;
			cCount = n->node->getChildCount();
			nodeRealSize = ((cCount - 1) / gangSize + 1) * gangSize;
			//now fill aabb (will fill it here and then just copy in in the constructor. performance isnt that important here)

			//soa order aabb
			for (int i = 0; i < 3; i++)
			{
				for (size_t j = 0; j < nodeRealSize; j++)
				{
					if (j < n->children.size())
					{
						bounds[i * nodeRealSize + j] = n->children[j]->boundMin[i];
					}
				}
			}
			for (int i = 0; i < 3; i++)
			{
				for (size_t j = 0; j < nodeRealSize; j++)
				{
					if (j < n->children.size())
					{
						bounds[nodeRealSize * 3 + i * nodeRealSize + j] = n->children[j]->boundMax[i];
					}
				}
			}
			if (nodeRealSize != nodeMemory)
			{
				bounds[bounds.size() - 2] = nodeRealSize;
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
			leafRealSize = ((pCount - 1) / gangSize + 1) * gangSize;
			uint32_t pTmp = pBegin + leafRealSize;
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
				trianglePoints.resize(pTmp);
				pTmp += leafRealSize;
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
				trianglePoints.resize(pTmp);
				pTmp += leafRealSize;
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
				trianglePoints.resize(pTmp);
				pTmp += leafRealSize;
			}
			//in theory this could be removed and the space for a leaf could be smaller
			//IF!! i save tri number and get the correct size to the pos and normal calculation
			trianglePoints.resize(pBegin + leafMemory * 9);
			if (leafRealSize != leafMemory)
			{
				trianglePoints[pBegin + leafMemory * 9 - 2] = leafRealSize;
				trianglePoints[pBegin + leafMemory * 9 - 1] = NAN;
			}
			bounds[0] = NAN;

		}
		//Aabb* aabb = static_cast<Aabb*>(n->node);

		compactNodes.push_back(FastNode<nodeMemory>(cBegin, nodeRealSize, pBegin, leafRealSize, bounds, n->node->traverseOrderEachAxis, childType));
	}

	//bloat memory:
	//after each float we add and additional float
#if nodeLeafPadding != 1
	int origTriSize = trianglePoints.size();
	trianglePoints.resize(origTriSize * nodeLeafPadding);
	for (int i = origTriSize - 1; i >= 0; i--)
	{
		trianglePoints[i * nodeLeafPadding] = trianglePoints[i];
	}
#endif // padding


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
inline void FastNodeManager<gangSize, nodeMemory, workGroupSize>::getSurfaceNormalTri(const FastRay& ray, glm::vec3& surfaceNormal, const uint32_t leafIndex, const uint8_t triIndex)
{
	glm::vec3 p0, p1, p2;
	getTriPoints(p0, p1, p2, leafIndex, triIndex);
	surfaceNormal = computeNormal(p0, p1, p2);
}

//calculates the surface normal and position
template <unsigned gangSize, unsigned nodeMemory, unsigned  workGroupSize>
inline void FastNodeManager<gangSize, nodeMemory, workGroupSize>::getSurfaceNormalPosition(const FastRay& ray, glm::vec3& surfaceNormal, glm::vec3& surfacePosition, const uint32_t leafIndex, const uint8_t triIndex)
{
	glm::vec3 p0, p1, p2;
	getTriPoints(p0, p1, p2, leafIndex, triIndex);

	surfaceNormal = computeNormal(p0, p1, p2);

	//calculating t a bit more accurate. (im not sure why the calcualtion in ispc is that different?)
	float denom = glm::dot(surfaceNormal, ray.direction);
	glm::vec3 p0l0 = p0 - ray.pos;
	float t = glm::dot(p0l0, surfaceNormal) / denom;
	surfacePosition = ray.pos + ray.direction * t;
}

template <unsigned gangSize, unsigned nodeMemory, unsigned  workGroupSize>
inline void FastNodeManager<gangSize, nodeMemory, workGroupSize>::getTriPoints(glm::vec3& p0, glm::vec3& p1, glm::vec3& p2, const uint32_t leafIndex, const uint8_t triIndex)
{
	int loopCount = getLeafLoopCount(leafIndex);
	uint32_t triId = triIndex + leafIndex;
	//adjust for padding
	loopCount *= nodeLeafPadding;
	triId *= nodeLeafPadding;
	p0 = glm::vec3(trianglePoints[triId], trianglePoints[triId + loopCount], trianglePoints[triId + loopCount * 2]);
	p1 = glm::vec3(trianglePoints[triId + loopCount * 3], trianglePoints[triId + loopCount * 4], trianglePoints[triId + loopCount * 5]);
	p2 = glm::vec3(trianglePoints[triId + loopCount * 6], trianglePoints[triId + loopCount * 7], trianglePoints[triId + loopCount * 8]);
}

template <unsigned gangSize, unsigned nodeMemory, unsigned  workGroupSize>
inline int FastNodeManager<gangSize, nodeMemory, workGroupSize>::getLeafLoopCount(const uint32_t leafIndex)
{
	//return leafMemory
	if (leafMemory == gangSize)
	{
		return leafMemory;
	}
	int loopCount = leafMemory;
	int triEndId = leafIndex + leafMemory * 9;

	if (isnan(trianglePoints[triEndId * nodeLeafPadding - 1]))
	{
		loopCount = trianglePoints[triEndId * nodeLeafPadding - nodeLeafPadding - 1];
	}


	return loopCount;
}

template <unsigned gangSize, unsigned nodeMemory, unsigned  workGroupSize>
inline int FastNodeManager<gangSize, nodeMemory, workGroupSize>::getNodeLoopCount(const std::array<float, nodeMemory * 6 * nodeLeafPadding>& bounds)
{
	//return nodeMemory;
	if constexpr (nodeMemory == gangSize)
	{
		return nodeMemory;
	}
	int loopCount = nodeMemory;

	if (isnan(bounds[nodeMemory * 6 * nodeLeafPadding - 1]))
	{
		loopCount = bounds[nodeMemory * 6 * nodeLeafPadding - 2];
	}
	return loopCount;
}