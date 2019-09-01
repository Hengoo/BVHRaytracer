#pragma once

#include <iostream>
#include <vector>
#include <array>

#include "../ray.h"
#include "node.h"
#include "aabb.h"
#include "../primitives/sphere.h"
#include "../gameobject.h"
#include "../primitives/triangle.h"

#include "../util.h"


class Bvh
{
public:

	Bvh()
	{
		root = std::make_shared<Aabb>(0);

		randomFillBvh();


		//naive idea: make a node with "everything" then iteratively split it up in the middle

		//Put everything inside the primitive vector, and split it with each iteration

		//DONT forget to call shrink_to_fit or to clear the primitive vector afterwards
	}

	Bvh(GameObject& gameObject)
	{
		root = std::make_shared<Aabb>(0);
		//iterate trough gameobject root and add all triangles to the aabb
		iterateGo(gameObject);
	}

	~Bvh()
	{
	}

	bool intersect(Ray& ray)
	{
		if (ray.nodeIntersectionCount.size() < 1)
		{
			ray.nodeIntersectionCount.resize(1);
		}
		ray.nodeIntersectionCount[0]++;
		return root->intersect(ray);
	}

	// copy constructor -> called when an already existing object is overwritten by an other
	//Bvh(const Bvh& other) = delete;
	// copy assignment -> called when an already existing object is used to create a new object
	//Bvh& operator=(const Bvh& other) = delete;

	void recursiveOctree(const unsigned int branchingFactor, const unsigned int leafCount)
	{
		//root->recursiveOctree(leafCount);
		root->recursiveBvh(branchingFactor, leafCount);

		//for better performance: could go trough all nodes and recreate primitives in the order they are in the tree (also with minimal needed data)
	}


protected:
	std::shared_ptr<Node> root;

private:

	void iterateGo(const GameObject& go)
	{
		//for (auto& p : (*go.mesh->vertices))
		if (go.mesh)
		{
			for (int i = 0; i < go.mesh->indices->size(); i += 3)
			{
				//triangle version:
				root->addPrimitive(std::make_shared<Triangle>(&go, &*go.mesh, i));

				//sphere version (one sphere for each triangle?
				//std::array<unsigned char, 4> color = { ruchar(0, 255), ruchar(0, 255), ruchar(0, 255), 255 };
				//auto p = std::make_shared<Sphere>(go.pos, 0.5f, color);
				//root->addPrimitive(p);
			}
		}
		for (auto& g : go.children)
		{
			iterateGo(*g);
		}
	}

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
	}



	void addRandomSphere()
	{
		float dist = 20;
		std::array<unsigned char, 4> color = { ruchar(0, 255), ruchar(0, 255), ruchar(0, 255), 255 };
		auto p = std::make_shared<Sphere>(glm::vec3(rfloat(-dist, dist), rfloat(-dist, dist), rfloat(-dist, dist)), rfloat(0.2f, 3.0f), color);
		root->addPrimitive(p);
	}
};
