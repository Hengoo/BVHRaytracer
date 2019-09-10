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
#include "../typedef.h"


class Bvh
{
public:

	Bvh()
	{
		//root = std::make_shared<Aabb>(0);

		//randomFillBvh();


		//naive idea: make a node with "everything" then iteratively split it up in the middle

		//Put everything inside the primitive vector, and split it with each iteration

		//DONT forget to call shrink_to_fit or to clear the primitive vector afterwards
	}

	Bvh(GameObject& gameObject)
	{
		//iterate trough gameobject root and add all triangles to the aabb
		std::shared_ptr<primPointVector> primitives = std::make_shared<primPointVector>();
		iterateGo(gameObject, primitives);
		root = std::make_shared<Aabb>(0, primitives, primitives->begin(), primitives->end());

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
		float dist = 0;
		if (root->intersectNode(ray, dist))
		{
			return root->intersect(ray);
		}
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

	//doubles the childen in a way that childcount children are in each node
	void collapseChilds(int childCount)
	{
		//default
		if (childCount == 0)
		{
			collapseChilds(root);
		}
		//need to implement special algotithm for 3, ...
		//current idea is to reshuffle based on the ammound of primitives so the tree is kinda balanced
		
	}



protected:
	std::shared_ptr<Node> root;

private:

	//doubles the ammound of children in each node by merging each node with its own children
	void collapseChilds(std::shared_ptr<Node> node)
	{
		//this assumes there are only primitives in the leaf nodes
		if (node->getPrimCount() != 0)
		{
			return;
		}
		std::vector<std::shared_ptr<Node>> newChildren;
		for (auto& child : node->children)
		{
			if (child->getPrimCount() == 0)
			{
				newChildren.insert(newChildren.end(), child->children.begin(), child->children.end());
			}
			else
			{
				newChildren.push_back(child);
			}

		}
		node->children = newChildren;
		//this could be parrallel (i dont think its needed)
		for (auto& child : node->children)
		{
			child->depth = node->depth + 1;
			collapseChilds(child);
		}
	}

	void iterateGo(const GameObject& go, std::shared_ptr<primPointVector>& primitives)
	{
		//for (auto& p : (*go.mesh->vertices))
		if (go.mesh)
		{
			for (int i = 0; i < go.mesh->indices->size(); i += 3)
			{
				//triangle version:
				primitives->push_back(std::make_shared<Triangle>(&go, &*go.mesh, i));

				//sphere version (one sphere for each triangle?
				//std::array<unsigned char, 4> color = { ruchar(0, 255), ruchar(0, 255), ruchar(0, 255), 255 };
				//auto p = std::make_shared<Sphere>(go.pos, 0.5f, color);
				//root->addPrimitive(p);
			}
		}
		for (auto& g : go.children)
		{
			iterateGo(*g, primitives);
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



	std::shared_ptr<Primitive> addRandomSphere()
	{
		float dist = 20;
		std::array<unsigned char, 4> color = { ruchar(0, 255), ruchar(0, 255), ruchar(0, 255), 255 };
		return std::make_shared<Sphere>(glm::vec3(rfloat(-dist, dist), rfloat(-dist, dist), rfloat(-dist, dist)), rfloat(0.2f, 3.0f), color);
	}
};
