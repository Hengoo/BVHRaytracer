#include "rayTracer.h"

#include "timing.h"
#include "accelerationStructure/aabb.h"
#include "accelerationStructure/node.h"
#include "accelerationStructure/bvh.h"
#include "accelerationStructure/compactNode.h"
#include "accelerationStructure/fastNodeManager.h"
#include "primitives/triangle.h"
#include "cameraData.h"
#include "cameraFast.h"


#include "modelLoader.h"

#include "lights/light.h"
#include "lights/pointLight.h"
#include "lights/directionalLight.h"

#include "configUtil.h"

// Include the header file that the ispc compiler generates
#include "ISPC/ISPCBuild/rayTracer_ISPC.h"
using namespace::ispc;


//macros to start the template render stuff ... yeay (i think i still prefer it over rekursive template)
//what it does: convert the 3 numbers we read from a file to compile time values so we can use them in templates
//it also starts multiple renders in case we need node memory tests
#define startPerfRender(gangS, branchMem, workGroupS){					\
	if(gangS == 4 ) startPerfRender2(4, branchMem, workGroupS);			\
	if(gangS == 8 ) startPerfRender2(8, branchMem, workGroupS);			\
}

#define startPerfRender2(gangS, branchMem, workGroupS){					\
if(workGroupS == 8 ) startPerfRender3(gangS, branchMem, 8);				\
if(workGroupS == 16) startPerfRender3(gangS, branchMem, 16);			\
if(workGroupS == 32) startPerfRender3(gangS, branchMem, 32);			\
}

#define startPerfRender3(gangS, branchMem, workGroupS) {				\
	if (doNodeMemoryTest)												\
	{																	\
		startPerfRender4(gangS, branchMem, workGroupS);					\
		startPerfRender4(gangS, branchMem + gangS, workGroupS);			\
		startPerfRender4(gangS, branchMem + gangS * 2, workGroupS);		\
		startPerfRender4(gangS, branchMem + gangS * 3, workGroupS);		\
	}																	\
	else																\
	{																	\
		startPerfRender4(gangS, branchMem, workGroupS)					\
	}																	\
}

//short version
#define startPerfRender4(gangS, branchMem, workGroupS){					\
	if(branchMem == 8 ) startPerfRender5(gangS, 8, workGroupS);			\
	if(branchMem == 16 ) startPerfRender5(gangS, 16, workGroupS);		\
if constexpr (gangS == 4)												\
{																		\
	if(branchMem == 4 ) startPerfRender5(gangS, 4, workGroupS);			\
	if(branchMem == 12 ) startPerfRender5(gangS, 12, workGroupS);		\
}}

//long version
/*
#define startPerfRender4(gangS, branchMem, workGroupS){					\
	if(branchMem == 8 ) startPerfRender5(gangS, 8, workGroupS);			\
	if(branchMem == 16 ) startPerfRender5(gangS, 16, workGroupS);		\
	if(branchMem == 24 ) startPerfRender5(gangS, 24, workGroupS);		\
	if(branchMem == 32 ) startPerfRender5(gangS, 32, workGroupS);		\
	if(branchMem == 40 ) startPerfRender5(gangS, 40, workGroupS);		\
	if(branchMem == 48 ) startPerfRender5(gangS, 48, workGroupS);		\
	if(branchMem == 56 ) startPerfRender5(gangS, 56, workGroupS);		\
	if(branchMem == 64 ) startPerfRender5(gangS, 64, workGroupS);		\
if constexpr (gangS == 4)												\
{																		\
	if(branchMem == 4 ) startPerfRender5(gangS, 4, workGroupS);			\
	if(branchMem == 12 ) startPerfRender5(gangS, 12, workGroupS);		\
	if(branchMem == 20 ) startPerfRender5(gangS, 20, workGroupS);		\
	if(branchMem == 28 ) startPerfRender5(gangS, 28, workGroupS);		\
	if(branchMem == 36 ) startPerfRender5(gangS, 36, workGroupS);		\
	if(branchMem == 44 ) startPerfRender5(gangS, 44, workGroupS);		\
	if(branchMem == 52 ) startPerfRender5(gangS, 52, workGroupS);		\
	if(branchMem == 60 ) startPerfRender5(gangS, 60, workGroupS);		\
}}
*/

