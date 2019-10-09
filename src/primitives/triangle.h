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
	//pointer to gameobject and mesh 
	const GameObject* gameObject;
	const Mesh* mesh;

	//transformed point coordinates (its faster to save than to recompute)
	std::array<glm::vec3, 3> points = {};

	//id to first indexbuffer (in mesh)
	uint32_t index;

	//worldspace bounds: (will keep bounds for super simple aabb to aabb intersection)
	glm::vec3 boundMin, boundMax;
	//its faster to save it than the recompute it
	glm::vec3 center;

	//update bounds from points:
	void update();


public:
	/*
	Triangle(glm::vec3 pos1, glm::vec3 pos2, glm::vec3 pos3) : points({ pos1, pos2, pos3 })
	{
		updateBounds();
	}*/

	Triangle(const GameObject* gameObject, const Mesh* mesh, int index)
		:gameObject(gameObject), mesh(mesh), index(index)
	{
		update();
	}


	~Triangle()
	{
	}

	virtual bool intersect(Ray& ray) override;

	virtual bool intersect(Node* node) override;

	virtual inline void getBounds(glm::vec3& min, glm::vec3& max) override
	{
		min = boundMin;
		max = boundMax;
	}

	virtual inline glm::vec3 getCenter() override
	{
		//return (boundMax * 0.5f + boundMin * 0.5f);
		return center;
	}

	void getVertexIds(uint32_t& vertex0, uint32_t& vertex1, uint32_t& vertex2);
};