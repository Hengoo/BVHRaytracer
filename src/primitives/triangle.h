#pragma once
#include <iostream>
#include <vector>
#include <array>

#include "../glmInclude.h"
#include "primitive.h"

//forward declarations:
class Mesh;
class GameObject;

class Triangle : public Primitive
{
	//need a triangleMesh that stores the vertices, normals?, textures, uv, ez
	//the mesh also stores the transforms of the object itself

	//For now the easy version, better version would not save them here but gather tham from the mesh*
	std::array<glm::vec3, 3> points;


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

	//calc bounds from points:
	void update();


public:
	/*
	Triangle(glm::vec3 pos1, glm::vec3 pos2, glm::vec3 pos3) : points({ pos1, pos2, pos3 })
	{
		updateBounds();
	}*/

	Triangle(GameObject* gameObject, Mesh* mesh, int index)
		:gameObject(gameObject), mesh(mesh), index(index)
	{
		update();
	}


	~Triangle()
	{
	}

	virtual bool intersect(Ray& ray) override;

	virtual bool intersect(Node* node) override;

	virtual void getBounds(glm::vec3& min, glm::vec3& max) override
	{
		min = boundMin;
		max = boundMax;
	}
};