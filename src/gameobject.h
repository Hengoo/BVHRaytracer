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

	void iterateGo(primPointVector& primitives)
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
						primitives.push_back(tri);
					}
				}
			}
		}
		for (auto& g : children)
		{
			g->iterateGo(primitives);
		}
	}
};