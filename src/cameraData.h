#pragma once

#include "lights/light.h"
#include "glmInclude.h"
#include "util.h"
#include "camera.h"

#include <iostream>
// writing on a text file
#include <fstream>

#include <algorithm>
#include <vector>

//for the parallel for
#include <execution>

//spawns all the rays in the scene
class CameraData : public Camera
{
public:

	//per pixel per depth counter:
	std::vector<std::vector<uint16_t>> nodeIntersectionPerPixelCount;
	std::vector<std::vector<uint16_t>> leafIntersectionPerPixelCount;
	std::vector<std::vector<uint16_t>> shadowNodeIntersectionPerPixelCount;
	std::vector<std::vector<uint16_t>> shadowLeafIntersectionPerPixelCount;
	std::vector<std::vector<uint16_t>> childFullnessPerPixelCount;
	std::vector<std::vector<uint16_t>> primitiveFullnessPerPixelCount;

	//per pixel counter:
	std::vector<uint16_t> primitiveIntersectionsPerPixel;
	std::vector<uint16_t> shadowPrimitiveIntersectionsPerPixel;
	std::vector<uint16_t> successfulPrimitiveIntersectionsPerPixel;
	std::vector<uint16_t> successfulAabbIntersectionsPerPixel;
	std::vector<uint16_t> aabbIntersectionsPerPixel;
	std::vector<uint16_t> shadowRayCounter;
	std::vector<uint16_t> shadowSuccessfulPrimitiveIntersectionsPerPixel;
	std::vector<uint16_t> shadowSuccessfulAabbIntersectionsPerPixel;
	std::vector<uint16_t> shadowAabbIntersectionsPerPixel;

	//per depth counter:
	std::vector<uint64_t> primitiveFullness;
	std::vector<uint64_t> childFullness;
	std::vector<uint64_t> nodeIntersectionPerDepthCount;
	std::vector<uint64_t> leafIntersectionPerDepthCount;
	std::vector<uint64_t> shadowNodeIntersectionPerDepthCount;
	std::vector<uint64_t> shadowLeafIntersectionPerDepthCount;

	//per workGroup per step:
	std::vector<std::vector<uint32_t>> nodeWorkPerStep;
	std::vector<std::vector<uint32_t>> leafWorkPerStep;
	std::vector<std::vector<uint32_t>> uniqueNodesPerStep;
	std::vector<std::vector<uint32_t>> uniqueLeafsPerStep;

	std::vector<std::vector<uint32_t>> secondaryNodeWorkPerStep;
	std::vector<std::vector<uint32_t>> secondaryLeafWorkPerStep;
	std::vector<std::vector<uint32_t>> secondaryUniqueNodesPerStep;
	std::vector<std::vector<uint32_t>> secondaryUniqueLeafsPerStep;

	//per workGroup
	//step count can be infered from  size of per Work Group per steps size. (and it should be the same as sum of node / leaf)

	//normal counter
	uint64_t nodeIntersectionCount;
	uint64_t leafIntersectionCount;
	uint64_t shadowNodeIntersectionCount;
	uint64_t shadowLeafIntersectionCount;
	uint64_t primitiveIntersections;
	uint64_t shadowPrimitiveIntersections;
	uint64_t successfulPrimitiveIntersections;
	uint64_t successfulAabbIntersections;
	uint64_t aabbIntersections;
	uint64_t shadowRayCount;
	uint64_t shadowSuccessfulPrimitiveIntersections;
	uint64_t shadowSuccessfulAabbIntersections;
	uint64_t shadowAabbIntersections;

	bool wideRender;

	CameraData(std::string path, std::string name, std::string problem, int workGroupSize, bool wideRender, std::vector<glm::vec3>& positions,
		std::vector<glm::vec3>& lookCenters, glm::vec3 upward = glm::vec3(0, 1, 0), float focalLength = 0.866f, size_t height = 1088, size_t width = 1920)
		:Camera(path, name, problem, workGroupSize, positions, lookCenters, upward, focalLength, height, width), wideRender(wideRender)
	{
		image.resize(height * width * 4);
		initializeVariables();
	}

	CameraData(std::string path, std::string name, std::string problem, int workGroupSize, bool wideRender, std::vector<glm::mat4>& transforms,
		float focalLength = 0.866f, size_t height = 1088, size_t width = 1920)
		:Camera(path, name, problem, workGroupSize, transforms, focalLength, height, width), wideRender(wideRender)
	{
		image.resize(height * width * 4);
		initializeVariables();
	}

	template<typename T>
	void renderImages(bool saveImage, bool saveDepthDebugImage, CompactNodeManager<T>& nodeManager
		, Bvh& bvh, std::vector<std::unique_ptr<Light>>& lights, unsigned ambientSampleCount,
		float ambientDistance, bool castShadows, int renderType, bool mute, bool doWorkGroupAnalysis)
	{
		renderImage(saveImage, saveDepthDebugImage, nodeManager, bvh, lights, ambientSampleCount,
			ambientDistance, castShadows, renderType, mute, doWorkGroupAnalysis, 0);
	}

