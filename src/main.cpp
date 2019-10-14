#pragma once
#include <iostream>
#include <direct.h>

#include "accelerationStructure/aabb.h"
#include "accelerationStructure/node.h"
#include "accelerationStructure/bvh.h"
#include "accelerationStructure/compactNode.h"
#include "camera.h"

#include "glmInclude.h"
#include "modelLoader.h"

#include "lights/light.h"
#include "lights/pointLight.h"
#include "lights/directionalLight.h"

//includes for the timer
#include <ctime>
#include <ratio>
#include <chrono>

//#include "primitives/triangle.h"

/*
	general performance things / todo:
		check inline (there is an optimization option to inline when possible?) --> also learn whats needed for inline to work
		check constexpr and where it can be usefull
		use more const (for safety)
*/

class RayTracer
{
	unsigned minLeafSize;
	unsigned maxLeafSize;
	unsigned minBranch;
	unsigned maxBranch;

	bool saveImage;
	bool saveDepthDetailedImage;
	bool bvhAnalysis;
	bool saveBvhImage;

	//true -> take new axis and sort for each split. False -> only do it once in the beginning
	bool sortEachSplit;

	//0 = bvh tree traversal, 1 = compact node, 2 = compact node immediate
	unsigned renderType;
	std::vector<unsigned> scenarios;
	unsigned bucketCount;

	//0 = custom order, 1 = level, 2 = depth first,
	unsigned compactNodeOrder;
public:

	RayTracer()
	{
	}

