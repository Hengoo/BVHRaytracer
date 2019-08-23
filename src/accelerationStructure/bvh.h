#pragma once

#include <iostream>
#include <algorithm>
#include <vector>
#include <array>

#include "../ray.h"
#include "node.h"
#include "aabb.h"
#include "../primitives/sphere.h"

class Bvh
{
public:

	Bvh()
	{
		root = std::make_shared<Aabb>();

		randomFillBvh();


		//naive idea: make a node with "everything" then iteratively split it up in the middle

		//Put everything inside the primitive vector, and split it with each iteration

		//DONT forget to call shrink_to_fit or to clear the primitive vector afterwards
	}

	~Bvh()
	{
	}

	bool intersect(Ray& ray)
	{
		return root->intersect(ray);
	}

	// copy constructor -> called when an already existing object is overwritten by an other
	Bvh(const Bvh& other) = delete;
	// copy assignment -> called when an already existing object is used to create a new object
	Bvh& operator=(const Bvh& other) = delete;

	void addPrimitives()
	{
		//adds the primitives to the first node
		//only call BEFORE constructBvh()

	}

	void constructBvh()
	{
		root->constructBvh();
	}


protected:
	std::shared_ptr<Node> root;

private:
	//debug method, just fill bvh with some stuff:
	void randomFillBvh()
	{
		//auto tmp = std::make_shared<Aabb>(glm::vec3(9, -1, -1), glm::vec3(2, 2, 2));
		//auto tmp = std::make_shared<Aabb>(glm::vec3(12, -1, -1), glm::vec3(2, 2, 2));

		//auto p = std::make_shared<Sphere>(glm::vec3(5, 0, 0), 1);
		//root->addPrimitive(p);


		srand(static_cast <unsigned> (42));
		for (int i = 0; i < 100; i++)
		{
			addRandomSphere();
		}

		constructBvh();
	}

	int rint(int min, int max)
	{
		//https://stackoverflow.com/questions/686353/random-float-number-generation
		int r = rand() % (max - min) + min;
		return r;
	}

	float rfloat(float min, float max)
	{
		//https://stackoverflow.com/questions/686353/random-float-number-generation
		float r3 = min + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (max - min)));
		return r3;
	}

	void addRandomSphere()
	{
		float dist = 20;
		std::array<unsigned char, 4> color = { rint(0, 255), rint(0, 255), rint(0, 255), 255 };
		auto p = std::make_shared<Sphere>(glm::vec3(rfloat(-dist, dist), rfloat(-dist, dist), rfloat(-dist, dist)), rfloat(0.2f, 3.0f), color);
		root->addPrimitive(p);
	}




};