	//spawns rays and collects results into image. Image is written on disk
	template<typename T>
	void renderImage(bool saveImage, bool saveDepthDebugImage, CompactNodeManager<T>& nodeManager
		, Bvh& bvh, std::vector<std::unique_ptr<Light>>& lights, unsigned ambientSampleCount,
		float ambientDistance, bool castShadows, int renderType, bool mute, bool doWorkGroupAnalysis, int cameraId)
	{
		int workGroupSize = nonTemplateWorkGroupSize;
		/*
		glm::vec3 decScale;
		glm::quat decOrientation;
		glm::vec3 decTranslation;
		glm::vec3 decSkew;
		glm::vec4 decPerspective;
		glm::decompose(transform, decScale, decOrientation, decTranslation, decSkew, decPerspective);
		*/
		//simplified ortho version for now:
		if (!wideRender)
		{
			//normal render:
#pragma omp parallel for schedule(dynamic, 4)
			for (int i = 0; i < (width / workGroupSize) * (height / workGroupSize); i++)
			{
				for (int j = 0; j < workGroupSquare; j++)
				{
					RenderInfo info(
						((i * workGroupSize) % width - width / 2.f) + (j % workGroupSize),
						-(((i * workGroupSize) / width) * workGroupSize - height / 2.f) - (j / workGroupSize),
						(i * workGroupSize) % width + (j % workGroupSize) + (((i * workGroupSize) / width) * workGroupSize + (j / workGroupSize)) * width);
					int realIndex = i * workGroupSquare + j;
					glm::vec3 targetPos = getRayTargetPosition(info, cameraId);
					auto ray = Ray(positions[cameraId], targetPos - positions[cameraId], bvh);

					bool result;
					switch (renderType)
					{
					case 0:
						result = bvh.intersect(ray);
						break;
					case 1:
						result = nodeManager.intersect(ray);
						break;
					case 2:
						result = nodeManager.intersectImmediately(ray, false);
						break;
					case 3:
						result = nodeManager.intersectImmediately(ray, true);
						break;
					default:
						result = nodeManager.intersectImmediately(ray, true);
						break;
					}

					//check shadows and ambient occlusion if ray hit something
					if (result)
					{
						//ambient occlusion:
						unsigned ambientResult = 0;

						for (size_t i = 0; i < ambientSampleCount; i++)
						{
							//deterministic random direction
							auto direction = getAmbientDirection(realIndex, ray.surfaceNormal, i);
							Ray ambientRay(ray.surfacePosition + ray.surfaceNormal * 0.001f, direction, bvh, true);
							ambientRay.tMax = ambientDistance;

							if (shootShadowRay(ambientRay, ray, info, bvh, nodeManager, renderType))
							{
								ambientResult++;
							}
						}
						if (ambientSampleCount != 0)
						{
							float factor = 1 - ambientResult / (float)ambientSampleCount;
							factor = (factor + 1) / 2.f;
							ray.surfaceColor.scale(factor);
							//ray.surfaceColor = Color(factor);
						}

						float factor = 1;

						//resolve shadows

						for (auto& l : lights)
						{
							float lightDistance;
							glm::vec3 lightVector;

							//todo use light color
							auto lightColor = l->getLightDirection(ray.surfacePosition, lightVector, lightDistance);

							float f = glm::dot(ray.surfaceNormal, lightVector);
							//add bias to vector to prevent shadow rays hitting the surface they where created for
							Ray shadowRay(ray.surfacePosition + ray.surfaceNormal * 0.001f, lightVector, bvh, true);
							shadowRay.tMax = lightDistance;


							//only shoot ray when surface points in light direction
							if (castShadows && factor > 0)
							{
								if (shootShadowRay(shadowRay, ray, info, bvh, nodeManager, renderType))
								{
									factor = 0;
								}
							}

							factor = std::max(0.3f, factor);
							ray.surfaceColor.scale(factor);
						}

					}

					image[info.index * 4 + 0] = (uint8_t)(ray.surfaceColor.r * 255);
					image[info.index * 4 + 1] = (uint8_t)(ray.surfaceColor.g * 255);
					image[info.index * 4 + 2] = (uint8_t)(ray.surfaceColor.b * 255);
					image[info.index * 4 + 3] = (uint8_t)(ray.surfaceColor.a * 255);

					//renders normal
					//image[info.index * 4 + 0] = (uint8_t)(ray.surfaceNormal.x * 127 + 127);
					//image[info.index * 4 + 1] = (uint8_t)(ray.surfaceNormal.y * 127 + 127);
					//image[info.index * 4 + 2] = (uint8_t)(ray.surfaceNormal.z * 127 + 127);

					collectPrimaryRayData(ray, info.index);
				}
			}
		}
		else
		{
			//wide render
#pragma omp parallel for schedule(dynamic, 4)
			for (int i = 0; i < (width / workGroupSize) * (height / workGroupSize); i++)
			{
				//prepare rays for render chunk
				std::vector<Ray> rays(workGroupSquare);

				int tmp0 = ((i * workGroupSize) / width) * workGroupSize - height / 2;
				int tmp1 = ((i * workGroupSize) % width - width / 2);
				for (int j = 0; j < workGroupSquare; j++)
				{
					glm::vec3 targetPos = getRayTargetPosition(
						(-tmp0 - (j / workGroupSize)),
						(tmp1 + (j % workGroupSize)), cameraId);
					rays[j] = Ray(positions[cameraId], targetPos - positions[cameraId], bvh);
				}

				//shoot primary rays
				nodeManager.intersectWide(rays, nodeWorkPerStep[i], leafWorkPerStep[i], uniqueNodesPerStep[i], uniqueLeafsPerStep[i]);


				//shoot secondary rays
				std::vector<Ray> secondaryRays(workGroupSquare);
				std::vector<uint8_t> ambientResult(workGroupSquare);
				for (int a = 0; a < ambientSampleCount; a++)
				{
					for (int j = 0; j < workGroupSquare; j++)
					{
						auto direction = getAmbientDirection(i * workGroupSquare + j, rays[j].surfaceNormal, a);
						secondaryRays[j] = Ray(rays[j].surfacePosition + rays[j].surfaceNormal * 0.001f, direction, bvh, true);
						secondaryRays[j].tMax = ambientDistance;
					}
					nodeManager.intersectWide(secondaryRays, secondaryNodeWorkPerStep[i], secondaryLeafWorkPerStep[i],
						secondaryUniqueNodesPerStep[i], secondaryUniqueLeafsPerStep[i]);

					for (int j = 0; j < workGroupSquare; j++)
					{
						if (isnan(secondaryRays[j].tMax))
						{
							ambientResult[j] ++;
						}

						int id = (i * workGroupSize) % width + (j % workGroupSize) + (((i * workGroupSize) / width) * workGroupSize + (j / workGroupSize)) * width;
						collectSecondaryRayData(secondaryRays[j], id);
					}
				}

				//write image
				for (int j = 0; j < workGroupSquare; j++)
				{
					int id = (i * workGroupSize) % width + (j % workGroupSize) + (((i * workGroupSize) / width) * workGroupSize + (j / workGroupSize)) * width;

					float factor = 1 - (ambientResult[j] / (float)ambientSampleCount);
					//factor = rays[j].tMax / 100.0f;
					uint8_t imageResult = (uint8_t)(factor * 255);
					image[id * 4 + 0] = imageResult;
					image[id * 4 + 1] = imageResult;
					image[id * 4 + 2] = imageResult;
					image[id * 4 + 3] = 255;

					collectPrimaryRayData(rays[j], id);
				}
			}
		}

		//gather data:

		leafIntersectionPerDepthCount.resize(bvh.bvhDepth);
		for (auto& perPixel : leafIntersectionPerPixelCount)
		{
			for (size_t i = 0; i < perPixel.size(); i++)
			{
				leafIntersectionPerDepthCount[i] += perPixel[i];
			}
		}
		nodeIntersectionPerDepthCount.resize(bvh.bvhDepth);
		for (auto& perPixel : nodeIntersectionPerPixelCount)
		{
			for (size_t i = 0; i < perPixel.size(); i++)
			{
				nodeIntersectionPerDepthCount[i] += perPixel[i];
			}
		}
		shadowLeafIntersectionPerDepthCount.resize(bvh.bvhDepth);
		for (auto& perPixel : shadowLeafIntersectionPerPixelCount)
		{
			for (size_t i = 0; i < perPixel.size(); i++)
			{
				shadowLeafIntersectionPerDepthCount[i] += perPixel[i];
			}
		}
		shadowNodeIntersectionPerDepthCount.resize(bvh.bvhDepth);
		for (auto& perPixel : shadowNodeIntersectionPerPixelCount)
		{
			for (size_t i = 0; i < perPixel.size(); i++)
			{
				shadowNodeIntersectionPerDepthCount[i] += perPixel[i];
			}
		}
		childFullness.resize(bvh.branchingFactor + 1);
		for (auto& perPixel : childFullnessPerPixelCount)
		{
			for (size_t i = 0; i < perPixel.size(); i++)
			{
				childFullness[i] += perPixel[i];
			}
		}
		primitiveFullness.resize(bvh.leafSize + 1);
		for (auto& perPixel : primitiveFullnessPerPixelCount)
		{
			for (size_t i = 0; i < perPixel.size(); i++)
			{
				primitiveFullness[i] += perPixel[i];
			}
		}

		leafIntersectionCount = std::accumulate(leafIntersectionPerDepthCount.begin(), leafIntersectionPerDepthCount.end(), 0);
		nodeIntersectionCount = std::accumulate(nodeIntersectionPerDepthCount.begin(), nodeIntersectionPerDepthCount.end(), 0);

		shadowLeafIntersectionCount = std::accumulate(shadowLeafIntersectionPerDepthCount.begin(), shadowLeafIntersectionPerDepthCount.end(), 0);
		shadowNodeIntersectionCount = std::accumulate(shadowNodeIntersectionPerDepthCount.begin(), shadowNodeIntersectionPerDepthCount.end(), 0);

		primitiveIntersections = std::accumulate(primitiveIntersectionsPerPixel.begin(), primitiveIntersectionsPerPixel.end(), 0);
		successfulPrimitiveIntersections = std::accumulate(successfulPrimitiveIntersectionsPerPixel.begin(), successfulPrimitiveIntersectionsPerPixel.end(), 0);
		successfulAabbIntersections = std::accumulate(successfulAabbIntersectionsPerPixel.begin(), successfulAabbIntersectionsPerPixel.end(), 0);
		aabbIntersections = std::accumulate(aabbIntersectionsPerPixel.begin(), aabbIntersectionsPerPixel.end(), 0);
		shadowRayCount = std::accumulate(shadowRayCounter.begin(), shadowRayCounter.end(), 0);
		shadowSuccessfulPrimitiveIntersections = std::accumulate(shadowSuccessfulPrimitiveIntersectionsPerPixel.begin(), shadowSuccessfulPrimitiveIntersectionsPerPixel.end(), 0);
		shadowSuccessfulAabbIntersections = std::accumulate(shadowSuccessfulAabbIntersectionsPerPixel.begin(), shadowSuccessfulAabbIntersectionsPerPixel.end(), 0);
		shadowAabbIntersections = std::accumulate(shadowAabbIntersectionsPerPixel.begin(), shadowAabbIntersectionsPerPixel.end(), 0);
		shadowPrimitiveIntersections = std::accumulate(shadowPrimitiveIntersectionsPerPixel.begin(), shadowPrimitiveIntersectionsPerPixel.end(), 0);

		//for wide renderer
		if (wideRender)
		{
			std::cerr << "test" << std::endl;
			uint64_t nodeCountTest = 0;
			uint64_t leafCountTest = 0;
			//verified that that the basic intersection numbers are right

			for (int i = 0; i < (width / workGroupSize) * (height / workGroupSize); i++)
			{
				nodeCountTest = std::accumulate(nodeWorkPerStep[i].begin(), nodeWorkPerStep[i].end(), nodeCountTest);
				leafCountTest = std::accumulate(leafWorkPerStep[i].begin(), leafWorkPerStep[i].end(), leafCountTest);
			}
			if (nodeCountTest != nodeIntersectionCount)
			{
				std::cerr << "nodeCount off " << nodeCountTest << " target: " << nodeIntersectionCount << std::endl;
			}
			if (leafCountTest != leafIntersectionCount)
			{
				std::cerr << "leafCount off " << leafCountTest << " target: " << leafIntersectionCount << std::endl;
			}
		}

		//normalize by ray
		float factor = 1 / (float)(width * height);
		float shadowFactor = 1 / (float)shadowRayCount;
		float bothFactor = 1 / (float)(width * height + shadowRayCount);

		if (!mute)
		{
			std::cout << "intersections counts are normalized per Ray" << std::endl << std::endl;
			std::cout << "primary intersections node: " << nodeIntersectionCount * factor << std::endl;
			std::cout << "primary aabb intersections: " << aabbIntersections * factor << std::endl;
			std::cout << "primary aabb success ration: " << successfulAabbIntersections / (float)aabbIntersections << std::endl;
			//wastefactor = "verschwendungsgrad".
			// basically number of nodes visited / number of aabb tested
			//the minus width*height is basically -1 for each ray -> needed to get right values
			float wasteFactor = (leafIntersectionCount + nodeIntersectionCount - width * height) / (float)(nodeIntersectionCount * bvh.branchingFactor);
			std::cout << "primary waste factor: " << 1 - wasteFactor << std::endl;
			std::cout << "primary intersections leaf: " << leafIntersectionCount * factor << std::endl;
			std::cout << "primary primitive intersections: " << primitiveIntersections * factor << std::endl;
			std::cout << "primary primitive success ratio: " << successfulPrimitiveIntersections / (float)primitiveIntersections << std::endl;
			std::cout << std::endl;
			std::cout << "secondary intersections node: " << shadowNodeIntersectionCount * shadowFactor << std::endl;
			std::cout << "secondary aabb intersections: " << shadowAabbIntersections * shadowFactor << std::endl;
			std::cout << "secondary aabb success ration: " << shadowSuccessfulAabbIntersections / (float)shadowAabbIntersections << std::endl;
			wasteFactor = (shadowLeafIntersectionCount + shadowNodeIntersectionCount - shadowRayCount) / (float)(shadowNodeIntersectionCount * bvh.branchingFactor);
			std::cout << "secondary waste factor: " << 1 - wasteFactor << std::endl;
			std::cout << "secondary intersections leaf: " << shadowLeafIntersectionCount * shadowFactor << std::endl;
			std::cout << "secondary primitive intersections: " << shadowPrimitiveIntersections * shadowFactor << std::endl;
			std::cout << "secondary primitive success ratio: " << shadowSuccessfulPrimitiveIntersections / (float)shadowPrimitiveIntersections << std::endl;
		}
		std::ofstream myfile(path + "/" + name + problem + "_Info.txt");
		if (myfile.is_open())
		{
			myfile << "scenario " << name << " with branching factor of " << std::to_string(bvh.branchingFactor) << " and leafsize of " << bvh.leafSize << std::endl;
			myfile << "intersections counts are normalized per Ray" << std::endl << std::endl;

			myfile << "primary intersections node: " << nodeIntersectionCount * factor << std::endl;
			myfile << "primary aabb intersections: " << aabbIntersections * factor << std::endl;
			myfile << "primary aabb success ration: " << successfulAabbIntersections / (float)aabbIntersections << std::endl;
			float wasteFactor = (leafIntersectionCount + nodeIntersectionCount - width * height) / (float)(nodeIntersectionCount * bvh.branchingFactor);
			myfile << "primary waste factor: " << 1 - wasteFactor << std::endl;
			myfile << "primary intersections leaf: " << leafIntersectionCount * factor << std::endl;
			myfile << "primary primitive intersections: " << primitiveIntersections * factor << std::endl;
			myfile << "primary primitive success ratio: " << successfulPrimitiveIntersections / (float)primitiveIntersections << std::endl;
			myfile << std::endl;
			myfile << "secondary intersections node: " << shadowNodeIntersectionCount * shadowFactor << std::endl;
			myfile << "secondary aabb intersections: " << shadowAabbIntersections * shadowFactor << std::endl;
			myfile << "secondary aabb success ration: " << shadowSuccessfulAabbIntersections / (float)shadowAabbIntersections << std::endl;
			wasteFactor = (shadowLeafIntersectionCount + shadowNodeIntersectionCount - shadowRayCount) / (float)(shadowNodeIntersectionCount * bvh.branchingFactor);
			myfile << "secondary waste factor: " << 1 - wasteFactor << std::endl;
			myfile << "secondary intersections leaf: " << shadowLeafIntersectionCount * shadowFactor << std::endl;
			myfile << "secondary primitive intersections: " << shadowPrimitiveIntersections * shadowFactor << std::endl;
			myfile << "secondary primitive success ratio: " << shadowSuccessfulPrimitiveIntersections / (float)shadowPrimitiveIntersections << std::endl;

			//this number is smaller than the nodecount + leafcount because the depth 0 intersections are left out
			//In addition: with shadow rays those also dont fit because shadowrays can stop when they find a tirangle
			myfile << std::endl;
			myfile << "intersections with nodes with x children :" << std::endl;
			float sum = 0;
			for (size_t i = 0; i < childFullness.size(); i++)
			{
				sum += childFullness[i] * i;
			}
			sum /= std::accumulate(childFullness.begin(), childFullness.end(), 0);
			myfile << "average child fullness: " << std::to_string(sum) << std::endl;
			for (size_t i = 0; i < childFullness.size(); i++)
			{
				//myfile << i << " : " << childFullness[i] * bothFactor << std::endl;
				myfile << i << " : " << childFullness[i] * factor << std::endl;
			}

			sum = 0;
			myfile << std::endl;
			myfile << "intersections with leaf nodes with x primitives :" << std::endl;
			for (size_t i = 0; i < primitiveFullness.size(); i++)
			{
				sum += primitiveFullness[i] * i;
			}
			sum /= std::accumulate(primitiveFullness.begin(), primitiveFullness.end(), 0);
			myfile << "average leaf fullness: " << std::to_string(sum) << std::endl;
			for (size_t i = 0; i < primitiveFullness.size(); i++)
			{
				//myfile << i << " : " << primitiveFullness[i] * bothFactor << std::endl;
				myfile << i << " : " << primitiveFullness[i] * factor << std::endl;
			}

			myfile << std::endl;
			myfile << "primary node intersections at depth x :" << std::endl;
			for (size_t i = 0; i < nodeIntersectionPerDepthCount.size(); i++)
			{
				myfile << i << " : " << nodeIntersectionPerDepthCount[i] * factor << std::endl;
			}
			myfile << std::endl;
			myfile << "primary leaf intersections at depth x :" << std::endl;
			for (size_t i = 0; i < leafIntersectionPerDepthCount.size(); i++)
			{
				myfile << i << " : " << leafIntersectionPerDepthCount[i] * factor << std::endl;
			}
			myfile << std::endl;
			myfile << "secondary node intersections at depth x :" << std::endl;
			for (size_t i = 0; i < shadowNodeIntersectionPerDepthCount.size(); i++)
			{
				myfile << i << " : " << shadowNodeIntersectionPerDepthCount[i] * shadowFactor << std::endl;
			}
			myfile << std::endl;
			myfile << "secondary leaf intersections at depth x :" << std::endl;
			for (size_t i = 0; i < shadowLeafIntersectionPerDepthCount.size(); i++)
			{
				myfile << i << " : " << shadowLeafIntersectionPerDepthCount[i] * shadowFactor << std::endl;
			}
			if (!saveDepthDebugImage)
			{
				myfile.close();
			}
		}
		else std::cerr << "Unable to open file for image analysis resutls" << std::endl;

		if (doWorkGroupAnalysis)
		{
			//some parts of this are kinda obsolete. (separated node and leafs, since only the combined one is intresting)

			//First part is the wisker plot of workload. -> need min, max, median , and standard deviation

			//lets look at average work group depth, variance (and standart deviation? )
			//std::ofstream fileWorkGroup0(path + "/" + name + problem + "_WorkGroupDepthInfo.txt");
			//std::ofstream fileWorkGroup1(path + "/" + name + problem + "_NodeWorkGroupWiskerPlot.txt");
			//std::ofstream fileWorkGroup2(path + "/" + name + problem + "_LeafWorkGroupWiskerPlot.txt");
			//std::ofstream fileWorkGroup3(path + "/" + name + problem + "_CombinedWorkGroupWiskerPlot.txt");

			std::ofstream fileWorkGroup0(path + "/" + name + problem + "_PrimaryWorkGroupWiskerPlot.txt");
			std::ofstream fileWorkGroup1(path + "/" + name + problem + "_SecondaryWorkGroupWiskerPlot.txt");


			//struct to store the per workgroup information so we can later sort it correctly
			struct StorageStruct
			{
				int median;
				int min;
				int max;
				float lowerSd;
				float upperSd;
				StorageStruct()
				{
				}
				StorageStruct(int median, int min, int max, float lowerSd, float upperSd)
					:median(median), min(min), max(max), lowerSd(lowerSd), upperSd(upperSd)
				{
				}

				bool operator< (const StorageStruct& other) const
				{
					if (median == other.median)
					{
						return max < other.max;
					}
					else
					{
						return median < other.median;
					}
				}
			};

			//later sorted by median
			std::vector<StorageStruct> nodeStorage((width / workGroupSize) * (height / workGroupSize));
			std::vector<StorageStruct> leafStorage((width / workGroupSize) * (height / workGroupSize));
			std::vector<StorageStruct> combinedStorage((width / workGroupSize) * (height / workGroupSize));
			std::vector<StorageStruct> secondaryCombinedStorage((width / workGroupSize) * (height / workGroupSize));

			if (fileWorkGroup0.is_open())
			{
				//output will be a table

				std::vector<int> nodeIntersections(workGroupSquare);
				std::vector<int> leafIntersections(workGroupSquare);
				std::vector<int> combinedIntersections(workGroupSquare);
				std::vector<int> secondaryCombinedIntersections(workGroupSquare);
				int medianId = workGroupSquare * 0.5f;
				//int lowerQuartileId = workGroupSquare * 0.25f;
				//int upperQuartileId = workGroupSquare * 0.75f;
				for (int i = 0; i < (width / workGroupSize) * (height / workGroupSize); i++)
				{
					int nodeSum = 0;
					int leafSum = 0;
					//take sum and max of node and leaf intersections
					for (int j = 0; j < workGroupSquare; j++)
					{
						int w = ((i * workGroupSize) % width) + (j % workGroupSize);
						int h = (((i * workGroupSize) / width) * workGroupSize) + (j / workGroupSize);
						int index = i * workGroupSquare + j;
						int index2 = w + h * width;
						int nodeInter = std::accumulate(nodeIntersectionPerPixelCount[index2].begin(), nodeIntersectionPerPixelCount[index2].end(), 0);
						int leafInter = std::accumulate(leafIntersectionPerPixelCount[index2].begin(), leafIntersectionPerPixelCount[index2].end(), 0);
						int secondaryNodeInter = std::accumulate(shadowNodeIntersectionPerPixelCount[index2].begin(), shadowNodeIntersectionPerPixelCount[index2].end(), 0);
						int secondaryLeafInter = std::accumulate(shadowLeafIntersectionPerPixelCount[index2].begin(), shadowLeafIntersectionPerPixelCount[index2].end(), 0);
						nodeSum += nodeInter;
						leafSum += leafInter;
						nodeIntersections[j] = nodeInter;
						leafIntersections[j] = leafInter;
						combinedIntersections[j] = nodeInter + leafInter;
						secondaryCombinedIntersections[j] = secondaryNodeInter + secondaryLeafInter;
					}
					float nodeAverage = nodeSum / (float)(workGroupSquare);
					float leafAverage = leafSum / (float)(workGroupSquare);
					float combinedAverage = nodeAverage + leafAverage;
					float secondaryCombinedAverage = std::accumulate(secondaryCombinedIntersections.begin(),
						secondaryCombinedIntersections.end(), 0) / (float)(workGroupSquare);

					float leafVariance = 0;
					float leafSd = 0;
					float nodeVariance = 0;
					float nodeSd = 0;
					float combinedVariance = 0;
					float combinedSd = 0;

					float secondaryCombinedVariance = 0;
					float secondaryCombinedSd = 0;
					//second loop for standard deviation and variance
					for (int j = 0; j < workGroupSquare; j++)
					{
						int w = ((i * workGroupSize) % width) + (j % workGroupSize);
						int h = (((i * workGroupSize) / width) * workGroupSize) + (j / workGroupSize);
						int index = i * workGroupSquare + j;
						int index2 = w + h * width;
						int nodeInter = std::accumulate(nodeIntersectionPerPixelCount[index2].begin(), nodeIntersectionPerPixelCount[index2].end(), 0);
						int leafInter = std::accumulate(leafIntersectionPerPixelCount[index2].begin(), leafIntersectionPerPixelCount[index2].end(), 0);
						int secondaryNodeInter = std::accumulate(shadowNodeIntersectionPerPixelCount[index2].begin(), shadowNodeIntersectionPerPixelCount[index2].end(), 0);
						int secondaryLeafInter = std::accumulate(shadowLeafIntersectionPerPixelCount[index2].begin(), shadowLeafIntersectionPerPixelCount[index2].end(), 0);
						leafVariance += pow(leafInter - leafAverage, 2);
						nodeVariance += pow(nodeInter - nodeAverage, 2);
						combinedVariance += pow((leafInter + nodeInter) - combinedAverage, 2);
						secondaryCombinedVariance += pow((secondaryNodeInter + secondaryLeafInter) - secondaryCombinedAverage, 2);
					}
					leafVariance /= workGroupSquare;
					nodeVariance /= workGroupSquare;
					combinedVariance /= workGroupSquare;
					secondaryCombinedVariance /= workGroupSquare;
					leafSd = sqrt(leafVariance);
					nodeSd = sqrt(nodeVariance);
					combinedSd = sqrt(combinedVariance);
					secondaryCombinedSd = sqrt(secondaryCombinedVariance);

					//median and quartile: 
					std::sort(nodeIntersections.begin(), nodeIntersections.end());
					std::sort(leafIntersections.begin(), leafIntersections.end());
					std::sort(combinedIntersections.begin(), combinedIntersections.end());
					std::sort(secondaryCombinedIntersections.begin(), secondaryCombinedIntersections.end());

					int leafMax = leafIntersections[workGroupSquare - 1];
					int nodeMax = nodeIntersections[workGroupSquare - 1];
					int combinedMax = combinedIntersections[workGroupSquare - 1];
					int secondaryCombinedMax = secondaryCombinedIntersections[workGroupSquare - 1];

					nodeStorage[i] = StorageStruct(nodeIntersections[medianId], nodeIntersections[0],
						nodeMax, nodeAverage - nodeSd, nodeAverage + nodeSd);
					leafStorage[i] = StorageStruct(leafIntersections[medianId], leafIntersections[0],
						leafMax, leafAverage - leafSd, leafAverage + leafSd);
					combinedStorage[i] = StorageStruct(combinedIntersections[medianId], combinedIntersections[0],
						combinedMax, combinedAverage - combinedSd, combinedAverage + combinedSd);
					secondaryCombinedStorage[i] = StorageStruct(secondaryCombinedIntersections[medianId], secondaryCombinedIntersections[0],
						secondaryCombinedMax, secondaryCombinedAverage - secondaryCombinedSd, secondaryCombinedAverage + secondaryCombinedSd);
				}
				std::sort(nodeStorage.begin(), nodeStorage.end());
				std::sort(leafStorage.begin(), leafStorage.end());
				std::sort(combinedStorage.begin(), combinedStorage.end());
				std::sort(secondaryCombinedStorage.begin(), secondaryCombinedStorage.end());

				fileWorkGroup0 << "median, min, max, lowerStdDeviation, upperStdDeviation" << std::endl;
				fileWorkGroup1 << "median, min, max, lowerStdDeviation, upperStdDeviation" << std::endl;

				for (int i = 0; i < (width / workGroupSize) * (height / workGroupSize); i++)
				{
					fileWorkGroup0 << combinedStorage[i].median << ", ";
					fileWorkGroup0 << combinedStorage[i].min << ", ";
					fileWorkGroup0 << combinedStorage[i].max << ", ";
					fileWorkGroup0 << combinedStorage[i].lowerSd << ", ";
					fileWorkGroup0 << combinedStorage[i].upperSd << std::endl;

					fileWorkGroup1 << secondaryCombinedStorage[i].median << ", ";
					fileWorkGroup1 << secondaryCombinedStorage[i].min << ", ";
					fileWorkGroup1 << secondaryCombinedStorage[i].max << ", ";
					fileWorkGroup1 << secondaryCombinedStorage[i].lowerSd << ", ";
					fileWorkGroup1 << secondaryCombinedStorage[i].upperSd << std::endl;
				}
				fileWorkGroup0.close();
				fileWorkGroup1.close();
			}
			else std::cerr << "Unable to open file for work group analysis" << std::endl;

			//second part is detailed analysis of when what work is done.
			if (wideRender)
			{
				std::ofstream fileWorkGroup(path + "/" + name + problem + "_WorkGroupData.txt");
				if (fileWorkGroup.is_open())
				{
					fileWorkGroup << "stepId, avgPrimaryNodeWork, avgPrimaryNodeUnique, avgPrimaryLeafWork, avgPrimaryLeafUnique,"
						<< " avgSecondaryNodeWork, avgSecondaryNodeUnique, avgSecondaryLeafWork, avgSecondaryLeafUnique" << std::endl;


					//what i want to show: for now average and standard deviation of the new values the workGroupRenderer collects
					//when what work is done, and the number of unique nodes / leafs that are used. For primary and secondary ray each

					//get max size of nodeWorkPerStep:
					int maxSize = 0;
					for (int i = 0; i < (width / workGroupSize) * (height / workGroupSize); i++)
					{
						if (nodeWorkPerStep[i].size() > maxSize)
						{
							maxSize = nodeWorkPerStep[i].size();
						}
						if (nodeWorkPerStep[i].size() > maxSize)
						{
							maxSize = secondaryNodeWorkPerStep[i].size();
						}
					}

					//average calculation is done per workStep
					std::vector<float> nodeWorkPerStepAverage(maxSize, 0);
					std::vector<float> leafWorkPerStepAverage(maxSize, 0);
					std::vector<float> uniqueNodesPerStepAverage(maxSize, 0);
					std::vector<float> uniqueLeafsPerStepAverage(maxSize, 0);

					std::vector<float> secondaryNodeWorkPerStepAverage(maxSize, 0);
					std::vector<float> secondaryLeafWorkPerStepAverage(maxSize, 0);
					std::vector<float> secondaryUniqueNodesPerStepAverage(maxSize, 0);
					std::vector<float> secondaryUniqueLeafsPerStepAverage(maxSize, 0);

					//go trough workSteps
					for (int j = 0; j < maxSize; j++)
					{
						//counter to calculate average.
						//general counters go over workgroups that still do work in this step
						//unique counters go over workroups that at least woked with one node / leaf
						int primaryCount = 0;
						int primaryNodeUniqueCount = 0;
						int primaryLeafUniqueCount = 0;
						int secondaryCount = 0;
						int secondaryNodeUniqueCount = 0;
						int secondaryLeafUniqueCount = 0;
						for (int i = 0; i < (width / workGroupSize) * (height / workGroupSize); i++)
						{
							if (nodeWorkPerStep[i].size() > j)
							{
								primaryCount++;
								nodeWorkPerStepAverage[j] += nodeWorkPerStep[i][j];
								leafWorkPerStepAverage[j] += leafWorkPerStep[i][j];
								uniqueNodesPerStepAverage[j] += uniqueNodesPerStep[i][j];
								uniqueLeafsPerStepAverage[j] += uniqueLeafsPerStep[i][j];

								if (uniqueNodesPerStep[i][j] != 0)
								{
									primaryNodeUniqueCount++;
								}
								if (uniqueLeafsPerStep[i][j] != 0)
								{
									primaryLeafUniqueCount++;
								}
							}
							if (secondaryNodeWorkPerStep[i].size() > j)
							{
								secondaryCount++;
								secondaryNodeWorkPerStepAverage[j] += secondaryNodeWorkPerStep[i][j];
								secondaryLeafWorkPerStepAverage[j] += secondaryLeafWorkPerStep[i][j];
								secondaryUniqueNodesPerStepAverage[j] += secondaryUniqueNodesPerStep[i][j];
								secondaryUniqueLeafsPerStepAverage[j] += secondaryUniqueLeafsPerStep[i][j];

								if (secondaryUniqueNodesPerStep[i][j] != 0)
								{
									secondaryNodeUniqueCount++;
								}
								if (secondaryUniqueLeafsPerStep[i][j] != 0)
								{
									secondaryLeafUniqueCount++;
								}
							}
						}
						//fix nan error for 0/0:
						primaryCount = std::max(primaryCount, 1);
						primaryNodeUniqueCount = std::max(primaryNodeUniqueCount, 1);
						primaryLeafUniqueCount = std::max(primaryLeafUniqueCount, 1);
						secondaryCount = std::max(secondaryCount, 1);
						secondaryNodeUniqueCount = std::max(secondaryNodeUniqueCount, 1);
						secondaryLeafUniqueCount = std::max(secondaryLeafUniqueCount, 1);

						nodeWorkPerStepAverage[j] /= (float)primaryCount;
						leafWorkPerStepAverage[j] /= (float)primaryCount;
						uniqueNodesPerStepAverage[j] /= (float)primaryNodeUniqueCount;
						uniqueLeafsPerStepAverage[j] /= (float)primaryLeafUniqueCount;

						secondaryNodeWorkPerStepAverage[j] /= (float)secondaryCount;
						secondaryLeafWorkPerStepAverage[j] /= (float)secondaryCount;
						secondaryUniqueNodesPerStepAverage[j] /= (float)secondaryNodeUniqueCount;
						secondaryUniqueLeafsPerStepAverage[j] /= (float)secondaryLeafUniqueCount;

						//could also do standard deviation? (mean doesnt really make sense)

						//values are written into file in step order.
						fileWorkGroup << j << ", ";
						fileWorkGroup << nodeWorkPerStepAverage[j] << ", ";
						fileWorkGroup << uniqueNodesPerStepAverage[j] << ", ";
						fileWorkGroup << leafWorkPerStepAverage[j] << ", ";
						fileWorkGroup << uniqueLeafsPerStepAverage[j] << ", ";

						fileWorkGroup << secondaryNodeWorkPerStepAverage[j] << ", ";
						fileWorkGroup << secondaryUniqueNodesPerStepAverage[j] << ", ";
						fileWorkGroup << secondaryLeafWorkPerStepAverage[j] << ", ";
						fileWorkGroup << secondaryUniqueLeafsPerStepAverage[j] << std::endl;

					}
				}
				else std::cerr << "Unable to open file for work group second analysis" << std::endl;
			}
		}

		if (saveImage)
		{
			encodeTwoSteps(path + "/" + name + ".png", image, width, height);
		}
		if (saveDepthDebugImage)
		{
			myfile << std::endl;
			myfile << "max node intersections at depth x :" << std::endl;
			//save an image with all the aabb intersections for every depth
			for (size_t d = 0; d < nodeIntersectionPerDepthCount.size(); d++)
			{
				uint16_t depthIntersections = 0;
				//find max element first to normalise to;
#pragma omp parallel for schedule(static, 128)
				for (int i = 0; i < width * height; i++)
				{
					if (d < nodeIntersectionPerPixelCount[i].size())
					{
						depthIntersections = std::max(nodeIntersectionPerPixelCount[i][d], depthIntersections);
					}
				}

				myfile << d << " : " << depthIntersections << std::endl;

				//go trough RenderInfo vector and use the stored nodeIntersectionPerPixelCount
#pragma omp parallel for schedule(static, 128)
				for (int i = 0; i < width * height; i++)
				{
					uint16_t sum = 0;
					if (d < nodeIntersectionPerPixelCount[i].size())
					{
						sum = nodeIntersectionPerPixelCount[i][d];
					}

					//Color c(sum * 0.01f);
					Color c(sum * (1 / (float)depthIntersections));
					image[i * 4 + 0] = (uint8_t)(c.r * 255);
					image[i * 4 + 1] = (uint8_t)(c.g * 255);
					image[i * 4 + 2] = (uint8_t)(c.b * 255);
					image[i * 4 + 3] = (uint8_t)(c.a * 255);
				}
				encodeTwoSteps(path + "/" + name + problem + "_NodeDepth" + std::to_string(d) + ".png", image, width, height);
			}

			unsigned maxNodeSum = 0;
			unsigned maxLeafSum = 0;
			//find min and max for nodeintersection and leafintersection: (and number of different depth intersections
#pragma omp parallel for schedule(static, 128)
			for (int i = 0; i < width * height; i++)
			{
				unsigned sum = std::accumulate(nodeIntersectionPerPixelCount[i].begin(), nodeIntersectionPerPixelCount[i].end(), 0);
				maxNodeSum = std::max(sum, maxNodeSum);
				sum = std::accumulate(leafIntersectionPerPixelCount[i].begin(), leafIntersectionPerPixelCount[i].end(), 0);
				maxLeafSum = std::max(sum, maxLeafSum);
			}
			float normalisation = 1 / (float)maxNodeSum;
			//pixel bvh depth
			//std::cout << normalisation << std::endl;
			//std::cout << minSum << std::endl;
			//std::cout << maxSum << std::endl;
#pragma omp parallel for schedule(static, 128)
			for (int i = 0; i < width * height; i++)
			{
				unsigned sum = std::accumulate(nodeIntersectionPerPixelCount[i].begin(), nodeIntersectionPerPixelCount[i].end(), 0);
				Color c(sum * normalisation);
				image[i * 4 + 0] = (uint8_t)(c.r * 255);
				image[i * 4 + 1] = (uint8_t)(c.g * 255);
				image[i * 4 + 2] = (uint8_t)(c.b * 255);
				image[i * 4 + 3] = (uint8_t)(c.a * 255);
			}
			encodeTwoSteps(path + "/" + name + problem + "_NodeIntersectionCount.png", image, width, height);

			myfile << std::endl;
			myfile << "max leaf intersections :" << maxLeafSum << std::endl;
			normalisation = 1.0 / maxLeafSum;
			//std::cout << maxLeafSum << std::endl;
#pragma omp parallel for schedule(static, 128)
			for (int i = 0; i < width * height; i++)
			{
				unsigned sum = std::accumulate(leafIntersectionPerPixelCount[i].begin(), leafIntersectionPerPixelCount[i].end(), 0);
				Color c(sum * normalisation);
				image[i * 4 + 0] = (uint8_t)(c.r * 255);
				image[i * 4 + 1] = (uint8_t)(c.g * 255);
				image[i * 4 + 2] = (uint8_t)(c.b * 255);
				image[i * 4 + 3] = (uint8_t)(c.a * 255);
			}
			encodeTwoSteps(path + "/" + name + problem + "_LeafIntersectionCount.png", image, width, height);
			myfile.close();
		}
	}

private:
	template<typename T>
	bool shootShadowRay(Ray& shadowRay, Ray& ray, RenderInfo& info, Bvh& bvh, CompactNodeManager<T>& nodeManager, int& renderType)
	{
		bool result;
		switch (renderType)
		{
		case 0:
			result = bvh.intersect(shadowRay);
			break;
		case 1:
			result = nodeManager.intersect(shadowRay);
			break;
		case 2:
			result = nodeManager.intersectImmediately(shadowRay, false);
			break;
		case 3:
			result = nodeManager.intersectImmediately(shadowRay, true);
			break;
		default:
			result = nodeManager.intersectImmediately(shadowRay, true);
			break;
		}

		collectSecondaryRayData(shadowRay, info.index);
		return result;
	}