	void run()
	{
		std::chrono::high_resolution_clock::time_point timeBegin = std::chrono::high_resolution_clock::now();
		//working progress options that should go to config soon:


		//settings sanity checks:
		maxBranch = std::max(maxBranch, minBranch);
		minBranch = std::min(maxBranch, minBranch);
		maxLeafSize = std::max(maxLeafSize, minLeafSize);
		minLeafSize = std::min(maxLeafSize, minLeafSize);
		if (renderType > 2)
		{
			std::cerr << "unknown renderType" << std::endl;
			return;
		}
		if (compactNodeOrder > 2)
		{
			std::cerr << "unknown compactNodeOrder" << std::endl;
			return;
		}

		std::cout << "Settings: " << std::endl;
		std::cout << "LeafSize from " << minLeafSize << " to " << maxLeafSize << std::endl;
		std::cout << "Branching factor from " << minBranch << " to " << maxBranch << std::endl;
		if (saveImage) std::cout << "save image" << std::endl;
		if (saveDepthDetailedImage) std::cout << "save DepthDetailedImage" << std::endl;
		if (bvhAnalysis) std::cout << "do bvhAnalysis" << std::endl;
		if (saveBvhImage) std::cout << "save BvhImage" << std::endl;
		std::cout << "render type: " << renderType << std::endl;
		for (auto& s : scenarios)
		{
			std::cout << "scenario: " << s << std::endl;
		}
		std::cout << "bucket count: " << bucketCount << std::endl;
		std::cout << "compact Node order: " << compactNodeOrder << std::endl;
		std::cout << std::endl;

		if (scenarios.size() > 1)
		{
			//mute cout
			std::cout.setstate(std::ios_base::failbit);
		}
		std::for_each(std::execution::par_unseq, scenarios.begin(), scenarios.end(),
			[&](auto& scenario)
			{
				std::string name;
				std::string path;
				glm::vec3  cameraPos;
				glm::vec3  cameraTarget;

				std::vector < std::unique_ptr<Light>> lights;

				std::vector<std::shared_ptr<GameObject>> gameObjects;
				gameObjects.push_back(std::make_shared<GameObject>("root"));
				gameObjects[0]->hasParent = true;
				std::vector<std::shared_ptr<MeshBin>> meshBins;

				std::chrono::high_resolution_clock::time_point time1 = std::chrono::high_resolution_clock::now();

				//reminder : blender coordiantes of this (0, 360, 50) are -> glm::vec3(0, 50, -360);
				switch (scenario)
				{
				case 0:
					//https://sketchfab.com/3d-models/lizard-mage-817d52d9887948bfa0ca43aef6064eaa
					name = "lizard";
					loadGltfModel("models/Lizard/scene.gltf", gameObjects, meshBins);
					cameraPos = glm::vec3(3.5f, 1.5f, 5.f);
					cameraTarget = glm::vec3(-1, -1, 1.1);

					lights.push_back(std::make_unique<DirectionalLight>(glm::vec3(0, -1, 0), 10));
					break;
				case 1:
					//https://sketchfab.com/3d-models/shift-happens-canyon-diorama-ffd36dfbfda8432d97388988883f6295
					name = "shiftHappens";
					loadGltfModel("models/ShiftHappensTest.glb", gameObjects, meshBins);
					cameraPos = glm::vec3(20, 10, -10);
					cameraTarget = glm::vec3(0, 5, 0);

					lights.push_back(std::make_unique<DirectionalLight>(glm::vec3(0, -1, 0), 10));
					break;
				case 2:
					//http://casual-effects.com/data/index.html
					//erato scene converted to glb with blender (replaced some #indoo with 0 so blender could load it?)
					name = "erato";
					loadGltfModel("models/erato/erato.glb", gameObjects, meshBins);
					cameraPos = glm::vec3(10, 6, 9);
					cameraTarget = glm::vec3(-3, 1.5, 0);

					lights.push_back(std::make_unique<DirectionalLight>(glm::vec3(0, -1, 0), 10));
					break;
				case 3:
					//just some cubes for debuging single rays
					name = "cubes";
					loadGltfModel("models/4Cubes.glb", gameObjects, meshBins);
					cameraPos = glm::vec3(0, 0, -3);
					cameraTarget = glm::vec3(0, 0, 0);

					lights.push_back(std::make_unique<DirectionalLight>(glm::vec3(0, -1, 0), 10));
					break;
				case 4:
					//http://casual-effects.com/data/index.html
					//the "Crytek Sponza"
					name = "sponza";
					loadGltfModel("models/sponzaColorful/sponzaColorful.glb", gameObjects, meshBins);
					cameraPos = glm::vec3(-1100, 300, 0);
					cameraTarget = glm::vec3(-900, 290, 0);

					lights.push_back(std::make_unique<DirectionalLight>(glm::vec3(0, -1, 0), 10));
					break;

				case 5:
					//https://sketchfab.com/3d-models/davia-rocks-b8576a61715a4feabd9637215eeb2e05
					name = "daviaRock";
					loadGltfModel("models/davia_rocks/scene.gltf", gameObjects, meshBins);
					cameraPos = glm::vec3(10, 2, 2);
					cameraTarget = glm::vec3(0, 0, 0);

					lights.push_back(std::make_unique<DirectionalLight>(glm::vec3(0, -1, 0), 10));
					break;
				case 6:
					//http://casual-effects.com/data/index.html
					//rungholt scene converted to glb with blender (took ages)
					name = "rungholt";
					loadGltfModel("models/rungholt/rungholt.glb", gameObjects, meshBins);

					cameraPos = glm::vec3(-100, 41, -200);
					cameraTarget = glm::vec3(-140, 0, -70);

					lights.push_back(std::make_unique<DirectionalLight>(glm::vec3(-0.3, -1, -0.1), 10));
					break;
				default:
					std::cerr << "unknown scene id" << std::endl;
					return;
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
						std::cerr << "failed to create directory" << std::endl;
						return;
					}
				}
				else
				{
					std::cerr << "failed to create directory" << std::endl;
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

				std::chrono::high_resolution_clock::time_point time2 = std::chrono::high_resolution_clock::now();
				std::chrono::duration<double> time_span0 = std::chrono::duration_cast<std::chrono::duration<double>>(time2 - time1);
				std::cout << std::endl << "Model loading took " << time_span0.count() << " seconds." << std::endl;

				std::vector<unsigned> leafWork(maxLeafSize - minLeafSize + 1);
				std::iota(std::begin(leafWork), std::end(leafWork), minLeafSize);
				std::vector<unsigned> branchWork(maxBranch - minBranch + 1);
				std::iota(std::begin(branchWork), std::end(branchWork), minBranch);

				for (size_t l = minLeafSize; l < maxLeafSize + 1; l++)
				{
					for (size_t b = minBranch; b < maxBranch + 1; b++)
					{
						std::string problem;
						unsigned int branchingFactor = b;
						unsigned int leafCount = l;
						std::cout << std::endl << std::endl << "-------------------------------------------------------------------" << std::endl;
						problem = "_b" + std::to_string(branchingFactor) + "_l" + std::to_string(leafCount);
						std::cout << "scenario " << name << " with branching factor of " << std::to_string(branchingFactor) << " and leafsize of " << leafCount << std::endl;
						std::cout << std::endl;

						//bvh of (seeded) random sphere
						//auto bvh = std::make_unique<Bvh>();

						std::chrono::high_resolution_clock::time_point timeLoop1 = std::chrono::high_resolution_clock::now();

						//bvh of loaded model:
						Bvh bvh = Bvh(*root, branchingFactor, leafCount, sortEachSplit);
						bvh.recursiveOctree(bucketCount);
						//bvh.recursiveOctree(2, leafCount);

						//collapses the next b child hierarchies to this node
						//bvh.collapseChilds(b - 1);
						//bvh.collapseChilds(1);
						//for other branching factors we need an other algorithm

						//construct compact tree representation:
						//4 versions i want to test: level, depth first, breadth first, and bvh version but for level


						//gather some bvh stats: node count, average branching factor, average leaf size, tree depth
						//This also duplicates the node system. the copy is used for the compact nodes
						bvh.bvhAnalysis(path, bvhAnalysis, saveBvhImage, name, problem);

						//determine type:

						std::chrono::high_resolution_clock::time_point timeLoop2 = std::chrono::high_resolution_clock::now();
						std::chrono::duration<double> time_span1 = std::chrono::duration_cast<std::chrono::duration<double>>(timeLoop2 - timeLoop1);
						std::cout << std::endl << "BVH building took " << time_span1.count() << " seconds." << std::endl;

						//only works with constant: (but i want an .txt as settings so i dont have to recompile...
						//const bool cond = compactNodeOrder == 0;
						//CompactNodeManager<std::conditional<cond, CompactNodeV0, CompactNodeV1>::type> manager(bvh);

						if (compactNodeOrder == 0 || compactNodeOrder == 1)
						{
							if (sortEachSplit)
							{
								CompactNodeManager<CompactNodeV3> manager(bvh, compactNodeOrder);
								//create camera and render image
								Camera c(path, name, problem, cameraPos, cameraTarget);
								c.renderImage(saveImage, saveDepthDetailedImage, manager, bvh, lights, renderType);
							}
							else
							{
								CompactNodeManager<CompactNodeV2> manager(bvh, compactNodeOrder);
								//create camera and render image
								Camera c(path, name, problem, cameraPos, cameraTarget);
								c.renderImage(saveImage, saveDepthDetailedImage, manager, bvh, lights, renderType);
							}
						}
						else
						{
							if (sortEachSplit)
							{
								std::cerr << "this nodeorder doesnt support sorting each split" << std::endl;
								throw(20);
							}
							CompactNodeManager<CompactNodeV0> manager(bvh, compactNodeOrder);
							//create camera and render image
							Camera c(path, name, problem, cameraPos, cameraTarget);
							c.renderImage(saveImage, saveDepthDetailedImage, manager, bvh, lights, renderType);
						}

						std::chrono::high_resolution_clock::time_point timeLoop3 = std::chrono::high_resolution_clock::now();
						std::chrono::duration<double> time_span2 = std::chrono::duration_cast<std::chrono::duration<double>>(timeLoop3 - timeLoop2);
						std::cout << "Rendering took " << time_span2.count() << " seconds." << std::endl;
					}
				}
			});

		if (scenarios.size() > 1)
		{
			std::cout.clear();
		}
		std::cout << std::endl;
		std::chrono::high_resolution_clock::time_point timeEnd = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> time_spanAll = std::chrono::duration_cast<std::chrono::duration<double>>(timeEnd - timeBegin);
		std::cout << "Everything took " << time_spanAll.count() << " seconds." << std::endl;
	}

