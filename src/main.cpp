#pragma once
#include <iostream>

#include "accelerationStructure/aabb.h"
#include "accelerationStructure/node.h"
#include "accelerationStructure/bvh.h"
#include "camera.h"

#include "glmInclude.h"
#include "modelLoader.h"



#include "primitives/triangle.h"

/*
	general performance things / todo:
		check inline (there is an optimization option to inline when possible?) --> also check whats needed for inline to work
		check constexpr and where it can be usefull
		make some stuff const (like most primitive positions(at least triangle))
*/

class RayTracer
{
public:
	void run()
	{
		
	}

	RayTracer()
	{
		std::vector<std::shared_ptr<GameObject>> gameObjects;
		gameObjects.push_back(std::make_shared<GameObject>("root"));
		gameObjects[0]->hasParent = true;
		std::vector<std::shared_ptr<Mesh>> meshes;
		loadGltfModel("models/OrientationTest.glb", gameObjects, meshes);
		//loadGltfModel("models/Triangle.glb", gameObjects, meshes);

		for (auto& go : gameObjects)
		{
			for (auto& c : go->childIds)
			{
				gameObjects[c]->hasParent = true;
				go->children.push_back(gameObjects[c]);
			}
		}
		for (auto& go : gameObjects)
		{
			if (!go->hasParent)
			{
				gameObjects[0]->children.push_back(go);
			}
			//clear ids because there is no use to keep them
			go->childIds.clear();
		}
		auto root = gameObjects[0];
		root->propagateTransform();

		//dont need those anymore
		gameObjects.clear();
		meshes.clear();

		//IMPORTANT!!!! Bvh b = Bvh() calls the copy constructor
		//Camera c(std::make_unique<Bvh>(), glm::vec3(0,-5,0), glm::vec3(1, -0.5, -0.5));
		
		//Camera c(std::make_unique<Bvh>(*root), glm::vec3(0, 5, 5), glm::vec3(1, -0.5, -0.5));
		//Camera c(std::make_unique<Bvh>(*root), glm::vec3(0, 50, 0), glm::vec3(1, -0.5, -0.5));
		Camera c(std::make_unique<Bvh>(*root), glm::vec3(0, 10, 0), glm::vec3(0.1, -1., 0));
		c.renderImage();
	}

private:

};

int main()
{
	//HelloTriangleApplication app;
	RayTracer rayTracer;

	//need to check performance impact of this:
	try
	{
		rayTracer.run();
		std::cout << "end of program" << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}