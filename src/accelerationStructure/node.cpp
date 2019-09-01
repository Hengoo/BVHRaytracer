#pragma once
#include <iostream>
#include <algorithm>
#include <vector>
//for the parallel for
#include <execution>

#include "node.h"
#include "../primitives/primitive.h"
#include "../ray.h"

void Node::addNode(std::shared_ptr<Node> n)
{
	children.push_back(n);
}

void Node::addPrimitive(std::shared_ptr<Primitive> p)
{
	primitives.push_back(p);
}

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

	for (auto& p : primitives)
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
	}

	for (auto& c : children)
	{
		if (ray.nodeIntersectionCount.size() < depth + 2)
		{
			ray.nodeIntersectionCount.resize(depth + 2);
		}
		ray.nodeIntersectionCount[depth + 1]++;
		if (c->intersect(ray))
		{
			result = true;
			if (ray.shadowRay)
			{
				return true;
			}
		}
	}



	return result;
}

int Node::getChildCount()
{
	return children.size();
}

int Node::getPrimCount()
{
	return primitives.size();
}
