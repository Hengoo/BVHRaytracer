#pragma once
#include <iostream>
#include <vector>
#include <array>
#include "../glmInclude.h"
#include "primitive.h"

#include "../accelerationStructure/aabb.h"

class Triangle : public Primitive
{
	std::array<glm::vec3, 3> points;

	glm::vec3 boundMin, boundMax;

	void updateBounds()
	{
		//calc bounds from points:
		boundMax = glm::vec3(-INT_MAX);
		boundMin = glm::vec3(INT_MAX);
		for (auto& p : points)
		{
			boundMin = glm::min(boundMin, p);
			boundMax = glm::min(boundMax, p);
		}
	}

public:

	Triangle(glm::vec3 pos1, glm::vec3 pos2, glm::vec3 pos3) : points({pos1, pos2, pos3})
	{
		updateBounds();
	}

	~Triangle()
	{
	}

	virtual bool intersect(std::shared_ptr<Ray> ray) override
	{	
		return false;
	}

	virtual bool intersect(std::shared_ptr<Node> node) override
	{
		std::shared_ptr<Aabb> aabb = std::dynamic_pointer_cast<Aabb>(node);
		if (aabb)
		{
			//aabb triangle intersection
		}
		
		return true;
	}

	virtual void getBounds(glm::vec3& min, glm::vec3& max) override
	{
		min = boundMin;
		max = boundMax;
	}

protected:



};