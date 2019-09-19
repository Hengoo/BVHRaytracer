#pragma once
#include <iostream>
#include <algorithm>
#include <vector>
//for the parallel for
#include <execution>

#include "node.h"
#include "../primitives/primitive.h"
#include "../ray.h"

struct DistanceNode
{
	int id;
	float distance;

	DistanceNode()
	{
		id = -1;
		distance = -1;
	}
	DistanceNode(int id, float distance)
		:id(id), distance(distance)
	{
	}
};
static bool sortDistanceNode(DistanceNode& d1, DistanceNode& d2)
{
	return d1.distance < d2.distance;
}

void Node::addNode(std::shared_ptr<Node> n)
{
	children.push_back(n);
}

//void Node::addPrimitive(std::shared_ptr<Primitive> p)
//{
//	primitives.push_back(p);
//}

void Node::recursiveBvh(const unsigned int branchingFactor, const unsigned int leafCount)
{
	std::for_each(std::execution::par_unseq, children.begin(), children.end(),
		[&](auto& c)
		{
			c->recursiveBvh(branchingFactor, leafCount);
		});
}

void Node::recursiveOctree(const unsigned int leafCount)
{
	std::for_each(std::execution::par_unseq, children.begin(), children.end(),
		[&](auto& c)
		{
			c->recursiveOctree(leafCount);
		});
}

bool Node::intersect(Ray& ray)
{
	bool result = false;

	if (getPrimCount() != 0)
	{
		//save primitivecount
		//so we know how much we space we waste (and how efficiently we use cachelines)
		if (ray.primitiveFullness.size() < getPrimCount() + 1)
		{
			ray.primitiveFullness.resize(getPrimCount() + 1);
		}
		ray.primitiveFullness[getPrimCount()] ++;

		if (ray.leafIntersectionCount.size() < depth + 1)
		{
			ray.leafIntersectionCount.resize(depth + 1);
		}
		ray.leafIntersectionCount[depth]++;

		//std::all_of stops loop when false is returned
		if (!ray.shadowRay)
		{
			std::for_each(primitiveBegin, primitiveEnd,
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
			return std::any_of(primitiveBegin, primitiveEnd,
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
		}

	}

	if (getChildCount() != 0)
	{
		//save childcount of this intersection
		if (ray.childFullness.size() < getChildCount() + 1)
		{
			ray.childFullness.resize(getChildCount() + 1);
		}
		ray.childFullness[getChildCount()] ++;
		//increment node intersection counter
		if (ray.nodeIntersectionCount.size() < depth + 1)
		{
			ray.nodeIntersectionCount.resize(depth + 1);
		}
		ray.nodeIntersectionCount[depth]++;

		if (getPrimCount() > 0)
		{
			std::cout << "TODO: implement correct counter for primitive intersection in upper nodes" << std::endl;
		}

		std::vector<std::shared_ptr<Node>>::iterator begin = children.begin();
		std::vector<std::shared_ptr<Node>>::iterator end = children.end();

		//code duplication because it is about 10% faster to do it this way instead of calling a method in lambda or using std::bind
		if (ray.direction[sortAxis] > 0)
		{
			if (!ray.shadowRay)
			{
				std::for_each(children.begin(), children.end(),
					[&](auto& c)
					{
						ray.aabbIntersectionCount++;
						float dist;
						if (c->intersectNode(ray, dist))
						{
							ray.successfulAabbIntersectionCount++;

							//node intersection successful: rekursion continues
							if (c->intersect(ray))
							{
								result = true;
							}
						}
					});
			}
			else
			{
				return std::any_of(children.begin(), children.end(),
					[&](auto& c)
					{
						ray.aabbIntersectionCount++;
						float dist;
						if (c->intersectNode(ray, dist))
						{
							ray.successfulAabbIntersectionCount++;

							//node intersection successful: rekursion continues
							if (c->intersect(ray))
							{
								return true;
							}
						}
						return false;
					});
			}
		}
		else
		{
			if (!ray.shadowRay)
			{
				std::for_each(children.rbegin(), children.rend(),
					[&](auto& c)
					{
						ray.aabbIntersectionCount++;
						float dist;
						if (c->intersectNode(ray, dist))
						{
							ray.successfulAabbIntersectionCount++;

							//node intersection successful: rekursion continues
							if (c->intersect(ray))
							{
								result = true;
							}
						}
					});
			}
			else
			{
				return std::any_of(children.rbegin(), children.rend(),
					[&](auto& c)
					{
						ray.aabbIntersectionCount++;
						float dist;
						if (c->intersectNode(ray, dist))
						{
							ray.successfulAabbIntersectionCount++;

							//node intersection successful: rekursion continues
							if (c->intersect(ray))
							{
								return true;
							}
						}
						return false;
					});
			}
		}
	}
	return result;
}

unsigned int Node::getChildCount()
{
	return children.size();
}

unsigned int Node::getPrimCount()
{
	return std::distance(primitiveBegin, primitiveEnd);
}