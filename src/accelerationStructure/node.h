#pragma once
#include <iostream>
#include <algorithm>
#include <vector>


#include "../ray.h"
#include "../primitives/primitive.h"

class Node
{
public:

	//better protected?
	//if i change this to array i save space IF it is mostly filled. Vector has 24 byte overhead so its not that bad
	std::vector<std::unique_ptr<Node>> children;

	//benefit of primitives is nodes is that it coulds have larger primitives way up -> less duplicates  (if a triangle is in all children nodes, have it directly here)
	std::vector<std::unique_ptr<Primitive>> primitives;

	Node()
	{
	}

	~Node()
	{
	}

	virtual void addNode(std::unique_ptr<Node> n)
	{
		children.push_back(std::move(n));
	}

	virtual void addPrimitive(std::unique_ptr<Primitive> p)
	{
		primitives.push_back(std::move(p));
	}

	virtual bool intersect(std::shared_ptr<Ray> ray)
	{
		//not sure if i need bool result?
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
			if(p->intersect(ray));
			{
				result = true;
			}
		}
		return result;
	}

protected:
	
};