	void collectPrimaryRayData(Ray ray, int id)
	{
		//make sure i call the method correct in the future:
		if (ray.shadowRay)
		{
			std::cerr << "collect Primary data primaryRay is not a primary ray" << std::endl;
		}


		nodeIntersectionPerPixelCount[id] = ray.nodeIntersectionCount;
		leafIntersectionPerPixelCount[id] = ray.leafIntersectionCount;
		childFullnessPerPixelCount[id] = ray.childFullness;
		primitiveFullnessPerPixelCount[id] = ray.primitiveFullness;

		primitiveIntersectionsPerPixel[id] += ray.primitiveIntersectionCount;
		successfulPrimitiveIntersectionsPerPixel[id] += ray.successfulPrimitiveIntersectionCount;
		successfulAabbIntersectionsPerPixel[id] += ray.successfulAabbIntersectionCount;
		aabbIntersectionsPerPixel[id] += ray.aabbIntersectionCount;
	}

	void collectSecondaryRayData(Ray shadowRay, int id)
	{
		//make sure i call the method correct in the future:
		if (!shadowRay.shadowRay)
		{
			std::cerr << "collect secondary data shadowray is not a shadowray" << std::endl;
		}
		shadowRayCounter[id] ++;
		if (shadowNodeIntersectionPerPixelCount[id].size() < shadowRay.nodeIntersectionCount.size())
		{
			shadowNodeIntersectionPerPixelCount[id].resize(shadowRay.nodeIntersectionCount.size());
		}
		for (size_t i = 0; i < shadowRay.nodeIntersectionCount.size(); i++)
		{
			shadowNodeIntersectionPerPixelCount[id][i] += shadowRay.nodeIntersectionCount[i];
		}

		if (shadowLeafIntersectionPerPixelCount[id].size() < shadowRay.leafIntersectionCount.size())
		{
			shadowLeafIntersectionPerPixelCount[id].resize(shadowRay.leafIntersectionCount.size());
		}
		for (size_t i = 0; i < shadowRay.leafIntersectionCount.size(); i++)
		{
			shadowLeafIntersectionPerPixelCount[id][i] += shadowRay.leafIntersectionCount[i];
		}

		shadowPrimitiveIntersectionsPerPixel[id] += shadowRay.primitiveIntersectionCount;

		//this is not correct since we can have multiple hits for our intersect (since its "simd"
		//if (shadowRay.successfulPrimitiveIntersectionCount > 1)
		//{
		//	std::cerr << "error: more than 1 successful shadowray primitive intersection for one light" << std::endl;
		//}
		shadowSuccessfulPrimitiveIntersectionsPerPixel[id] += shadowRay.successfulPrimitiveIntersectionCount;
		shadowSuccessfulAabbIntersectionsPerPixel[id] += shadowRay.successfulAabbIntersectionCount;
		shadowAabbIntersectionsPerPixel[id] += shadowRay.aabbIntersectionCount;
	}