#define startPerfRender5(gangS, branchMem, workGroupS) 	{												\
	std::string problemPrefix = "_mb" + std::to_string(branchMem) + "_ml" + std::to_string(leafMemory);	\
	FastNodeManager<gangS, branchMem, workGroupS> manager(bvh, leafMemory, cache);								\
	CameraFast c(pathPerf, name, problem, problemPrefix, workGroupS, saveDistance, wideRender, cameraPositions, cameraTargets, xRes, yRes);			\
	c.renderImages(saveImage, manager, ambientSampleCount, ambientDistance, mute, wideAlternative, saveRayTimes, doCacheAnalysis);						\
}


void RayTracer::run()
{
	auto timeProgrammBegin = getTime();

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
	std::cout << "LeafSize from " << minLeafSize << " to " << maxLeafSize << " with a step size of " << leafStep << std::endl;
	std::cout << "Branching factor from " << minBranch << " to " << maxBranch << " with a step size of " << branchStep << std::endl;
	if (renderAnalysisImage)
	{
		std::cout << "render analysis image" << std::endl;
		if (saveImage) std::cout << "save image" << std::endl;
		if (saveDepthDetailedImage) std::cout << "save DepthDetailedImage" << std::endl;
	}

	if (bvhAnalysis)
	{
		std::cout << "do bvhAnalysis" << std::endl;
		if (saveBvhImage) std::cout << "save BvhImage" << std::endl;
	}
	if (doPerformanceTest)
	{
		std::cout << "do Performance tests" << std::endl;
		if (doNodeMemoryTest)std::cout << "do Node Memory tests" << std::endl;
		if (doLeafMemoryTest)std::cout << "do Leaf Memory tests" << std::endl;
	}
	std::cout << "render type: " << renderType << std::endl;
	for (auto& s : scenarios)
	{
		std::cout << "scenario: " << s << std::endl;
	}
	std::cout << "compact Node order: " << compactNodeOrder << std::endl;
	//std::cout << "Sah Node cost factor: " << nodeCostFactor << std::endl;
	//std::cout << "Sah Triangle cost factor: " << triangleCostFactor << std::endl;

	std::cout << "Workgroup size of " << workGroupSize << std::endl;

	unsigned gangSize = getGangSize();
	std::cout << "Gang size of " << gangSize << std::endl;
	std::cout << std::endl;

	if (wideRender)
	{
		std::cout << "Doing wide renderer" << std::endl;
	}
	else
	{
		std::cout << "Doing single ray traversal" << std::endl;
	}
	if (doWorkGroupAnalysis)
	{
		std::cout << "do Workgroup Analysis" << std::endl;
	}

	if (doCacheAnalysis)
	{
		std::cout << "Doing cache analysis with the follwoing cachesizes:" << std::endl;
		for (auto& c : cacheSizes)
		{
			std::cout << c << ", ";
		}
		std::cout << std::endl;
	}

	if (doPerformanceTest && !doCacheAnalysis)
	{
		std::cout << "press button to continue performance test (set high periority fist)" << std::endl;
		system("pause");
	}

	mute = scenarios.size() > 1 || maxBranch != minBranch || maxLeafSize != minLeafSize;
	if (mute)
	{
		//mute cout (so we dont have to place an if arround every cout)
		std::cout.setstate(std::ios_base::failbit);
	}

	//multiple scenarios in parallel are not really needed. it also takes quite some ram
	std::for_each(std::execution::seq, scenarios.begin(), scenarios.end(),
		[&](auto& scenario)
		{
			std::string name;
			//path where we save bvh results and intersections counts
			std::string path;
			//path where we save performance results (this will include avx or sse in the foldername)
			std::string pathPerf;
			std::vector<glm::vec3>  cameraPositions;
			std::vector<glm::vec3>  cameraTargets;

			std::vector<std::unique_ptr<Light>> lights;

			std::vector<std::shared_ptr<GameObject>> gameObjects;
			gameObjects.push_back(std::make_shared<GameObject>("root"));
			gameObjects[0]->hasParent = true;
			std::vector<std::shared_ptr<MeshBin>> meshBins;

			auto timeModelLoadBegin = getTime();

			//reminder : blender coordiantes of this (0, 360, 50) are -> glm::vec3(0, 50, -360);
			switch (scenario)
			{
			case 0:
				//https://sketchfab.com/3d-models/lizard-mage-817d52d9887948bfa0ca43aef6064eaa
				name = "lizard";
				loadGltfModel("models/Lizard/scene.gltf", gameObjects, meshBins);
				cameraPositions.push_back(glm::vec3(3.5f, 1.5f, 5.f));
				cameraTargets.push_back(glm::vec3(-1, -1, 1.1));

				cameraPositions.push_back(glm::vec3(3.5f, 1.5f, 5.f));
				cameraTargets.push_back(glm::vec3(-1, -1, 1.1));

				lights.push_back(std::make_unique<DirectionalLight>(glm::vec3(0, -1, 0), 10));
				break;
			case 1:
				//https://sketchfab.com/3d-models/shift-happens-canyon-diorama-ffd36dfbfda8432d97388988883f6295
				name = "shiftHappens";
				loadGltfModel("models/ShiftHappensTest.glb", gameObjects, meshBins);
				cameraPositions.push_back(glm::vec3(20, 10, -10));
				cameraTargets.push_back(glm::vec3(0, 5, 0));

				lights.push_back(std::make_unique<DirectionalLight>(glm::vec3(0, -1, 0), 10));
				break;
			case 2:
				//http://casual-effects.com/data/index.html
				//erato scene converted to glb with blender (replaced some #indoo with 0 so blender could load it?)
				name = "erato";
				loadGltfModel("models/erato/erato.glb", gameObjects, meshBins);
				cameraPositions.push_back(glm::vec3(10, 6, 9));
				cameraTargets.push_back(glm::vec3(-3, 1.5, 0));

				lights.push_back(std::make_unique<DirectionalLight>(glm::vec3(0, -1, 0), 10));
				break;
			case 3:
				//just some cubes for debuging single rays
				name = "cubes";
				loadGltfModel("models/4Cubes.glb", gameObjects, meshBins);
				cameraPositions.push_back(glm::vec3(0, 0, -3));
				cameraTargets.push_back(glm::vec3(0, 0, 0));

				lights.push_back(std::make_unique<DirectionalLight>(glm::vec3(0, -1, 0), 10));
				break;
			case 4:
				//http://casual-effects.com/data/index.html
				//the "Crytek Sponza"
				name = "sponza";
				loadGltfModel("models/sponzaColorful/sponzaColorful.glb", gameObjects, meshBins);
				cameraPositions.push_back(glm::vec3(-1100, 300, 0));
				cameraTargets.push_back(glm::vec3(-900, 290, 0));

				lights.push_back(std::make_unique<DirectionalLight>(glm::vec3(0, -1, 0), 10));
				break;

			case 5:
				//https://sketchfab.com/3d-models/davia-rocks-b8576a61715a4feabd9637215eeb2e05
				name = "daviaRock";
				loadGltfModel("models/davia_rocks/scene.gltf", gameObjects, meshBins);
				cameraPositions.push_back(glm::vec3(10, 2, 2));
				cameraTargets.push_back(glm::vec3(0, 0, 0));

				lights.push_back(std::make_unique<DirectionalLight>(glm::vec3(0, -1, 0), 10));
				break;
			case 6:
				//http://casual-effects.com/data/index.html
				//rungholt scene converted to glb with blender (took ages)
				name = "rungholt";
				loadGltfModel("models/rungholt/rungholt.glb", gameObjects, meshBins);

				cameraPositions.push_back(glm::vec3(-100, 41, -200));
				cameraTargets.push_back(glm::vec3(-140, 0, -70));

				lights.push_back(std::make_unique<DirectionalLight>(glm::vec3(-0.3, -1, -0.1), 10));
				break;
				//reminder : blender coordiantes of this (0, 360, 50) are -> glm::vec3(0, 50, -360);
			case 7:
				//http://casual-effects.com/data/index.html
				//Brackfast room scene converted to glb with blender
				name = "breakfast";
				loadGltfModel("models/breakfast_room/breakfast_room.glb", gameObjects, meshBins);

				cameraPositions.push_back(glm::vec3(4, 7.8, 9.5));
				cameraTargets.push_back(glm::vec3(2.4, 5.2, 5.1));

				lights.push_back(std::make_unique<DirectionalLight>(glm::vec3(-0.3, -1, -0.1), 10));
				break;
			case 8:
				//http://casual-effects.com/data/index.html
				//San miguel "low poly" scene converted to glb with this tool https://blackthread.io/gltf-converter/
				name = "sanMiguel";
				loadGltfModel("models/san-miguel-low-poly/san-miguel-low-poly.glb", gameObjects, meshBins);
				cameraPositions.push_back(glm::vec3(22, 2.2, 12.6));
				cameraTargets.push_back(glm::vec3(14, 2.0, 5.8));

				lights.push_back(std::make_unique<DirectionalLight>(glm::vec3(-0.3, -1, -0.1), 10));
				break;
			case 9:
				//http://casual-effects.com/data/index.html
				//amazon lumberyard inside scene converted to glb with this tool https://blackthread.io/gltf-converter/
				name = "amazonLumberyardInterior";
				loadGltfModel("models/AmazonLumberyard/interior.glb", gameObjects, meshBins);

				if (!doCacheAnalysis)
				{
					cameraPositions.push_back(glm::vec3(-109, 284, -147));
					cameraTargets.push_back(glm::vec3(20, 257, -151));
				}
				else
				{
					std::cout << "disabled camera 0 of amazon lumberyard since we do cache analysis" << std::endl;
				}

				cameraPositions.push_back(glm::vec3(765, 207, 550));
				cameraTargets.push_back(glm::vec3(786, 174, 123));

				lights.push_back(std::make_unique<DirectionalLight>(glm::vec3(-0.3, -1, -0.1), 10));
				break;
			case 10:
				//http://casual-effects.com/data/index.html
				//amazon lumberyard exterior scene converted to glb with this tool https://blackthread.io/gltf-converter/
				name = "amazonLumberyardExterior";
				loadGltfModel("models/AmazonLumberyard/exterior.glb", gameObjects, meshBins);
				cameraPositions.push_back(glm::vec3(-1059, 384, -200));
				cameraTargets.push_back(glm::vec3(300, 257, 330));

				lights.push_back(std::make_unique<DirectionalLight>(glm::vec3(-0.3, -1, -0.1), 10));
				break;
			case 11:
				//http://casual-effects.com/data/index.html
				//amazon lumberyard combined scene converted to glb with this tool https://blackthread.io/gltf-converter/
				name = "amazonLumberyardCombinedExterior";
				loadGltfModel("models/AmazonLumberyard/exterior.glb", gameObjects, meshBins);
				loadGltfModel("models/AmazonLumberyard/interior.glb", gameObjects, meshBins);
				cameraPositions.push_back(glm::vec3(-1059, 384, -200));
				cameraTargets.push_back(glm::vec3(300, 257, 330));

				lights.push_back(std::make_unique<DirectionalLight>(glm::vec3(-0.3, -1, -0.1), 10));
				break;
			case 12:
				//http://casual-effects.com/data/index.html
				//gallery converted to glb with this tool https://blackthread.io/gltf-converter/
				name = "gallery";
				loadGltfModel("models/gallery/gallery.glb", gameObjects, meshBins);
				cameraPositions.push_back(glm::vec3(-2.7, 2.88, 8));
				cameraTargets.push_back(glm::vec3(-1.2, 2.2, 1));

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

			path = "Analysis/Results";
			pathPerf = path;
			if (CreateDirectory(path.data(), NULL) ||
				ERROR_ALREADY_EXISTS == GetLastError())
			{
				if (doPerformanceTest)
				{
					std::string workType = "";
					if (gangSize == 4)
					{
						workType = "SsePerf";
					}
					else if (gangSize == 8)
					{
						workType = "AvxPerf";
					}
					else
					{
						std::cout << "unkown gang size of " << gangSize << ". Name needs to be added to raytracer.cpp so the results can be saved to a folder";
					}
					pathPerf += "/" + name + workType;
					if (CreateDirectory(pathPerf.data(), NULL) ||
						ERROR_ALREADY_EXISTS == GetLastError())
					{

						//create folders for the workGroup sizes we use:
						if (!renderAllOptions)
						{
							if (!wideRender)
							{
								std::string pathPerfWorkGroup0 = pathPerf + "/WorkGroupSize_" + std::to_string(workGroupSize) + "_Normal";
								if (!(CreateDirectory(pathPerfWorkGroup0.data(), NULL) ||
									ERROR_ALREADY_EXISTS == GetLastError()))
								{
									std::cerr << "failed to create performance workGroup scene directory" << std::endl;
								}
							}
							else if (wideAlternative == 0)
							{
								std::string pathPerfWorkGroup = pathPerf + "/WorkGroupSize_" + std::to_string(workGroupSize) + "_WideOld";
								if (!(CreateDirectory(pathPerfWorkGroup.data(), NULL) ||
									ERROR_ALREADY_EXISTS == GetLastError()))
								{
									std::cerr << "failed to create performance workGroup scene directory" << std::endl;
								}
							}
							else
							{
								std::string pathPerfWorkGroup = pathPerf + "/WorkGroupSize_" + std::to_string(workGroupSize) + "_Wide";
								if (!(CreateDirectory(pathPerfWorkGroup.data(), NULL) ||
									ERROR_ALREADY_EXISTS == GetLastError()))
								{
									std::cerr << "failed to create performance workGroup scene directory" << std::endl;
								}
							}

						}
						else
						{
							std::string pathPerfWorkGroup0 = pathPerf + "/WorkGroupSize_" + std::to_string(workGroupSize) + "_Normal";
							if (!(CreateDirectory(pathPerfWorkGroup0.data(), NULL) ||
								ERROR_ALREADY_EXISTS == GetLastError()))
							{
								std::cerr << "failed to create performance workGroup scene directory" << std::endl;
							}

							std::string pathPerfWorkGroup1 = pathPerf + "/WorkGroupSize_" + std::to_string(workGroupSize) + "_Wide";
							if (!(CreateDirectory(pathPerfWorkGroup1.data(), NULL) ||
								ERROR_ALREADY_EXISTS == GetLastError()))
							{
								std::cerr << "failed to create performance workGroup scene directory" << std::endl;
							}
						}


					}
					else
					{
						std::cerr << "failed to create performance scene directory" << std::endl;
						return;
					}
				}
				if (renderAnalysisImage || bvhAnalysis)
				{
					path += "/" + name;
					if (CreateDirectory(path.data(), NULL) ||
						ERROR_ALREADY_EXISTS == GetLastError())
					{
						if (doWorkGroupAnalysis)
						{
							//create folder for the work group sizes we use:
							std::string pathWorkGroup = path + "/WorkGroupSize_" + std::to_string(workGroupSize) + "_Version_" + std::to_string(wideAlternative);
							if (!(CreateDirectory(pathWorkGroup.data(), NULL) ||
								ERROR_ALREADY_EXISTS == GetLastError()))
							{
								std::cerr << "failed to create general workGroup scene directory" << std::endl;
							}
						}
					}
					else
					{
						std::cerr << "failed to create general scene directory" << std::endl;
						return;
					}
				}
			}
			else
			{
				std::cerr << "failed to create analysis directory" << std::endl;
				return;
			}

			//TODO: loading multiple models might have an error somewhere?

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
			//get primitive vector (this is what the bvh creator works on, so its copied for each image)

			auto pathOrig = path;
			auto pathPerfOrig = pathPerf;

			primPointVector primitives;
			preparePrimitives(primitives, *root, 0);
			size_t primVectorSize = primitives.size();
			for (int subDivCount = subdivisionStart; subDivCount < subdivisionEnd + 1; subDivCount += subdivisionStep)
			{
				if (subDivCount != 0)
				{
					primitives.clear();
					primitives.reserve(primVectorSize * (subDivCount + 1));
					preparePrimitives(primitives, *root, subDivCount);
				}
				//dont need those anymore (might need it for subdivision?
				//gameObjects.clear();
				//meshBins.clear();
				//gameObjects.shrink_to_fit();
				//meshBins.shrink_to_fit();

				std::cout << std::endl << "Model loading took " << getTimeFloat(timeModelLoadBegin) << " seconds." << std::endl;

				if (subdivisionEnd != 0)
				{
					if (renderAnalysisImage || bvhAnalysis)
					{
						path = pathOrig + "Sub" + ::std::to_string(subDivCount);
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
					if (doPerformanceTest)
					{
						pathPerf = pathPerfOrig + "Sub" + ::std::to_string(subDivCount);
						if (CreateDirectory(pathPerf.data(), NULL) ||
							ERROR_ALREADY_EXISTS == GetLastError())
						{

						}
						else
						{
							std::cerr << "failed to create directory" << std::endl;
							return;
						}
					}
				}

				//two versions:
				//non parallel when we already have multiple scenes

				if (maxBranch == minBranch && maxLeafSize == minLeafSize || doPerformanceTest)
				{
					//non parallel version:
					for (size_t l = minLeafSize; l < maxLeafSize + 1; l += leafStep)
					{
						for (size_t b = minBranch; b < maxBranch + 1; b += branchStep)
						{
							float sahFactor = loadSahFactor(l, b, gangSize);
							renderImage(b, l, gangSize, primitives, cameraPositions, cameraTargets,
								lights, name, path, pathPerf, sahFactor);
						}
					}
				}
				else
				{
					//parallel version

					//number of renders that should run in parallel:
					unsigned parallelCount = 4;

					if ((maxLeafSize - minLeafSize) > (maxBranch - minBranch))
					{
						//in case leafsize is larger than branching factor:
						for (size_t i = minLeafSize; i <= maxLeafSize; i = i + parallelCount)
						{

							std::vector<unsigned> leafWork(parallelCount);
							std::iota(std::begin(leafWork), std::end(leafWork), i);

							std::for_each(std::execution::par_unseq, leafWork.begin(), leafWork.end(),
								[&](auto& l)
								{
									if ((l - minLeafSize) * leafStep + minLeafSize <= maxLeafSize)
									{
										for (size_t b = minBranch; b < maxBranch + 1; b += branchStep)
										{
											float sahFactor = loadSahFactor(l, b, gangSize);
											renderImage(b, (l - minLeafSize) * leafStep + minLeafSize, gangSize, primitives,
												cameraPositions, cameraTargets, lights, name, path, pathPerf, sahFactor);
										}
									}
								});
						}
					}
					else
					{
						//in case branching factor is larger than leafsize:
						for (size_t i = minBranch; i <= maxBranch; i = i + parallelCount)
						{
							std::vector<unsigned> branchWork(parallelCount);
							std::iota(std::begin(branchWork), std::end(branchWork), i);

							std::for_each(std::execution::par_unseq, branchWork.begin(), branchWork.end(),
								[&](auto& b)
								{
									if ((b - minBranch) * branchStep + minBranch <= maxBranch)
									{
										for (size_t l = minLeafSize; l < maxLeafSize + 1; l += leafStep)
										{
											float sahFactor = loadSahFactor(l, b, gangSize);
											renderImage((b - minBranch) * branchStep + minBranch, l, gangSize, primitives,
												cameraPositions, cameraTargets, lights, name, path, pathPerf, sahFactor);
										}
									}
								});
						}
					}
				}
			}
		});

	if (mute)
	{
		std::cout.clear();
	}
	std::cout << std::endl;
	std::cout << "Everything took " << getTimeFloat(timeProgrammBegin) << " seconds." << std::endl;
}

void RayTracer::renderImage(unsigned branchingFactor, unsigned leafSize, unsigned gangSize, primPointVector& primitives,
	std::vector<glm::vec3>& cameraPositions, std::vector<glm::vec3>& cameraTargets, std::vector<std::unique_ptr<Light>>& lights,
	std::string& name, std::string& path, std::string& pathPerf, float sahFactor)
{
	std::string problem;
	std::cout << std::endl << std::endl << "-------------------------------------------------------------------" << std::endl;
	problem = "_b" + std::to_string(branchingFactor) + "_l" + std::to_string(leafSize);
	std::cout << "scenario " << name << " with branching factor of " << std::to_string(branchingFactor) << " and leafsize of " << leafSize << std::endl;
	std::cout << std::endl;

	//bvh of (seeded) random sphere
	//auto bvh = std::make_unique<Bvh>();

	auto timeBeginBvhBuild = getTime();

	//bvh of loaded model:
	Bvh bvh = Bvh(primitives, branchingFactor, leafSize, sortEachSplit, leafSplitOption);
	bvh.recursiveBvh(sahFactor);

	float ambientDistance = cbrt(bvh.getRoot()->getVolume()) / 10.f;
	//std::cout << ambientDistance << std::endl;

	//bvh.recursiveOctree(2, leafCount);

	//collapses the next b child hierarchies to this node
	//bvh.collapseChilds(b - 1);
	//bvh.collapseChilds(1);
	//for other branching factors we need an other algorithm

	//construct compact tree representation:
	//4 versions i want to test: level, depth first, breadth first, and bvh version but for level


	//gather some bvh stats: node count, average branching factor, average leaf size, tree depth
	//This also duplicates the node system. the copy is used for the compact nodes
	bvh.bvhAnalysis(path, bvhAnalysis, doPerformanceTest, saveBvhImage, name, problem, triangleCostFactor, nodeCostFactor, mute);
	std::cout << std::endl << "BVH building and bvh Analysis took " << getTimeFloat(timeBeginBvhBuild) << " seconds." << std::endl;
	auto timeBeginRendering = getTime();

	if (doPerformanceTest)
	{
		//calculte the leaf and node memory (multiples of gangsize (4 or 8)
		int minLeafMemory = ceil(leafSize / (float)gangSize) * gangSize;
		int minBranchMemory = ceil(branchingFactor / (float)gangSize) * gangSize;
		int maxLeafMemory = minLeafMemory + gangSize * 3;

		if (!doLeafMemoryTest)
		{
			maxLeafMemory = minLeafMemory;
		}

		for (int leafMemory = minLeafMemory; leafMemory <= maxLeafMemory; leafMemory += gangSize)
		{
			if (doCacheAnalysis)
			{
				//write how large the nodes, ez are?				

				for (auto& cacheSize : cacheSizes)
				{
					std::cerr << "Testing cacheSize " << cacheSize << std::endl;
					CacheSimulator cache(cacheSize);
					//macro that manages the calling of the right renderer (thx templates...)
					if (!renderAllOptions)
					{
						startPerfRender(gangSize, minBranchMemory, workGroupSize);
					}
					else
					{
						//renders all versions -> can resue bvh (~3.5 seconds for lumberyard)
						wideRender = true;
						wideAlternative = true;
						std::cout << std::endl << "Rendering Wide V1" << std::endl;
						startPerfRender(gangSize, minBranchMemory, workGroupSize);
						//wideAlternative = false;
						//std::cout << std::endl << "Rendering Wide V0" << std::endl;
						//startPerfRender(gangSize, minBranchMemory, workGroupSize);
						wideRender = false;
						std::cout << std::endl << "Rendering normal" << std::endl;
						startPerfRender(gangSize, minBranchMemory, workGroupSize);
					}
				}
			}
			else
			{
				CacheSimulator cache(0);
				//macro that manages the calling of the right renderer (thx templates...)
				if (!renderAllOptions)
				{
					startPerfRender(gangSize, minBranchMemory, workGroupSize);
				}
				else
				{
					//renders all versions -> can resue bvh (~3.5 seconds for lumberyard)
					wideRender = true;
					wideAlternative = true;
					std::cout << std::endl << "Rendering Wide V1" << std::endl;
					startPerfRender(gangSize, minBranchMemory, workGroupSize);
					//wideAlternative = false;
					//std::cout << std::endl << "Rendering Wide V0" << std::endl;
					//startPerfRender(gangSize, minBranchMemory, workGroupSize);
					wideRender = false;
					std::cout << std::endl << "Rendering normal" << std::endl;
					startPerfRender(gangSize, minBranchMemory, workGroupSize);
				}
			}
		}
	}

	if (renderAnalysisImage)
	{
		if (renderType == 2 && saveDistance)
		{
			renderType == 3;
		}
		if (wideRender)
		{
			if (renderType != 2 || saveDistance)
			{
				std::cerr << "wide renderer only supports fake simd intersect (renderType 2) without distance saving" << std::endl;
				std::cerr << "the settings where adjusted for this render." << std::endl;
			}
			renderType == 2;
		}
		if (compactNodeOrder == 0 || compactNodeOrder == 1)
		{
			if (sortEachSplit)
			{
				CompactNodeManager<CompactNodeV3> manager(bvh, compactNodeOrder);
				//create camera and render image
				CameraData c(path, name, problem, workGroupSize, wideRender, cameraPositions, cameraTargets, xRes, yRes);
				c.renderImages(saveImage, saveDepthDetailedImage, manager, bvh, lights, ambientSampleCount,
					ambientDistance, castShadows, renderType, mute, doWorkGroupAnalysis, wideAlternative);
			}
			else
			{
				CompactNodeManager<CompactNodeV2> manager(bvh, compactNodeOrder);
				//create camera and render image
				CameraData c(path, name, problem, workGroupSize, wideRender, cameraPositions, cameraTargets, xRes, yRes);
				c.renderImages(saveImage, saveDepthDetailedImage, manager, bvh, lights, ambientSampleCount,
					ambientDistance, castShadows, renderType, mute, doWorkGroupAnalysis, wideAlternative);
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
			CameraData c(path, name, problem, workGroupSize, wideRender, cameraPositions, cameraTargets, xRes, yRes);
			c.renderImages(saveImage, saveDepthDetailedImage, manager, bvh, lights, ambientSampleCount,
				ambientDistance, castShadows, renderType, mute, doWorkGroupAnalysis, wideAlternative);
		}
	}
	std::cout << "All to do with rendering took " << getTimeFloat(timeBeginRendering) << " seconds." << std::endl;
}

void RayTracer::preparePrimitives(primPointVector& primitives, GameObject& root, int subdivision)
{
	root.iterateGo(primitives, subdivision);

	//sort primitive vector along the largest axis (rungholt needs ~1.5 seconds) (otherwise this operation is done 15*16 times)
	chooseAxisAndSort(primitives.begin(), primitives.end());
	std::cout << "creating bvh of " << primitives.size() << " Triangles" << std::endl;
}

void RayTracer::readConfig()
{
	std::string line;
	std::ifstream myfile("config.txt");

	if (myfile.is_open())
	{
		while (std::getline(myfile, line))
		{

			if (!line.empty() && line[0] != '#')
			{
				//integers:
				std::optional<int> result;
				readInt(line, "minLeafSize", minLeafSize);

				readInt(line, "maxLeafSize", maxLeafSize);
				readInt(line, "leafStep", leafStep);

				readInt(line, "minBranch", minBranch);
				readInt(line, "maxBranch", maxBranch);
				readInt(line, "branchStep", branchStep);

				readInt(line, "subdivisionStart", subdivisionStart);
				readInt(line, "subdivisionEnd", subdivisionEnd);
				readInt(line, "subdivisionStep", subdivisionStep);

				readInt(line, "renderType", renderType);
				readInt(line, "ambientSampleCount", ambientSampleCount);

				readInt(line, "workGroupSize", workGroupSize);
				readInt(line, "leafSplitOption", leafSplitOption);

				readInt(line, "xRes", xRes);
				readInt(line, "yRes", yRes);

				//booleans:
				readBool(line, "saveImage", saveImage);
				readBool(line, "saveDepthDetailedImage", saveDepthDetailedImage);
				readBool(line, "bvhAnalysis", bvhAnalysis);
				readBool(line, "saveBvhImage", saveBvhImage);
				readBool(line, "sortEachSplit", sortEachSplit);
				readBool(line, "renderAnalysisImage", renderAnalysisImage);
				readBool(line, "castShadows", castShadows);
				readBool(line, "doPerformanceTest", doPerformanceTest);
				readBool(line, "doNodeMemoryTest", doNodeMemoryTest);
				readBool(line, "doLeafMemoryTest", doLeafMemoryTest);
				readBool(line, "saveRayTimes", saveRayTimes);
				readBool(line, "doWorkGroupAnalysis", doWorkGroupAnalysis);
				readBool(line, "saveDistance", saveDistance);
				readBool(line, "wideRender", wideRender);
				readBool(line, "wideAlternative", wideAlternative);
				readBool(line, "renderAllOptions", renderAllOptions);
				readBool(line, "doCacheAnalysis", doCacheAnalysis);

				//scenario has an int for each scenario.
				auto res = line.find("scenario", 0);
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

					if (scenarios.empty())
					{
						std::cerr << "no Scenario found. Using default scenario of 9" << std::endl;
						scenarios.push_back(9);
					}
				}


				//read in cacheline sizes to test:
				if (doCacheAnalysis)
				{
					res = line.find("cacheSize", 0);
					if (res != std::string::npos)
					{
						std::string s = line.substr(line.find("=") + 1);
						std::string delimiter = ",";
						size_t pos = 0;
						std::string token;
						while ((pos = s.find(delimiter)) != std::string::npos) {
							token = s.substr(0, pos);
							int size = std::stoi(token);
							if (size % 8 != 0)
							{
								std::cerr << "cachesize must be divideable by 8 to fit in 8 way set. The cachesize" << size << "is ignored." << std::endl;
								continue;
							}
							cacheSizes.push_back(size);
							s.erase(0, pos + delimiter.length());
						}
						int size = std::stoi(s);
						if (size % 8 != 0)
						{
							std::cerr << "cachesize must be divideable by 8 to fit in 8 way set. The cachesize" << size << "is ignored." << std::endl;
							continue;
						}
						cacheSizes.push_back(size);
						if (cacheSizes.empty())
						{
							std::cerr << "no cachesize found. Using default cachesize of 256" << std::endl;
							cacheSizes.push_back(256);
						}
					}
				}

				//floats:
				//currently nothing method is readFloat


			}
		}
	}
}


float RayTracer::loadSahFactor(int leafSize, int nodeSize, int gangSize)
{
	//i think i will shrink the table from summary to just have leafsize, nodesize, and sahfactor and read from that
	//read sahNodeFactorConfig
	float sahFactor = NAN;

	std::string line;
	std::string gangSizeName = "Sse";
	if (gangSize == 8)
	{
		gangSizeName = "Avx";
		std::cerr << "Avx sah factor config not set up / tested." << std::endl;
		return 1;
	}
	std::string versionName = "";
	if (wideRender)
	{
		versionName = "V" + std::to_string(wideAlternative);
	}
	std::ifstream myfile("sahFactorConfig" + gangSizeName + versionName + ".txt");

	std::vector<float> lineContent;
	if (myfile.is_open())
	{
		std::getline(myfile, line);
		while (std::getline(myfile, line))
		{
			if (!line.empty())
			{
				int configNodeSize = 0;
				int configLeafSize = 0;
				lineContent = readConfigLine(line, configNodeSize, configLeafSize);
				if ((configNodeSize <= nodeSize && configNodeSize > nodeSize - gangSize)
					&& (configLeafSize <= leafSize && configLeafSize > leafSize - leafSize))
				{
					sahFactor = lineContent[0];
				}
			}
		}
	}
	else
		if (isnan(sahFactor))
		{
			std::cerr << "not able to find fitting sahFactor in config, instead took 1" << std::endl;
			sahFactor = 1;
		}
	std::cout << "using sah factor of " << sahFactor << std::endl;
	return sahFactor;
}