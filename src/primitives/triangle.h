#pragma once
#include <iostream>
#include <vector>
#include <array>

#include "../glmInclude.h"
#include "../glmUtil.h"
#include "primitive.h"
#include "../accelerationStructure/aabb.h"
#include "../gameobject.h"
#include "../mesh.h"

class Triangle : public Primitive
{
	//need a triangleMesh that stores the vertices, normals?, textures, uv, ez
	//the mesh also stores the transforms of the object itself

	//For now the easy version, alternative would be to have a reference to the 
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
	void update()
	{
		//why:
		//auto t1 = mesh->vertices->operator[](index);
		//auto t2 = (*mesh->vertices)[index];
		//auto t3 = mesh->vertices->at(index);

		glm::vec4 pos0((*mesh->vertices)[(*mesh->indices)[index + 0]].pos, 1);
		glm::vec4 pos1((*mesh->vertices)[(*mesh->indices)[index + 1]].pos, 1);
		glm::vec4 pos2((*mesh->vertices)[(*mesh->indices)[index + 2]].pos, 1);
		//points[0] = (gameObject->globalTransform * pos0).xyz;
		//points[1] = (gameObject->globalTransform * pos1).xyz;
		//points[2] = (gameObject->globalTransform * pos2).xyz;
		points[0] = glm::vec3(gameObject->globalTransform * pos0);
		points[1] = glm::vec3(gameObject->globalTransform * pos1);
		points[2] = glm::vec3(gameObject->globalTransform * pos2);



		boundMin = glm::min(points[0], points[1]);
		boundMax = glm::max(points[0], points[1]);
		boundMin = glm::min(boundMin, points[2]);
		boundMax = glm::max(boundMax, points[2]);
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
		update();
	}

	~Triangle()
	{
	}

	virtual bool intersect(Ray& ray) override
	{
		//mostly from here: http://www.pbr-book.org/3ed-2018/Shapes/Triangle_Meshes.html
		//code here https://github.com/mmp/pbrt-v3/blob/master/src/shapes/triangle.cpp
		//it seems very long but every watertight algorithm i found is similarly complex long so i will give this one a try

		//transform vertices to ray coordinate space:
		glm::vec3 p0t = points[0] - ray.pos;
		glm::vec3 p1t = points[1] - ray.pos;
		glm::vec3 p2t = points[2] - ray.pos;

		//permute components  of triangle vertices and ray direction
		glm::vec3 t = glm::abs(ray.direction);
		int kz = maxDimension(t);
		int kx = kz + 1; if (kx == 3) kx = 0;
		int ky = kx + 1; if (ky == 3) ky = 0;
		glm::vec3 d = permute(ray.direction, kx, ky, kz);
		p0t = permute(p0t, kx, ky, kz);
		p1t = permute(p1t, kx, ky, kz);
		p2t = permute(p2t, kx, ky, kz);

		//apply shear transformation to translated vertex positions
		float Sx = -d.x / d.z;
		float Sy = -d.y / d.z;
		float Sz = 1.f / d.z;
		p0t.x += Sx * p0t.z;
		p0t.y += Sy * p0t.z;
		p1t.x += Sx * p1t.z;
		p1t.y += Sy * p1t.z;
		p2t.x += Sx * p2t.z;
		p2t.y += Sy * p2t.z;

		// Compute edge function coefficients _e0_, _e1_, and _e2_
		float e0 = p1t.x * p2t.y - p1t.y * p2t.x;
		float e1 = p2t.x * p0t.y - p2t.y * p0t.x;
		float e2 = p0t.x * p1t.y - p0t.y * p1t.x;
		
		// Fall back to double precision test at triangle edges
		//TODO copy



		// Perform triangle edge and determinant tests
		if ((e0 < 0 || e1 < 0 || e2 < 0) && (e0 > 0 || e1 > 0 || e2 > 0))
			return false;
		float det = e0 + e1 + e2;
		if (det == 0) return false;

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