	inline void initializeVariables()
	{
		nodeIntersectionPerPixelCount.resize(height * width);
		leafIntersectionPerPixelCount.resize(height * width);
		shadowNodeIntersectionPerPixelCount.resize(height * width);
		shadowLeafIntersectionPerPixelCount.resize(height * width);
		childFullnessPerPixelCount.resize(height * width);
		primitiveFullnessPerPixelCount.resize(height * width);
		primitiveIntersectionsPerPixel.resize(height * width);
		successfulPrimitiveIntersectionsPerPixel.resize(height * width);
		successfulAabbIntersectionsPerPixel.resize(height * width);
		aabbIntersectionsPerPixel.resize(height * width);
		shadowSuccessfulAabbIntersectionsPerPixel.resize(height * width);
		shadowRayCounter.resize(height * width);
		shadowAabbIntersectionsPerPixel.resize(height * width);
		shadowPrimitiveIntersectionsPerPixel.resize(height * width);
		shadowSuccessfulPrimitiveIntersectionsPerPixel.resize(height * width);

		nodeIntersectionCount = 0;
		leafIntersectionCount = 0;
		primitiveIntersections = 0;
		shadowPrimitiveIntersections = 0;
		successfulPrimitiveIntersections = 0;
		successfulAabbIntersections = 0;
		aabbIntersections = 0;
		shadowSuccessfulAabbIntersections = 0;
		shadowAabbIntersections = 0;
		shadowSuccessfulPrimitiveIntersections = 0;
		shadowRayCount = 0;

		if (wideRender)
		{
			int size = (width / nonTemplateWorkGroupSize) * (height / nonTemplateWorkGroupSize);

			nodeWorkPerStep.resize(size);
			leafWorkPerStep.resize(size);
			uniqueNodesPerStep.resize(size);
			uniqueLeafsPerStep.resize(size);

			secondaryNodeWorkPerStep.resize(size);
			secondaryLeafWorkPerStep.resize(size);
			secondaryUniqueNodesPerStep.resize(size);
			secondaryUniqueLeafsPerStep.resize(size);
		}
	}
};