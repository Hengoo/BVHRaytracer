#pragma once

#include <iostream>
#include <ostream>
#include <vector>
#include <array>

#include "typedef.h"
#include "glmInclude.h"
#include "mesh.h"
#include "meshBin.h"
#include "primitives/triangle.h"

class MeshBin;

//gameobject, contains a mesh and ifferent transforms
class GameObject
{
public:
	//pointer to mesh. one mesh can be used by multiple gameobjects
	std::shared_ptr<MeshBin> meshBin;

	std::vector<int> childIds;
	std::vector<std::shared_ptr<GameObject>> children;
	//basically only there so i can build the hierarchy and find gameobjects without parents 
	bool hasParent;

	//should/could represent those as transform
	glm::vec3 pos;
	glm::quat rot;
	glm::vec3 scale;
	const std::string name;

	//not sure if i need both
	glm::mat4 globalTransform;
	glm::mat4 invGlobalTransform;

	GameObject(std::shared_ptr<MeshBin> meshBin, std::vector<int> childIds, glm::vec3 pos, glm::quat rot, glm::vec3 scale, std::string name)
		:meshBin(meshBin), childIds(childIds), pos(pos), rot(rot), scale(scale), name(name)
	{
		globalTransform = glm::mat4(1);
		invGlobalTransform = glm::mat4(1);
		hasParent = false;
	}

	//empty gameobject with name ->used for at least the root
	GameObject(std::string name)
		: name(name)
	{
		pos = glm::vec3(0);
		rot = glm::mat4(1.0f);
		scale = glm::vec3(1);
		globalTransform = glm::mat4(1);
		invGlobalTransform = glm::mat4(1);
		hasParent = false;
	}
	~GameObject()
	{
		//std::cout << "gameobject descruct" << std::endl;
	}

	glm::mat4 getLocalTransform()
	{
		auto transMat = glm::translate(glm::mat4(1.0f), pos);
		auto rotMat = glm::mat4(rot);
		auto scaleMat = glm::scale(glm::mat4(1.0f), scale);
		//first scale, then rot, than trans
		return transMat * rotMat * scaleMat;
		//return  scaleMat * rotMat * transMat;
	}

	void propagateTransform(const glm::mat4& parentGlobalTransform)
	{
		globalTransform = parentGlobalTransform * getLocalTransform();
		invGlobalTransform = glm::inverse(globalTransform);
		for (auto& c : children)
		{
			c->propagateTransform(globalTransform);
		}
	}

	void propagateTransform()
	{
		propagateTransform(glm::mat4(1));
	}

	void propagateHierarchy(const bool& fromParent)
	{
		if (fromParent)
		{
			hasParent = true;
		}
		else
		{
			for (auto& c : children)
			{
				c->propagateHierarchy(true);
			}
		}
	}

	//recenters and rescales gameobject so that aabb goes from ~ (-1,-1-,1) to (1,1,1)
	void recenter(const glm::vec3& boundMin, const glm::vec3& boundMax)
	{
		pos = (boundMin + boundMax) * 0.5f;
		scale = glm::vec3(1) / (boundMin - boundMax);
	}

	//iterates all gameobjects and puts them into the primitive vector. Subdivision describes the extra number of trinagles we generate
	void iterateGo(primPointVector& primitives, int subdivision)
	{
		if (meshBin)
		{
			for (auto& m : meshBin->meshes)
			{
				for (int i = 0; i < m->indices->size(); i += 3)
				{
					//triangle version:
					auto tri = std::make_shared<Triangle>(this, m.get(), i);
					glm::vec3 v0, v1, v2;
					tri->getVertexPositions(v0, v1, v2);
					if (calcTriangleSurfaceArea(v0, v1, v2) != 0)
					{
						//Its important to conserve the normal of the triangles!
						if (subdivision == 0)
						{
							primitives.push_back(tri);
						}
						else
						{
							addSubdividedTri(primitives, v0, v1, v2, m, i, subdivision);
						}
					}
				}
			}
		}
		for (auto& g : children)
		{
			g->iterateGo(primitives, subdivision);
		}
	}

	//adds the subdivided triangle to the primitive list.
	inline void addSubdividedTri(primPointVector& primitives, glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, std::shared_ptr<Mesh>& m, int i, int subdivision)
	{
		//TODO use recusion for larger combis (would be intresting to scale a small scene like sub10 or more)
		if (subdivision == 1)
		{
			//add one triangle by splitting the existing one in half at the largest distance.
			std::array<float, 3> l = { glm::length(v0 - v1), glm::length(v0 - v1), glm::length(v0 - v1) };
			if (l[0] >= l[1] && l[0] >= l[2])
			{
				auto vNew = (v0 + v1) / 2.0f;
				std::array<glm::vec3, 3> points = { v0, vNew, v2 };
				primitives.push_back(std::make_shared<Triangle>(this, m.get(), i, points));
				points = { v1, v2, vNew };
				primitives.push_back(std::make_shared<Triangle>(this, m.get(), i, points));

			}
			else if (l[1] >= l[0] && l[1] >= l[2])
			{
				auto vNew = (v1 + v2) / 2.0f;
				std::array<glm::vec3, 3> points = { v0, v1, vNew };
				primitives.push_back(std::make_shared<Triangle>(this, m.get(), i, points));
				points = { v0, vNew, v2 };
				primitives.push_back(std::make_shared<Triangle>(this, m.get(), i, points));
			}
			else
			{
				auto vNew = (v2 + v0) / 2.0f;
				std::array<glm::vec3, 3> points = { v0, v1, vNew };
				primitives.push_back(std::make_shared<Triangle>(this, m.get(), i, points));
				points = { v1, v2, vNew };
				primitives.push_back(std::make_shared<Triangle>(this, m.get(), i, points));
			}
		}
		else if (subdivision == 2)
		{
			//add two triangle by adding an extra point in the middle
			glm::vec3 vm = (v0 + v1 + v2) / 3.0f;
			std::array<glm::vec3, 3> points = { v0, v1, vm };
			primitives.push_back(std::make_shared<Triangle>(this, m.get(), i, points));
			points = { v1, v2, vm };
			primitives.push_back(std::make_shared<Triangle>(this, m.get(), i, points));
			points = { v0, vm, v2 };
			primitives.push_back(std::make_shared<Triangle>(this, m.get(), i, points));
		}
		else if (subdivision == 3)
		{
			//add three triangles by splitting each edge at half
			glm::vec3 v05 = (v0 + v1) / 2.0f;
			glm::vec3 v15 = (v1 + v2) / 2.0f;
			glm::vec3 v25 = (v2 + v0) / 2.0f;
			std::array<glm::vec3, 3> points = { v0, v05, v25 };
			primitives.push_back(std::make_shared<Triangle>(this, m.get(), i, points));
			points = { v05, v1, v15 };
			primitives.push_back(std::make_shared<Triangle>(this, m.get(), i, points));
			points = { v15, v2, v25 };
			primitives.push_back(std::make_shared<Triangle>(this, m.get(), i, points));
			points = { v05, v15, v25 };
			primitives.push_back(std::make_shared<Triangle>(this, m.get(), i, points));
		}
		
	}
};