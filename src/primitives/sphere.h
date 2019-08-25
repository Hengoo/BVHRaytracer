#pragma once
#include <iostream>
#include <vector>
#include <array>

#include "../glmInclude.h"
#include "primitive.h"

/*
most simple primitive, only implenented to see first results and get compfortable with structure. 
-> will mostly likely not use it when triable meshes are implemented
--> might copy parts of it for an possible aabb replacement (just a test)
*/
class Sphere : public Primitive
{
public:
	glm::vec3 pos;
	float radius;

	std::array<unsigned char,4> color;

	Sphere(glm::vec3 pos, float radius, std::array<unsigned char, 4> color) : pos(pos), radius(radius), color(color)
	{
	}

	~Sphere()
	{
	}

	virtual bool intersect(Ray& ray) override
	{


		glm::vec3 oc = ray.pos - pos;
		float a = glm::dot(ray.direction, ray.direction);
		float b = 2.0f * glm::dot(oc, ray.direction);
		float c = glm::dot(oc, oc) - radius * radius;
		float discriminant = b * b - 4.0f * a * c;
		if (discriminant < 0)
		{
			return false;
		}
		else
		{
			//std::cout << "sphere intersect true" << std::endl;
			float dist = (-b - sqrt(discriminant)) / (2.0f * a);
			if (dist < ray.tMax)
			{
				//ray.result = {0,0,0,255};
				auto tmp = ray.pos * 10.0f + ray.direction * dist;
				//ray.result = { (unsigned char)tmp.x,(unsigned char)tmp.y, (unsigned char)tmp.z,255 };
				ray.result = color;
				ray.tMax = dist;
			}
			return true;
		}
	}

	virtual bool intersect(Node* node) override
	{
		//i really dislike how this turned out but i need to determine what primitive to intersect with what node.
		Aabb* aabb = dynamic_cast<Aabb*>(node);
		if (aabb)
		{
			//aabb sphere intersection

			//https://stackoverflow.com/questions/4578967/cube-sphere-intersection-test
			float r2 = radius * radius;
			if (pos.x < aabb->boundMin.x) r2 -= pow(pos.x - aabb->boundMin.x, 2);
			else if (pos.x > aabb->boundMax.x) r2 -= pow(pos.x - aabb->boundMax.x, 2);

			if (pos.y < aabb->boundMin.y) r2 -= pow(pos.y - aabb->boundMin.y, 2);
			else if (pos.y > aabb->boundMax.y) r2 -= pow(pos.y - aabb->boundMax.y, 2);

			if (pos.z < aabb->boundMin.z) r2 -= pow(pos.z - aabb->boundMin.z, 2);
			else if (pos.z > aabb->boundMax.z) r2 -= pow(pos.z - aabb->boundMax.z, 2);

			return r2 > 0;
		}

		//no derived type-> should not happen
		return false;
	}

	virtual void getBounds(glm::vec3& min, glm::vec3& max) override
	{
		min = pos - glm::vec3(radius);
		max = pos + glm::vec3(radius);
	}

protected:



};