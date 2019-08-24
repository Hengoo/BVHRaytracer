#pragma once
#include <iostream>
#include <algorithm>
#include <vector>

#include "node.h"
#include "../primitives/primitive.h"

void Node::addNode(std::shared_ptr<Node> n)
{
	children.push_back(n);
}

void Node::addPrimitive(std::shared_ptr<Primitive> p)
{
	primitives.push_back(p);
}

void Node::constructBvh()
{
	for (auto& c : children)
	{
		c->constructBvh();
	}
}

bool Node::intersect(Ray& ray)
{
	//not sure if i need bool results? i store result in the ray
	bool result = false;
	for (auto& c : children)
	{
		if (c->intersect(ray))
		{
			result = true;
		}
	}

	for (auto& p : primitives)
	{
		if (p->intersect(ray))
		{
			result = true;
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
