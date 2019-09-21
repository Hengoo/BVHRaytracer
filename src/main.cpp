#pragma once
#include <iostream>
#include <direct.h>

#include "accelerationStructure/aabb.h"
#include "accelerationStructure/node.h"
#include "accelerationStructure/bvh.h"
#include "camera.h"

#include "glmInclude.h"
#include "modelLoader.h"

#include "lights/light.h"
#include "lights/pointLight.h"
#include "lights/directionalLight.h"
#include "global.h"


//#include "primitives/triangle.h"

/*
	general performance things / todo:
		check inline (there is an optimization option to inline when possible?) --> also learn whats needed for inline to work
		check constexpr and where it can be usefull
		use more const (for safety)
*/

class RayTracer
{
public:
	void run()
	{

	}

	RayTracer()
	{
		unsigned int branchingFactor = 0;
		unsigned int leafCount = 0;

		std::vector<std::shared_ptr<GameObject>> gameObjects;
		gameObjects.push_back(std::make_shared<GameObject>("root"));
		gameObjects[0]->hasParent = true;
		std::vector<std::shared_ptr<MeshBin>> meshBins;
		std::string name;
		std::string path;
		std::string problem;
		int scenario = 4;
		glm::vec3  cameraPos;
		glm::vec3  cameraTarget;

		switch (scenario)
		{
		case 0:
			name = "lizard";
			loadGltfModel("models/Lizard/scene.gltf", gameObjects, meshBins);
			cameraPos = glm::vec3(3.5f, 1.5f, 5.f);
			cameraTarget = glm::vec3(-1, -1, 1.1);

			lights.push_back(std::make_unique<DirectionalLight>(glm::vec3(0, -1, 0), 10));
			break;
		case 1:
			name = "shiftHappens";
			loadGltfModel("models/ShiftHappensTest.glb", gameObjects, meshBins);
			cameraPos = glm::vec3(20, 10, -10);
			cameraTarget = glm::vec3(0, 5, 0);

			lights.push_back(std::make_unique<DirectionalLight>(glm::vec3(0, -1, 0), 10));
			break;
		case 2:
			name = "gearbox";
			//loadGltfModel("models/GearboxAssy.glb", gameObjects, meshBins);
			loadGltfModel("models/GearboxAssyBlenderExport.glb", gameObjects, meshBins);
			cameraPos = glm::vec3(0, 0, 0);
			cameraTarget = glm::vec3(50, 0, 0);

			lights.push_back(std::make_unique<DirectionalLight>(glm::vec3(0, -1, 0), 10));
			break;
		case 3:
			name = "cubes";
			loadGltfModel("models/4Cubes.glb", gameObjects, meshBins);
			cameraPos = glm::vec3(0, 0, -3);
			cameraTarget = glm::vec3(0, 0, 0);

			lights.push_back(std::make_unique<DirectionalLight>(glm::vec3(0, -1, 0), 10));
			break;
		case 4:
			name = "sponza";
			loadGltfModel("models/CasualEffect/sponzaColorful/sponzaColorful.glb", gameObjects, meshBins);
			cameraPos = glm::vec3(-1100, 300, 0);
			cameraTarget = glm::vec3(-900, 290, 0);

			lights.push_back(std::make_unique<DirectionalLight>(glm::vec3(0, -1, 0), 10));
			break;
		default:
			break;
		}
		//create folder to save files:
		//this seems to
		//_mkdir("test");

		//create folder (yeay windwos string shit)
		//std::wstring stemp = std::wstring(searchParam.begin(), searchParam.end());

		path = "Analysis";
		if (CreateDirectory(path.data(), NULL) ||
			ERROR_ALREADY_EXISTS == GetLastError())
		{
			path += "/" + name;
			if (CreateDirectory(path.data(), NULL) ||
				ERROR_ALREADY_EXISTS == GetLastError())
			{

			}
			else
			{
				std::cout << "failed to create directory" << std::endl;
				return;
			}
		}
		else
		{
			std::cout << "failed to create directory" << std::endl;
			return;
		}

		//TODO: loading multiple models might have an error somehwrer?

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
		//gameObjects.clear();
		//meshBins.clear();



		for (size_t l = 4; l <5; l++)
		{
			for (size_t b = 4; b < 5; b++)
			{
				leafCount = l;
				branchingFactor = b; // std::exp2(b);
				std::cout << std::endl << std::endl << "-------------------------------------------------------------------" << std::endl;
				problem = "_b" + std::to_string(branchingFactor) + "_l" + std::to_string(leafCount);
				std::cout << "scenario " << name << " with branching factor of " << std::to_string(branchingFactor) << " and leafsize of " << leafCount << std::endl;


				//bvh of (seeded) random sphere
				//auto bvh = std::make_unique<Bvh>();

				//bvh of loaded model:
				bvh = Bvh(*root);
				bvh.recursiveOctree(branchingFactor, leafCount);
				//bvh.recursiveOctree(2, leafCount);

				//collapses the next b child hierarchies to this node
				//bvh.collapseChilds(b - 1);
				//bvh.collapseChilds(1);
				//for other branching factors we need an other algorithm


				//TODO: gather some bvh stats: node count, average branching factor, average leaf size, tree depth
				bvh.bvhAnalysis(path, name, problem);

				//create camera and render image
				Camera c(path, name, problem, cameraPos, cameraTarget);
				c.renderImage(true, false);
			}
		}
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