	void readConfig()
	{
		//ugly but works 
		std::string line;
		std::ifstream myfile("config.txt");

		if (myfile.is_open())
		{
			while (std::getline(myfile, line))
			{

				if (!line.empty() && line[0] != '#')
				{
					auto res = line.find("minLeafSize", 0);
					auto res2 = res;
					if (res != std::string::npos)
					{
						minLeafSize = std::stoi(line.substr(line.find("=") + 1));
					}
					res = line.find("maxLeafSize", 0);
					if (res != std::string::npos)
					{
						maxLeafSize = std::stoi(line.substr(line.find("=") + 1));
					}
					res = line.find("minBranch", 0);
					if (res != std::string::npos)
					{
						minBranch = std::stoi(line.substr(line.find("=") + 1));
					}
					res = line.find("maxBranch", 0);
					if (res != std::string::npos)
					{
						maxBranch = std::stoi(line.substr(line.find("=") + 1));
					}

					res = line.find("renderType", 0);
					if (res != std::string::npos)
					{
						renderType = std::stoi(line.substr(line.find("=") + 1));
					}
					res = line.find("scenario", 0);
					if (res != std::string::npos)
					{
						std::string s = line.substr(line.find("=") + 1);
						std::string delimiter = ",";
						size_t pos = 0;
						std::string token;
						while ((pos = s.find(delimiter)) != std::string::npos) {
							token = s.substr(0, pos);
							scenarios.push_back(std::stoi(token));
							s.erase(0, pos + delimiter.length());
						}
						scenarios.push_back(std::stoi(s));
					}
					res = line.find("bucketCount", 0);
					if (res != std::string::npos)
					{
						bucketCount = std::stoi(line.substr(line.find("=") + 1));
					}
					res = line.find("compactNodeOrder", 0);
					if (res != std::string::npos)
					{
						compactNodeOrder = std::stoi(line.substr(line.find("=") + 1));
					}
					//booleans:
					res = line.find("saveImage", 0);
					if (res != std::string::npos)
					{
						res = line.find("true", 0);
						res2 = line.find("false", 0);
						if (res != std::string::npos)saveImage = true;
						else if (res2 != std::string::npos)saveImage = false;
						else
						{
							std::cerr << "saveImage value written wrong -> default = false" << std::endl;
						}
					}
					res = line.find("saveDepthDetailedImage ", 0);
					if (res != std::string::npos)
					{
						res = line.find("true", 0);
						res2 = line.find("false", 0);
						if (res != std::string::npos)saveDepthDetailedImage = true;
						else if (res2 != std::string::npos)saveDepthDetailedImage = false;
						else
						{
							std::cerr << "saveDepthDetailedImage value written wrong -> default = false" << std::endl;
						}
					}
					res = line.find("bvhAnalysis", 0);
					if (res != std::string::npos)
					{
						res = line.find("true", 0);
						res2 = line.find("false", 0);
						if (res != std::string::npos)bvhAnalysis = true;
						else if (res2 != std::string::npos)bvhAnalysis = false;
						else
						{
							std::cerr << "bvhAnalysis value written wrong -> default = false" << std::endl;
						}
					}
					res = line.find("saveBvhImage", 0);
					if (res != std::string::npos)
					{
						res = line.find("true", 0);
						res2 = line.find("false", 0);
						if (res != std::string::npos)saveBvhImage = true;
						else if (res2 != std::string::npos)saveBvhImage = false;
						else
						{
							std::cerr << "saveBvhImage value written wrong -> default = false" << std::endl;
						}
					}
					res = line.find("sortEachSplit", 0);
					if (res != std::string::npos)
					{
						res = line.find("true", 0);
						res2 = line.find("false", 0);
						if (res != std::string::npos)sortEachSplit = true;
						else if (res2 != std::string::npos)sortEachSplit = false;
						else
						{
							std::cerr << "sortEachSplit value written wrong -> default = false" << std::endl;
						}
					}
				}
			}
		}
	}
};

int main()
{
	//HelloTriangleApplication app;
	RayTracer rayTracer;
	try
	{
		rayTracer.readConfig();
	}
	catch (const std::exception & e)
	{
		std::cerr << "failed to read config?" << std::endl;
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	//need to check performance impact of this: (seems to be super low if any)
	try
	{
		rayTracer.run();
		std::cout << "end of program" << std::endl;
		system("pause");
	}
	catch (const std::exception & e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}