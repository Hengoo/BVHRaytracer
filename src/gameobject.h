#pragma once

#include <iostream>
#include <ostream>
#include <vector>
#include <array>


#include "glmInclude.h"
#include "mesh.h"

//gameobject, contains a mesh and ifferent transforms
class GameObject
{
public:
	//pointer to mesh. one mesh can be used by multiple gameobjects
	std::shared_ptr<Mesh> mesh;

	std::vector<int> childIds;
	std::vector<std::shared_ptr<GameObject>> children;
	//basically only there so i can build the hierarchy and find gameobjects without parents 
	bool hasParent;

	//should/could represent those as transform
	glm::vec3 pos;
	glm::quat rot;
	glm::vec3 scale;
	std::string name;

	//not fully sure how to exactly use those
	glm::mat4 globalTransform;
	glm::mat4 invGlobalTransform;

	GameObject(std::shared_ptr<Mesh> mesh, std::vector<int> childIds, glm::vec3 pos, glm::quat rot, glm::vec3 scale, std::string name)
		:mesh(mesh), childIds(childIds), pos(pos), rot(rot), scale(scale), name(name)
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
		std::cout << "gameobject descruct" << std::endl;
	}

	glm::mat4 getLocalTransform()
	{
		auto transMat = glm::translate(glm::mat4(1.0f), pos);
		auto rotMat = glm::mat4(rot);
		auto scaleMat = glm::scale(glm::mat4(1.0f), scale);
		//first scale, then rot, than trans
		return transMat * rotMat * scaleMat;
	}

	void propagateTransform(glm::mat4 parentGlobalTransform)
	{
		globalTransform = getLocalTransform() * parentGlobalTransform;
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

	void propagateHierarchy(bool fromParent)
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


};