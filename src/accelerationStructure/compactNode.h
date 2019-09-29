#pragma once
#include <iostream>
#include <algorithm>
#include <vector>
#include <numeric>

#include "bvh.h"
#include "../ray.h"

//compact node with random child ids
struct CompactNodeV0
{
	char sortAxis;
	std::vector<size_t> childrenIds;
	size_t primIdBegin;
	size_t primIdEnd;

	glm::vec3 boundMin;
	glm::vec3 boundMax;

	CompactNodeV0(std::vector<size_t> childrenIds, size_t primIdBegin, size_t primIdEnd, glm::vec3 boundMin, glm::vec3 boundMax, char sortAxis)
		:childrenIds(childrenIds), primIdBegin(primIdBegin), primIdEnd(primIdEnd), boundMin(boundMin), boundMax(boundMax), sortAxis(sortAxis)
	{
	}

	inline int getChildCount()
	{
		return childrenIds.size();
	}

	inline bool noChildren()
	{
		return childrenIds.size() == 0;
	}

	inline std::vector<size_t> getChildVector()
	{
		return childrenIds;
	}
};

//compact node with consecutive child ids (use if possible)
struct CompactNodeV1
{
	char sortAxis;
	//in theory the end part only needs to be really small (could be offset to begin) -> right now its not
	size_t childIdBegin;
	size_t childIdEnd;
	size_t primIdBegin;
	size_t primIdEnd;

	glm::vec3 boundMin;
	glm::vec3 boundMax;

	CompactNodeV1()
	{
	}
	CompactNodeV1(size_t childIdBegin, size_t childIdEnd, size_t primIdBegin, size_t primIdEnd, glm::vec3 boundMin, glm::vec3 boundMax, char sortAxis)
		: childIdBegin(childIdBegin), childIdEnd(childIdEnd), primIdBegin(primIdBegin), primIdEnd(primIdEnd), boundMin(boundMin), boundMax(boundMax), sortAxis(sortAxis)
	{
	}
	/*
	CompactNodeV1(std::vector<size_t> childrenIds, size_t primIdBegin, size_t primIdEnd, glm::vec3 boundMin, glm::vec3 boundMax, char sortAxis)
		: primIdBegin(primIdBegin), primIdEnd(primIdEnd), boundMin(boundMin), boundMax(boundMax), sortAxis(sortAxis)
	{
		if (childrenIds.size() != 0)
		{
			childIdBegin = *childrenIds.begin();
			childIdEnd = *(childrenIds.end() - 1);
		}
		else
		{
			childIdBegin = 0;
			childIdEnd = 0;
		}
	}*/

	inline int getChildCount()
	{
		return childIdEnd - childIdBegin + 1;
	}

	inline bool noChildren()
	{
		return childIdBegin == childIdEnd;
	}

	inline std::vector<size_t> getChildVector()
	{
		std::vector<size_t> result(getChildCount());
		std::iota(result.begin(), result.end(), childIdBegin);
		return result;
	}
};

//only for aabb!
template<typename T>
class CompactNodeManager
{
	//depending on the node order we can choose the consecutive V1 or have to use V0
	std::vector<T> compactNodes;

	std::shared_ptr<primPointVector> primitives;
public:

	//copies a bvh and rearanges it into a single vector of compact nodes
	CompactNodeManager(Bvh bvh, int nodeOrder);

	//compact node intersect with the same results as the bvh intersect
	bool intersect(Ray& ray);

	//similar to normal intersect but instantly tests all child aabbs immediately (instead of only testing the closest)
	bool intersectImmediately(Ray& ray);
	
	//first add all children of node, then rekusion for each child
	void customTreeOrder(NodeAnalysis* n, std::vector<NodeAnalysis*>& nodeVector);

	inline bool aabbCheck(Ray& ray, int id)// CompactNodeV1& node)
	{
		ray.aabbIntersectionCount++;
		//aabb intersection (same as in Aabb)
		//code modified from here : https://gamedev.stackexchange.com/questions/18436/most-efficient-aabb-vs-ray-collision-algorithms
		float t;

		glm::fvec3 t1 = (compactNodes[id].boundMin - ray.pos) * ray.invDirection;
		glm::fvec3 t2 = (compactNodes[id].boundMax - ray.pos) * ray.invDirection;
		float tmin = glm::compMax(glm::min(t1, t2));
		float tmax = glm::compMin(glm::max(t1, t2));

		// if tmax < 0, ray (line) is intersecting AABB, but the whole AABB is behind us
		if (tmax < 0)
		{
			t = tmax;
			return false;
		}
		// if tmin > tmax, ray doesn't intersect AABB
		if (tmin > tmax)
		{
			t = tmax;
			return false;
		}
		//stop when current ray distance is closer than minimum possible distance of the aabb
		if (ray.tMax < tmin)
		{
			return false;
		}
		ray.successfulAabbIntersectionCount++;
		return true;
	}

	//debug method to test if all nodes are connected corretly
	/*
	int testTraverse()
	{
		int result = 0;
		int count2 = 0;


		std::vector<size_t> queue;
		queue.push_back(0);
		while (queue.size() != 0)
		{
			//get current id (most recently added because we do depth first)
			size_t id = queue.back();
			queue.pop_back();

			//check intersection with id
			CompactNodeV1 node = compactNodes[id];
			result++;

			if (node.primIdBegin != node.primIdEnd)
			{
				count2++;
			}

			if (node.childIdBegin != node.childIdEnd)
			{
				for (size_t i = node.childIdEnd; i >= node.childIdBegin; i--)
				{
					queue.push_back(i);
				}
			}
		}
		return result;
	}*/
};