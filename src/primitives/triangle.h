#pragma once
#include <iostream>
#include <vector>
#include <array>
#include "../glmInclude.h"
#include "primitive.h"

#include "../accelerationStructure/aabb.h"
#include "../gameobject.h"
#include "../mesh.h"

class Triangle : public Primitive
{
	//need a triangleMesh that stores the vertices, normals?, textures, uv, ez
	//the mesh also stores the transforms of the object itself

	//better: pointer to vertices -> could save pointer to first vertex INDEX to reduce memory
	//std::array<glm::vec3, 3> points;


	//TODO: definitly not the most efficient version -> can try different ways if more performance is needed?

	//pointer to gameobject and mesh 
	GameObject* gameObject;
	Mesh* mesh;
	//pointer to the first index of the triangle
	//std::shared_ptr<uint32_t> index;
	//id to first indexbuffer (in mesh)
	int index;

	//worldspace bounds:
	glm::vec3 boundMin, boundMax;

	void updateBounds()
	{
		auto t1 = mesh->vertices->operator[](index);
		auto t2 = (*mesh->vertices)[index];
		auto t3 = mesh->vertices->at(index);
		//calc bounds from points:

		/*
		boundMin = glm::min(points[0], points[1]);
		boundMax = glm::max(points[0], points[1]);
		boundMin = glm::min(boundMin, points[2]);
		boundMax = glm::min(boundMax, points[2]);*/
	}

public:
	/*
	Triangle(glm::vec3 pos1, glm::vec3 pos2, glm::vec3 pos3) : points({ pos1, pos2, pos3 })
	{
		updateBounds();
	}*/

	Triangle(GameObject* gameObject, Mesh* mesh, int index)
		:gameObject(gameObject), mesh(mesh), index(index)
	{
		updateBounds();
	}

	~Triangle()
	{
	}

	virtual bool intersect(Ray& ray) override
	{
		return false;
	}

	virtual bool intersect(Node* node) override
	{
		return true;
	}

	virtual void getBounds(glm::vec3& min, glm::vec3& max) override
	{
		min = boundMin;
		max = boundMax;
	}

protected:



};