#pragma once

#include <iostream>
#include <algorithm>
#include <vector>

#include "../ray.h"
#include "node.h"
#include "aabb.h"
#include "../primitives/sphere.h"

class Bvh
{
public:

	Bvh()
	{
		root = std::make_unique<Node>();

		fillBvh();


		//naive idea: make a node with "everything" then iteratively split it up in the middle
		
		//Put everything inside the primitive vector, and split it with each iteration

		//DONT forget to call shrink_to_fit or to clear the primitive vector afterwards
	}

	~Bvh()
	{
	}

	bool intersect(std::shared_ptr<Ray> ray)
	{
		return root->intersect(ray);
	}

	// copy constructor -> called when an already existing object is overwritten by an other
	Bvh(const Bvh& other) = delete;
	// copy assignment -> called when an already existing object is used to create a new object
	Bvh & operator=(const Bvh & other) = delete;


protected:
	std::unique_ptr<Node> root;

private:
	//debug method, just fill bvh with some stuff:
	void fillBvh()
	{
		//auto tmp = std::make_unique<Aabb>(glm::vec3(9, -1, -1), glm::vec3(2, 2, 2));
		auto tmp = std::make_unique<Aabb>(glm::vec3(10, -1, -1), glm::vec3(2, 2, 2));
		tmp->addPrimitive(std::make_unique<Sphere>(glm::vec3(10, 0, 0), 1));
		root->addNode(std::move(tmp));
	}


};
