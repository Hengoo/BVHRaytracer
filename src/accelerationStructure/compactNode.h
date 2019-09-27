#pragma once
#include <iostream>
#include <algorithm>
#include <vector>

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

	CompactNodeV1(size_t childIdBegin, size_t childIdEnd, size_t primIdBegin, size_t primIdEnd, glm::vec3 boundMin, glm::vec3 boundMax, char sortAxis)
		: childIdBegin(childIdBegin), childIdEnd(childIdEnd), primIdBegin(primIdBegin), primIdEnd(primIdEnd), boundMin(boundMin), boundMax(boundMax), sortAxis(sortAxis)
	{
	}
};

//only for aabb!
class CompactNodeManager
{
	int usedNodeType;
	//depending on the node order we can choose the consecutive V1 or have to use V0
	std::vector<CompactNodeV0> compactNodesV0;
	std::vector<CompactNodeV1> compactNodesV1;

	std::shared_ptr<primPointVector> primitives;
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
				Aabb* aabb = static_cast<Aabb*>(n->node);
				compactNodesV0.push_back(CompactNodeV0(childs, pBegin, pEnd, aabb->boundMin, aabb->boundMax, n->node->sortAxis));
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
				Aabb* aabb = static_cast<Aabb*>(n->node);
				compactNodesV1.push_back(CompactNodeV1(cBegin, cEnd, pBegin, pEnd, aabb->boundMin, aabb->boundMax, n->node->sortAxis));
			}
		}
		int test = testTraverse();
		int deb = 0;
		primitives = bvh.primitives;
	}

	//debug method to test if all nodes are connected corretly
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
			CompactNodeV1 node = compactNodesV1[id];
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
	}

	bool intersect(Ray& ray)
	{
		//traverse compact node vector

		//most of the logging can also be done here, but we do not know the depth of

		//ids of ndodes that we still need to test:
		std::vector<size_t> queue;
		queue.push_back(0);

		bool result = false;

		while (queue.size() != 0)
		{
			//get current id (most recently added because we do depth first)
			size_t id = queue.back();
			queue.pop_back();

			//check intersection with id
			CompactNodeV1 node = compactNodesV1[id];

			if (!aabbCheck(ray, node))
			{
				continue;
			}

			//intersection happened, test primitives
			if (node.primIdBegin != node.primIdEnd)
			{
				int primCount = node.primIdEnd - node.primIdBegin;
				//logging: primitive fullness:
				ray.primitiveFullness[primCount] ++;
				ray.leafIntersectionCount[0]++;
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
			if (node.childIdBegin != node.childIdEnd)
			{
				//update child FUllness:
				int childCount = node.childIdEnd - node.childIdBegin;
				ray.childFullness[childCount + 1] ++;

				//increment node intersection counter
				ray.nodeIntersectionCount[0]++;
				if (ray.direction[node.sortAxis] > 0)
				{
					for (size_t i = node.childIdEnd; i >= node.childIdBegin; i--)
					{
						queue.push_back(i);
					}
				}
				else
				{
					for (size_t i = node.childIdBegin; i <= node.childIdEnd; i++)
					{
						queue.push_back(i);
					}
				}

			}
		}
		return result;
	}

	inline bool aabbCheck(Ray& ray, CompactNodeV1& node)
	{
		ray.aabbIntersectionCount++;

		//aabb intersection (same as in Aabb)
		//code modified from here : https://gamedev.stackexchange.com/questions/18436/most-efficient-aabb-vs-ray-collision-algorithms
		float t;
		glm::fvec3 t1 = (node.boundMin - ray.pos) * ray.invDirection;
		glm::fvec3 t2 = (node.boundMax - ray.pos) * ray.invDirection;
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

	//similar to normal intersect but instantly tests all child aabbs (instead of only testing the closest)
	bool intersectTest(Ray& ray)
	{
		//traverse compact node vector

		//check root node
		CompactNodeV1 node = compactNodesV1[0];
		if (!aabbCheck(ray, node))
		{
			return false;
		}

		//ids of nodes that are already tested but didnt test the children jet:
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
			node = compactNodesV1[id];

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
			if (node.childIdBegin != node.childIdEnd)
			{
				//update counters
				int childCount = node.childIdEnd - node.childIdBegin;

				ray.childFullness[childCount + 1] ++;
				ray.nodeIntersectionCount[d]++;

				if (ray.direction[node.sortAxis] > 0)
				{
					int insertPoint = queue.size();
					for (size_t i = node.childIdBegin; i <= node.childIdEnd; i++)
						//for (size_t i = node.childIdEnd; i >= node.childIdBegin; i--)
					{
						CompactNodeV1 c = compactNodesV1[i];
						if (!aabbCheck(ray, c))
						{
							continue;
						}
						queue.insert(queue.begin() + insertPoint, i);
						depths.insert(depths.begin() + insertPoint, d + 1);
					}
				}
				else
				{
					for (size_t i = node.childIdBegin; i <= node.childIdEnd; i++)
					{
						CompactNodeV1 c = compactNodesV1[i];
						if (!aabbCheck(ray, c))
						{
							continue;
						}
						queue.push_back(i);
						depths.push_back(d + 1);
					}
				}

			}
		}
		return result;
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