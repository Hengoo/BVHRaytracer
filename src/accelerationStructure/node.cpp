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

	std::for_each(primitiveBegin, primitiveEnd,
		[&](auto& p)
		{
			if (ray.primitiveIntersectionCount.size() < depth + 2)
			{
				ray.primitiveIntersectionCount.resize(depth + 2);
			}
			ray.primitiveIntersectionCount[depth + 1]++;
			if (p->intersect(ray))
			{
				result = true;
				if (ray.shadowRay)
				{
					return true;
				}
			}
		});

	for (auto& c : children)
	{
		if (ray.nodeIntersectionCount.size() < depth + 2)
		{
			ray.nodeIntersectionCount.resize(depth + 2);
		}
		ray.nodeIntersectionCount[depth + 1]++;
		float dist;
		if (c->intersectNode(ray, dist))
		{
			if (c->intersect(ray))
			{
				result = true;
				if (ray.shadowRay)
				{
					return true;
				}
			}
		}
	}

	//this has less intersections but is SLOWER (38 seconds instead of 31 seconds for 8 times the shift happens)
	//TODO: want to retest this with a larger branching factor(and probably only for nodes with depth under 5?)
	/*
	//idea: sort nodes by distance to them -> traverse closer ones first
	std::vector<DistanceNode> d;
	//TODO: need to check if its faster to reserve or not
	//d.reserve(children.size());

	for (size_t i = 0; i < children.size(); i++)
	{
		if (ray.nodeIntersectionCount.size() < depth + 2)
		{
			ray.nodeIntersectionCount.resize(depth + 2);
		}
		ray.nodeIntersectionCount[depth + 1]++;
		float dist;
		if (children[i]->intersectNode(ray, dist))
		{
			d.push_back(DistanceNode(i, dist));
		}
	}

	std::sort(d.begin(), d.end(), sortDistanceNode);
	for (auto& di : d)
	{
		if (children[di.id]->intersect(ray))
		{
			result = true;
			if (ray.shadowRay)
			{
				return true;
			}
		}
	}*/

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
