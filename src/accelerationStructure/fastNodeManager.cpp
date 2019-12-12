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
macro5(gS, 20, wGS)\
macro5(gS, 24, wGS)\
macro5(gS, 28, wGS)\
macro5(gS, 32, wGS)\
macro5(gS, 36, wGS)\
macro5(gS, 40, wGS)\
macro5(gS, 44, wGS)\
macro5(gS, 48, wGS)\
macro5(gS, 52, wGS)\
macro5(gS, 56, wGS)\
macro5(gS, 60, wGS)\
macro5(gS, 64, wGS)\

//macro4 does gangsize 8
#define macro4(gS, wGS) macro5(gS, 8, wGS)\
macro5(gS, 16, wGS)\
macro5(gS, 24, wGS)\
macro5(gS, 32, wGS)\
macro5(gS, 40, wGS)\
macro5(gS, 48, wGS)\
macro5(gS, 56, wGS)\
macro5(gS, 64, wGS)

#define macro5(gS, mS, wGS) template class FastNodeManager<gS, mS, wGS>;

//template class FastNodeManager<4, 4, 8>;
macro1()

template <unsigned gangSize, unsigned nodeMemory, unsigned  workGroupSize>
bool FastNodeManager<gangSize, nodeMemory, workGroupSize>::intersectSaveDistance(FastRay& ray, uint32_t& leafIndex, uint8_t& triIndex, double& timeTriangleTest) const
{
	//ids of ndodes that we still need to test:
	std::array<std::tuple<uint32_t, float>, 32> queueArray;
	queueArray[0] = std::make_tuple<uint32_t, float>(0, 0);
	uint8_t queueIndex = 1;

	bool result = false;

	std::array<float, nodeMemory> aabbDistances;
	aabbDistances.fill(-100000);

	while (queueIndex != 0)
	{
		//get current id (most recently added because we do depth first)
		auto& tup = queueArray[--queueIndex];
		if (std::get<1>(tup) > ray.tMax)
		{
			continue;
		}
		const FastNode<nodeMemory>* node = &compactNodes[std::get<0>(tup)];

		if (!node->hasChildren)
		{
			auto timeBeforeTriangleTest = getTime();

			int resultIndex = triIntersect(trianglePoints.data(), node->primIdBegin, reinterpret_cast<float*>(&ray), leafMemory, leafSize);
			//it returns the hit id -> -1 = no hit
			if (resultIndex != -1)
			{
				result = true;
				leafIndex = node->primIdBegin;
				triIndex = resultIndex;
			}
			timeTriangleTest += getTimeSpan(timeBeforeTriangleTest);
		}
		else
		{
			//child tests: 
			//ispc version:

			//call ispc method that returns an array of min distances. if minDist = 0 -> no hit otherwise hit.
			//go trough traverseOrder and add those childs with distance != 0 to queue (in right order)

			if (aabbIntersect((float*)node->bounds.data(), (float*)aabbDistances.data(), reinterpret_cast<float*>(&ray), nodeMemory, branchingFactor))
			{
				int8_t code = 0;
				code = code | (ray.direction[0] <= 0);
				code = code | ((ray.direction[1] <= 0) << 1);
				bool reverse = ray.direction[2] <= 0;
				if (reverse)
				{
					code = code ^ 3;
					std::for_each(std::execution::seq, node->traverseOrderEachAxis[code].begin(), node->traverseOrderEachAxis[code].end(),
						[&](auto& cId)
						{
							if (aabbDistances[cId] != -100000)
							{
								queueArray[queueIndex++] = std::make_tuple(node->childIdBegin + cId, aabbDistances[cId]);
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
								queueArray[queueIndex++] = std::make_tuple(node->childIdBegin + cId, aabbDistances[cId]);
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
bool FastNodeManager<gangSize, nodeMemory, workGroupSize>::intersect(FastRay& ray, uint32_t& leafIndex, uint8_t& triIndex, double& timeTriangleTest) const
{
	//ids of ndodes that we still need to test:
	std::array<uint32_t, 32> queueArray;
	queueArray[0] = 0;
	uint8_t queueIndex = 1;

	bool result = false;

	std::array<float, nodeMemory> aabbDistances;
	aabbDistances.fill(-100000);

	while (queueIndex != 0)
	{
		//get current id (most recently added because we do depth first)
		const FastNode<nodeMemory>* node = &compactNodes[queueArray[--queueIndex]];

		if (!node->hasChildren)
		{
			auto timeBeforeTriangleTest = getTime();

			int resultIndex = triIntersect(trianglePoints.data(), node->primIdBegin, reinterpret_cast<float*>(&ray), leafMemory, leafSize);
			//it returns the hit id -> -1 = no hit
			if (resultIndex != -1)
			{
				result = true;
				leafIndex = node->primIdBegin;
				triIndex = resultIndex;
			}
			timeTriangleTest += getTimeSpan(timeBeforeTriangleTest);
		}
		else
		{
			//child tests: 
			//ispc version:

			//call ispc method that returns an array of min distances. if minDist = 0 -> no hit otherwise hit.
			//go trough traverseOrder and add those childs with distance != 0 to queue (in right order)

			if (aabbIntersect((float*)node->bounds.data(), (float*)aabbDistances.data(), reinterpret_cast<float*>(&ray), nodeMemory, branchingFactor))
			{
				int8_t code = 0;
				code = code | (ray.direction[0] <= 0);
				code = code | ((ray.direction[1] <= 0) << 1);
				bool reverse = ray.direction[2] <= 0;
				if (reverse)
				{
					code = code ^ 3;
					std::for_each(std::execution::seq, node->traverseOrderEachAxis[code].begin(), node->traverseOrderEachAxis[code].end(),
						[&](auto& cId)
						{
							if (aabbDistances[cId] != -100000)
							{
								queueArray[queueIndex++] = node->childIdBegin + cId;
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
								queueArray[queueIndex++] = node->childIdBegin + cId;
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
bool FastNodeManager<gangSize, nodeMemory, workGroupSize>::intersectSecondary(FastRay& ray, double& timeTriangleTest) const
{
	//bvh intersection for secondary hits. -> anyhit
	//differences: no return for triangles. No aabb distance stop (no saving of distance of aabb intersections since we stop after first hit)

	//ids of ndodes that we still need to test:
	std::array<uint32_t, 32> queueArray;
	queueArray[0] = 0;
	uint8_t queueIndex = 1;
	bool result = false;

	std::array<float, nodeMemory> aabbDistances;
	aabbDistances.fill(-100000);

	while (queueIndex != 0)
	{
		//get current id (most recently added because we do depth first)
		uint32_t id = queueArray[--queueIndex];

		const FastNode<nodeMemory>* node = &compactNodes[id];

		if (!node->hasChildren)
		{
			auto timeBeforeTriangleTest = getTime();

			if (triAnyHit(trianglePoints.data(), node->primIdBegin, reinterpret_cast<float*>(&ray), leafMemory, leafSize))
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

			if (aabbIntersect((float*)node->bounds.data(), (float*)aabbDistances.data(), reinterpret_cast<float*>(&ray), nodeMemory, branchingFactor))
			{
				for (int i = 0; i < branchingFactor; i++)
				{
					if (aabbDistances[i] != -100000)
					{
						queueArray[queueIndex++] = node->childIdBegin + i;
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
		}
		//Aabb* aabb = static_cast<Aabb*>(n->node);

		compactNodes.push_back(FastNode<nodeMemory>(cBegin, cCount, pBegin, pCount, bounds, n->node->traverseOrderEachAxis));
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