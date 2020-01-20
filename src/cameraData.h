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

	//per workGroup per step: (those are the only arrays that are reset for every camera)
	std::vector<std::vector<uint32_t>> nodeWorkPerStep;
	std::vector<std::vector<uint32_t>> leafWorkPerStep;
	std::vector<std::vector<uint32_t>> terminationsPerStep;
	std::vector<std::vector<uint32_t>> uniqueNodesPerStep;
	std::vector<std::vector<uint32_t>> uniqueLeafsPerStep;

	std::vector<std::vector<uint32_t>> secondaryNodeWorkPerStep;
	std::vector<std::vector<uint32_t>> secondaryLeafWorkPerStep;
	std::vector<std::vector<uint32_t>> secondaryTerminationsPerStep;
	std::vector<std::vector<uint32_t>> secondaryUniqueNodesPerStep;
	std::vector<std::vector<uint32_t>> secondaryUniqueLeafsPerStep;

	//extra counter to calculate per camrea values from other counter
	std::vector<uint32_t> nodeIntersectionPerPixelCountCameraSum;
	std::vector<uint32_t> shadowNodeIntersectionPerPixelCountCameraSum;
	std::vector<uint32_t> leafIntersectionPerPixelCountCameraSum;
	std::vector<uint32_t> shadowLeafIntersectionPerPixelCountCameraSum;

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

	CameraData(std::string path, std::string name, std::string problem, int workGroupSize, bool wideRender,
		std::vector<glm::vec3>& positions, std::vector<glm::vec3>& lookCenters, size_t width = 1920, size_t height = 1088, glm::vec3 upward = glm::vec3(0, 1, 0), float focalLength = 0.866f)
		:Camera(path, name, problem, workGroupSize, positions, lookCenters, width, height, upward, focalLength), wideRender(wideRender)
	{
		image.resize(height * width * 4);
		initializeVariables();
	}

	CameraData(std::string path, std::string name, std::string problem, int workGroupSize, bool wideRender, std::vector<glm::mat4>& transforms,
		size_t width = 1920, size_t height = 1088, float focalLength = 0.866f)
		:Camera(path, name, problem, workGroupSize, transforms, width, height, focalLength), wideRender(wideRender)
	{
		image.resize(height * width * 4);
		initializeVariables();
	}

	template<typename T>
	void renderImages(bool saveImage, bool saveDepthDebugImage, CompactNodeManager<T>& nodeManager
		, Bvh& bvh, std::vector<std::unique_ptr<Light>>& lights, unsigned ambientSampleCount,
		float ambientDistance, bool castShadows, int renderType, bool mute, bool doWorkGroupAnalysis, bool wideAlternative)
	{
		int workGroupSize = nonTemplateWorkGroupSize;
		int cameraCount = positions.size();

		for (int cameraId = 0; cameraId < cameraCount; cameraId++)
		{
			//reset wide render storage arrays
			if (wideRender)
			{
				initializeWideRenderVariables();
			}

			renderImage(saveImage, saveDepthDebugImage, nodeManager, bvh, lights, ambientSampleCount,
				ambientDistance, castShadows, renderType, mute, doWorkGroupAnalysis, cameraId, wideAlternative);

			//save image for each camera and the data from wideRender
			std::string cameraName = "_c" + std::to_string(cameraId);
			if (doWorkGroupAnalysis)
			{
				if (wideRender)
				{
					workGroupAnalysisPerImage(cameraName, wideAlternative);
				}
				else
				{
					std::cerr << "Workgroup analysis is not possible without wide renderer!" << std::endl;
				}
			}

			if (saveImage)
			{
				encodeTwoSteps(path + "/" + name + cameraName + ".png", image, width, height);
			}
			if (cameraId == 0 && saveDepthDebugImage)
			{
				createDepthDebugImage(bvh.bvhDepth);
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

		leafIntersectionCount = std::accumulate(leafIntersectionPerDepthCount.begin(), leafIntersectionPerDepthCount.end(), 0ULL);
		nodeIntersectionCount = std::accumulate(nodeIntersectionPerDepthCount.begin(), nodeIntersectionPerDepthCount.end(), 0ULL);

		shadowLeafIntersectionCount = std::accumulate(shadowLeafIntersectionPerDepthCount.begin(), shadowLeafIntersectionPerDepthCount.end(), 0ULL);
		shadowNodeIntersectionCount = std::accumulate(shadowNodeIntersectionPerDepthCount.begin(), shadowNodeIntersectionPerDepthCount.end(), 0ULL);

		primitiveIntersections = std::accumulate(primitiveIntersectionsPerPixel.begin(), primitiveIntersectionsPerPixel.end(), 0ULL);
		successfulPrimitiveIntersections = std::accumulate(successfulPrimitiveIntersectionsPerPixel.begin(), successfulPrimitiveIntersectionsPerPixel.end(), 0ULL);
		successfulAabbIntersections = std::accumulate(successfulAabbIntersectionsPerPixel.begin(), successfulAabbIntersectionsPerPixel.end(), 0ULL);
		aabbIntersections = std::accumulate(aabbIntersectionsPerPixel.begin(), aabbIntersectionsPerPixel.end(), 0ULL);
		shadowRayCount = std::accumulate(shadowRayCounter.begin(), shadowRayCounter.end(), 0ULL);
		shadowSuccessfulPrimitiveIntersections = std::accumulate(shadowSuccessfulPrimitiveIntersectionsPerPixel.begin(), shadowSuccessfulPrimitiveIntersectionsPerPixel.end(), 0ULL);
		shadowSuccessfulAabbIntersections = std::accumulate(shadowSuccessfulAabbIntersectionsPerPixel.begin(), shadowSuccessfulAabbIntersectionsPerPixel.end(), 0ULL);
		shadowAabbIntersections = std::accumulate(shadowAabbIntersectionsPerPixel.begin(), shadowAabbIntersectionsPerPixel.end(), 0ULL);
		shadowPrimitiveIntersections = std::accumulate(shadowPrimitiveIntersectionsPerPixel.begin(), shadowPrimitiveIntersectionsPerPixel.end(), 0ULL);

		//normalize by ray
		int primaryRayCount = width * height * cameraCount;
		float factor = 1 / (float)(primaryRayCount);
		float shadowFactor = 1 / (float)shadowRayCount;
		float bothFactor = 1 / (float)(primaryRayCount + shadowRayCount);

		if (!mute)
		{
			std::cout << "intersections counts are normalized per Ray" << std::endl << std::endl;
			std::cout << "primary intersections node: " << nodeIntersectionCount * factor << std::endl;
			std::cout << "primary aabb intersections: " << aabbIntersections * factor << std::endl;
			std::cout << "primary aabb success ration: " << successfulAabbIntersections / (float)aabbIntersections << std::endl;
			//wastefactor = "verschwendungsgrad".
			// basically number of nodes visited / number of aabb tested
			//the minus primaryRayCount is basically -1 for each ray -> needed to get right values
			float wasteFactor = (leafIntersectionCount + nodeIntersectionCount - primaryRayCount) / (float)(nodeIntersectionCount * bvh.branchingFactor);
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
			float wasteFactor = (leafIntersectionCount + nodeIntersectionCount - primaryRayCount) / (float)(nodeIntersectionCount * bvh.branchingFactor);
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
		myfile.close();
	}

	//spawns rays and collects results into image. Image is written on disk
	template<typename T>
	void renderImage(bool saveImage, bool saveDepthDebugImage, CompactNodeManager<T>& nodeManager
		, Bvh& bvh, std::vector<std::unique_ptr<Light>>& lights, unsigned ambientSampleCount,
		float ambientDistance, bool castShadows, int renderType, bool mute, bool doWorkGroupAnalysis, int cameraId, bool wideAlternative)
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

							//could use light color
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
				if (wideAlternative)
				{
					nodeManager.intersectWideAlternative(rays, nodeWorkPerStep[i], leafWorkPerStep[i], uniqueNodesPerStep[i], uniqueLeafsPerStep[i], terminationsPerStep[i]);
				}
				else
				{
					nodeManager.intersectWide(rays, nodeWorkPerStep[i], leafWorkPerStep[i], uniqueNodesPerStep[i], uniqueLeafsPerStep[i], terminationsPerStep[i]);
				}

				//shoot secondary rays
				std::vector<Ray> secondaryRays(workGroupSquare);
				std::vector<uint8_t> ambientResult(workGroupSquare);
				for (int a = 0; a < ambientSampleCount; a++)
				{
					for (int j = 0; j < workGroupSquare; j++)
					{
						if (isnan(rays[j].surfacePosition.x))
						{
							secondaryRays[j] = Ray(glm::vec3(NAN), glm::vec3(NAN), bvh, true);
							secondaryRays[j].tMax = NAN;
						}
						else
						{
							auto direction = getAmbientDirection(i * workGroupSquare + j, rays[j].surfaceNormal, a);
							secondaryRays[j] = Ray(rays[j].surfacePosition + rays[j].surfaceNormal * 0.001f, direction, bvh, true);
							secondaryRays[j].tMax = ambientDistance;
						}
					}

					if (wideAlternative)
					{
						nodeManager.intersectWideAlternative(secondaryRays, secondaryNodeWorkPerStep[i], secondaryLeafWorkPerStep[i],
							secondaryUniqueNodesPerStep[i], secondaryUniqueLeafsPerStep[i], secondaryTerminationsPerStep[i]);
					}
					else
					{
						nodeManager.intersectWide(secondaryRays, secondaryNodeWorkPerStep[i], secondaryLeafWorkPerStep[i],
							secondaryUniqueNodesPerStep[i], secondaryUniqueLeafsPerStep[i], secondaryTerminationsPerStep[i]);
					}

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

	void collectPrimaryRayData(const Ray& ray, int id)
	{
		//make sure i call the method correct in the future:
		if (ray.shadowRay)
		{
			std::cerr << "collect Primary data primaryRay is not a primary ray" << std::endl;
		}

		if (nodeIntersectionPerPixelCount[id].size() < ray.nodeIntersectionCount.size())
		{
			nodeIntersectionPerPixelCount[id].resize(ray.nodeIntersectionCount.size());
		}
		for (size_t i = 0; i < ray.nodeIntersectionCount.size(); i++)
		{
			nodeIntersectionPerPixelCount[id][i] += ray.nodeIntersectionCount[i];
		}

		if (leafIntersectionPerPixelCount[id].size() < ray.leafIntersectionCount.size())
		{
			leafIntersectionPerPixelCount[id].resize(ray.leafIntersectionCount.size());
		}
		for (size_t i = 0; i < ray.leafIntersectionCount.size(); i++)
		{
			leafIntersectionPerPixelCount[id][i] += ray.leafIntersectionCount[i];
		}

		if (childFullnessPerPixelCount[id].size() < ray.childFullness.size())
		{
			childFullnessPerPixelCount[id].resize(ray.childFullness.size());
		}
		for (size_t i = 0; i < ray.childFullness.size(); i++)
		{
			childFullnessPerPixelCount[id][i] += ray.childFullness[i];
		}

		if (primitiveFullnessPerPixelCount[id].size() < ray.primitiveFullness.size())
		{
			primitiveFullnessPerPixelCount[id].resize(ray.primitiveFullness.size());
		}
		for (size_t i = 0; i < ray.primitiveFullness.size(); i++)
		{
			primitiveFullnessPerPixelCount[id][i] += ray.primitiveFullness[i];
		}

		primitiveIntersectionsPerPixel[id] += ray.primitiveIntersectionCount;
		successfulPrimitiveIntersectionsPerPixel[id] += ray.successfulPrimitiveIntersectionCount;
		successfulAabbIntersectionsPerPixel[id] += ray.successfulAabbIntersectionCount;
		aabbIntersectionsPerPixel[id] += ray.aabbIntersectionCount;
	}

	void collectSecondaryRayData(const Ray& shadowRay, int id)
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
		shadowSuccessfulPrimitiveIntersectionsPerPixel[id] += shadowRay.successfulPrimitiveIntersectionCount;
		shadowSuccessfulAabbIntersectionsPerPixel[id] += shadowRay.successfulAabbIntersectionCount;
		shadowAabbIntersectionsPerPixel[id] += shadowRay.aabbIntersectionCount;
	}

	void workGroupAnalysisPerImage(std::string& cameraName, bool wideAlternative)
	{
		int workGroupSize = nonTemplateWorkGroupSize;

		//First part is the wisker plot of workload. -> need min, max, median , and standard deviation

		std::string sizeName = "WorkGroupSize_" + std::to_string(workGroupSize) + "_Version_" + std::to_string(wideAlternative);;
		std::ofstream fileWorkGroup0(path + "/" + sizeName + "/" + name + problem + cameraName + "_PrimaryWorkGroupWiskerPlot.txt");
		std::ofstream fileWorkGroup1(path + "/" + sizeName + "/" + name + problem + cameraName + "_SecondaryWorkGroupWiskerPlot.txt");


		//struct to store the per workgroup information so we can later sort it correctly
		struct StorageStruct
		{
			int median;
			int min;
			int max;
			float avg;
			float sd;
			StorageStruct()
			{
			}
			StorageStruct(int median, int min, int max, float avg, float sd)
				:median(median), min(min), max(max), avg(avg), sd(sd)
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
		std::vector<StorageStruct> primaryStorage((width / workGroupSize) * (height / workGroupSize));
		std::vector<StorageStruct> secondaryStorage((width / workGroupSize) * (height / workGroupSize));

		if (fileWorkGroup0.is_open() && fileWorkGroup1.is_open())
		{
			int medianId = workGroupSquare * 0.5f;
			for (int i = 0; i < (width / workGroupSize) * (height / workGroupSize); i++)
			{
				std::vector<int> primaryDepth;
				primaryDepth.reserve(workGroupSquare);
				std::vector<int> secondaryDepth;
				secondaryDepth.reserve(workGroupSquare);

				int primaryWorkSum = 0;
				int secondaryWorkSum = 0;

				//make sure terminations per step counter are correct
				if (std::accumulate(terminationsPerStep[i].begin(), terminationsPerStep[i].end(), 0) != workGroupSquare)
				{
					std::cerr << "primary ray workgroup termination count != rays" << std::endl;
				}
				if (std::accumulate(secondaryTerminationsPerStep[i].begin(), secondaryTerminationsPerStep[i].end(), 0) != workGroupSquare)
				{
					std::cerr << "secondary ray workgroup termination count != rays" << std::endl;
				}

				//write depth of each ray into vector to calculate median, max, average, ez.
				for (int tId = 0; tId < terminationsPerStep[i].size(); tId++)
				{
					primaryWorkSum += tId * terminationsPerStep[i][tId];
					for (int j = 0; j < terminationsPerStep[i][tId]; j++)
					{
						primaryDepth.push_back(tId);
					}
				}
				for (int tId = 0; tId < secondaryTerminationsPerStep[i].size(); tId++)
				{
					secondaryWorkSum += tId * secondaryTerminationsPerStep[i][tId];
					for (int j = 0; j < secondaryTerminationsPerStep[i][tId]; j++)
					{
						secondaryDepth.push_back(tId);
					}
				}

				if (primaryDepth.size() != workGroupSquare)
				{
					std::cerr << "primaryDepth array in workgroup analysis has wrong size " << std::endl;
				}
				if (secondaryDepth.size() != workGroupSquare)
				{
					std::cerr << "secondaryDepth array in workgroup analysis has wrong size " << std::endl;
				}

				//take sum and max of node and leaf intersections

				float primaryWorkAverage = primaryWorkSum / (float)workGroupSquare;
				float secondaryWorkAverage = secondaryWorkSum / (float)(workGroupSquare);

				float primaryVariance = 0;
				float primarySd = 0;

				float secondaryVariance = 0;
				float secondarySd = 0;
				//loop for standard deviation / variance
				for (int j = 0; j < workGroupSquare; j++)
				{
					primaryVariance += pow((primaryDepth[j]) - primaryWorkAverage, 2);
					secondaryVariance += pow((secondaryDepth[j]) - secondaryWorkAverage, 2);
				}
				primaryVariance /= workGroupSquare;
				secondaryVariance /= workGroupSquare;
				primarySd = sqrt(primaryVariance);
				secondarySd = sqrt(secondaryVariance);

				//median and quartile: 
				int primaryMax = primaryDepth[workGroupSquare - 1];
				int secondaryMax = secondaryDepth[workGroupSquare - 1];

				primaryStorage[i] = StorageStruct(primaryDepth[medianId], primaryDepth[0],
					primaryMax, primaryWorkAverage, primarySd);
				secondaryStorage[i] = StorageStruct(secondaryDepth[medianId], secondaryDepth[0],
					secondaryMax, secondaryWorkAverage, secondarySd);
			}
			std::sort(primaryStorage.begin(), primaryStorage.end());
			std::sort(secondaryStorage.begin(), secondaryStorage.end());

			fileWorkGroup0 << "median, min, max, lowerStdDeviation, upperStdDeviation" << std::endl;
			fileWorkGroup1 << "median, min, max, lowerStdDeviation, upperStdDeviation" << std::endl;

			for (int i = 0; i < (width / workGroupSize) * (height / workGroupSize); i++)
			{
				fileWorkGroup0 << primaryStorage[i].median << ", ";
				fileWorkGroup0 << primaryStorage[i].min << ", ";
				fileWorkGroup0 << primaryStorage[i].max << ", ";
				fileWorkGroup0 << primaryStorage[i].avg << ", ";
				fileWorkGroup0 << primaryStorage[i].sd << std::endl;

				fileWorkGroup1 << secondaryStorage[i].median << ", ";
				fileWorkGroup1 << secondaryStorage[i].min << ", ";
				fileWorkGroup1 << secondaryStorage[i].max << ", ";
				fileWorkGroup1 << secondaryStorage[i].avg << ", ";
				fileWorkGroup1 << secondaryStorage[i].sd << std::endl;
			}
			fileWorkGroup0.close();
			fileWorkGroup1.close();
		}
		else std::cerr << "Unable to open file for work group whisker plots" << std::endl;

		//second part is detailed analysis of when what work is done.
		std::ofstream fileWorkGroup(path + "/" + sizeName + "/" + name + problem + cameraName + "_WorkGroupData.txt");
		if (fileWorkGroup.is_open())
		{
			//what i want to show: for now average and ?standard deviation? of the new values the workGroupRenderer collects
			//when what work is done, and the number of unique nodes / leafs that are used. For primary and secondary ray each
			//in addition also how many rays terminate at each step(avg).

			fileWorkGroup << "stepId, avgPrimaryNodeWork, avgPrimaryNodeUnique, avgPrimaryLeafWork, avgPrimaryLeafUnique, avgPrimaryRayTermination, primaryNodeWorkMax, primaryNodeWorkMin, primaryLeafWorkMax, primaryLeafWorkMin"
				<< " avgSecondaryNodeWork, avgSecondaryNodeUnique, avgSecondaryLeafWork, avgSecondaryLeafUnique, avgSecondaryRayTermination, secondaryNodeWorkMax, secondaryNodeWorkMin, secondaryLeafWorkMax, secondaryLeafWorkMin" << std::endl;

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
			std::vector<float> terminatedRaysStepAverage(maxSize, 0);

			std::vector<float> secondaryNodeWorkPerStepAverage(maxSize, 0);
			std::vector<float> secondaryLeafWorkPerStepAverage(maxSize, 0);
			std::vector<float> secondaryUniqueNodesPerStepAverage(maxSize, 0);
			std::vector<float> secondaryUniqueLeafsPerStepAverage(maxSize, 0);
			std::vector<float> secondaryTerminatedRaysStepAverage(maxSize, 0);
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

				uint32_t primaryNodeUniqueMax = 0;
				uint32_t primaryNodeUniqueMin = workGroupSquare + 1;
				uint32_t primaryLeafUniqueMax = 0;
				uint32_t primaryLeafUniqueMin = workGroupSquare + 1;
				uint32_t secondaryNodeUniqueMax = 0;
				uint32_t secondaryNodeUniqueMin = workGroupSquare + 1;
				uint32_t secondaryLeafUniqueMax = 0;
				uint32_t secondaryLeafUniqueMin = workGroupSquare + 1;
				for (int i = 0; i < (width / workGroupSize) * (height / workGroupSize); i++)
				{
					if (terminationsPerStep[i].size() > j)
					{
						primaryCount++;

						nodeWorkPerStepAverage[j] += nodeWorkPerStep[i][j];
						leafWorkPerStepAverage[j] += leafWorkPerStep[i][j];
						uniqueNodesPerStepAverage[j] += uniqueNodesPerStep[i][j];
						uniqueLeafsPerStepAverage[j] += uniqueLeafsPerStep[i][j];
						terminatedRaysStepAverage[j] += terminationsPerStep[i][j];

						if (uniqueNodesPerStep[i][j] != 0)
						{
							primaryNodeUniqueCount++;
							primaryNodeUniqueMax = std::max(uniqueNodesPerStep[i][j], primaryNodeUniqueMax);
							primaryNodeUniqueMin = std::min(uniqueNodesPerStep[i][j], primaryNodeUniqueMin);
						}
						if (uniqueLeafsPerStep[i][j] != 0)
						{
							primaryLeafUniqueCount++;
							primaryLeafUniqueMax = std::max(uniqueLeafsPerStep[i][j], primaryLeafUniqueMax);
							primaryLeafUniqueMin = std::min(uniqueLeafsPerStep[i][j], primaryLeafUniqueMin);
						}
					}

					if (secondaryNodeWorkPerStep[i].size() > j)
					{
						secondaryCount++;
						secondaryNodeWorkPerStepAverage[j] += secondaryNodeWorkPerStep[i][j];
						secondaryLeafWorkPerStepAverage[j] += secondaryLeafWorkPerStep[i][j];
						secondaryUniqueNodesPerStepAverage[j] += secondaryUniqueNodesPerStep[i][j];
						secondaryUniqueLeafsPerStepAverage[j] += secondaryUniqueLeafsPerStep[i][j];
						secondaryTerminatedRaysStepAverage[j] += secondaryTerminationsPerStep[i][j];

						if (secondaryUniqueNodesPerStep[i][j] != 0)
						{
							secondaryNodeUniqueCount++;
							secondaryNodeUniqueMax = std::max(secondaryUniqueNodesPerStep[i][j], secondaryNodeUniqueMax);
							secondaryNodeUniqueMin = std::min(secondaryUniqueNodesPerStep[i][j], secondaryNodeUniqueMin);
						}
						if (secondaryUniqueLeafsPerStep[i][j] != 0)
						{
							secondaryLeafUniqueCount++;
							secondaryLeafUniqueMax = std::max(secondaryUniqueLeafsPerStep[i][j], secondaryLeafUniqueMax);
							secondaryLeafUniqueMin = std::min(secondaryUniqueLeafsPerStep[i][j], secondaryLeafUniqueMin);
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

				//set those min that didnt change 0:
				if (primaryNodeUniqueMin == workGroupSquare + 1)
				{
					primaryNodeUniqueMin = 0;
				}
				if (primaryLeafUniqueMin == workGroupSquare + 1)
				{
					primaryLeafUniqueMin = 0;
				}
				if (secondaryNodeUniqueMin == workGroupSquare + 1)
				{
					secondaryNodeUniqueMin = 0;
				}
				if (secondaryLeafUniqueMin == workGroupSquare + 1)
				{
					secondaryLeafUniqueMin = 0;
				}

				//calculate average
				nodeWorkPerStepAverage[j] /= (float)primaryCount;
				leafWorkPerStepAverage[j] /= (float)primaryCount;
				uniqueNodesPerStepAverage[j] /= (float)primaryNodeUniqueCount;
				uniqueLeafsPerStepAverage[j] /= (float)primaryLeafUniqueCount;
				terminatedRaysStepAverage[j] /= (float)(width / workGroupSize) * (height / workGroupSize);

				secondaryNodeWorkPerStepAverage[j] /= (float)secondaryCount;
				secondaryLeafWorkPerStepAverage[j] /= (float)secondaryCount;
				secondaryUniqueNodesPerStepAverage[j] /= (float)secondaryNodeUniqueCount;
				secondaryUniqueLeafsPerStepAverage[j] /= (float)secondaryLeafUniqueCount;
				secondaryTerminatedRaysStepAverage[j] /= (float)(width / workGroupSize) * (height / workGroupSize);

				//add Terminated rays together:
				if (j != 0)
				{
					terminatedRaysStepAverage[j] += terminatedRaysStepAverage[j - 1];
					secondaryTerminatedRaysStepAverage[j] += secondaryTerminatedRaysStepAverage[j - 1];
				}

				//could also do standard deviation? (mean doesnt really make sense)

				//values are written into file in step order.
				fileWorkGroup << j << ", ";
				fileWorkGroup << nodeWorkPerStepAverage[j] << ", ";
				fileWorkGroup << uniqueNodesPerStepAverage[j] << ", ";
				fileWorkGroup << leafWorkPerStepAverage[j] << ", ";
				fileWorkGroup << uniqueLeafsPerStepAverage[j] << ", ";
				fileWorkGroup << terminatedRaysStepAverage[j] << ", ";
				fileWorkGroup << primaryNodeUniqueMax << ", ";
				fileWorkGroup << primaryNodeUniqueMin << ", ";
				fileWorkGroup << primaryLeafUniqueMax << ", ";
				fileWorkGroup << primaryLeafUniqueMin << ", ";

				fileWorkGroup << secondaryNodeWorkPerStepAverage[j] << ", ";
				fileWorkGroup << secondaryUniqueNodesPerStepAverage[j] << ", ";
				fileWorkGroup << secondaryLeafWorkPerStepAverage[j] << ", ";
				fileWorkGroup << secondaryUniqueLeafsPerStepAverage[j] << ", ";
				fileWorkGroup << secondaryTerminatedRaysStepAverage[j] << ", ";
				fileWorkGroup << secondaryNodeUniqueMax << ", ";
				fileWorkGroup << secondaryNodeUniqueMin << ", ";
				fileWorkGroup << secondaryLeafUniqueMax << ", ";
				fileWorkGroup << secondaryLeafUniqueMin << std::endl;
			}
			fileWorkGroup.close();
		}
		else std::cerr << "Unable to open file for work group analysis" << std::endl;

		//now ammount of unique nodes and leafs loaded per workgroup + average and min max of what would have been loaded without wide
		std::ofstream fileUniqueWork(path + "/" + sizeName + "/" + name + problem + cameraName + "_WorkGroupUniqueWork.txt");
		if (fileUniqueWork.is_open())
		{
			//The values are per workGroup
			fileUniqueWork << "loadedPrimaryNodes, loadedPrimaryLeafs, loadedPrimaryNodesMax, loadedPrimaryLeafsMax, loadedPrimaryNodesMin, loadedPrimaryLeafsMin, ";
			fileUniqueWork << "loadedSecondaryNodes, loadedSecondaryLeafs, loadedSecondaryNodesMax, loadedSecondaryLeafsMax, loadedSecondaryNodesMin, loadedSecondaryLeafsMin, ";
			fileUniqueWork << "loadedWidePrimaryNodes, loadedWidePrimaryLeafs, loadedWideSecondaryNodes, loadedWideSecondaryLeafs" << std::endl;

			for (int i = 0; i < (width / workGroupSize) * (height / workGroupSize); i++)
			{
				float loadedPrimaryNodes = 0;
				float loadedPrimaryLeafs = 0;
				int loadedPrimaryNodesMax = 0;
				int loadedPrimaryLeafsMax = 0;
				int loadedPrimaryNodesMin = 1000;
				int loadedPrimaryLeafsMin = 1000;

				float loadedSecondaryNodes = 0;
				float loadedSecondaryLeafs = 0;
				int loadedSecondaryNodesMax = 0;
				int loadedSecondaryLeafsMax = 0;
				int loadedSecondaryNodesMin = 1000;
				int loadedSecondaryLeafsMin = 1000;

				float loadedWidePrimaryNodes = std::accumulate(uniqueNodesPerStep[i].begin(), uniqueNodesPerStep[i].end(), 0);
				float loadedWidePrimaryLeafs = std::accumulate(uniqueLeafsPerStep[i].begin(), uniqueLeafsPerStep[i].end(), 0);

				float loadedWideSecondaryNodes = std::accumulate(secondaryUniqueNodesPerStep[i].begin(), secondaryUniqueNodesPerStep[i].end(), 0);
				float loadedWideSecondaryLeafs = std::accumulate(secondaryUniqueLeafsPerStep[i].begin(), secondaryUniqueLeafsPerStep[i].end(), 0);

				//loop over single rays for how many node and leafs have been intersected (-> loaded nodes)
				for (int j = 0; j < workGroupSquare; j++)
				{
					int index = (i * workGroupSize) % width + (j % workGroupSize) + (((i * workGroupSize) / width) * workGroupSize + (j / workGroupSize)) * width;
					int realIndex = i * workGroupSquare + j;

					int tmp0 = std::accumulate(nodeIntersectionPerPixelCount[index].begin(), nodeIntersectionPerPixelCount[index].end(), 0);
					tmp0 -= nodeIntersectionPerPixelCountCameraSum[index];
					loadedPrimaryNodes += tmp0;
					loadedPrimaryNodesMax = std::max(tmp0, loadedPrimaryNodesMax);
					loadedPrimaryNodesMin = std::min(tmp0, loadedPrimaryNodesMin);

					int tmp1 = std::accumulate(shadowNodeIntersectionPerPixelCount[index].begin(), shadowNodeIntersectionPerPixelCount[index].end(), 0);
					tmp1 -= shadowNodeIntersectionPerPixelCountCameraSum[index];
					loadedSecondaryNodes += tmp1;
					loadedSecondaryNodesMax = std::max(tmp1, loadedSecondaryNodesMax);
					loadedSecondaryNodesMin = std::min(tmp1, loadedSecondaryNodesMin);

					int tmp2 = std::accumulate(leafIntersectionPerPixelCount[index].begin(), leafIntersectionPerPixelCount[index].end(), 0);
					tmp2 -= leafIntersectionPerPixelCountCameraSum[index];
					loadedPrimaryLeafs += tmp2;
					loadedPrimaryLeafsMax = std::max(tmp2, loadedPrimaryLeafsMax);
					loadedPrimaryLeafsMin = std::min(tmp2, loadedPrimaryLeafsMin);

					int tmp3 = std::accumulate(shadowLeafIntersectionPerPixelCount[index].begin(), shadowLeafIntersectionPerPixelCount[index].end(), 0);
					tmp3 -= shadowLeafIntersectionPerPixelCountCameraSum[index];
					loadedSecondaryLeafs += tmp3;
					loadedSecondaryLeafsMax = std::max(tmp3, loadedSecondaryLeafsMax);
					loadedSecondaryLeafsMin = std::min(tmp3, loadedSecondaryLeafsMin);

					nodeIntersectionPerPixelCountCameraSum[index] += tmp0;
					shadowNodeIntersectionPerPixelCountCameraSum[index] += tmp1;
					leafIntersectionPerPixelCountCameraSum[index] += tmp2;
					shadowLeafIntersectionPerPixelCountCameraSum[index] += tmp3;
				}

				//set the min values that didnt have any values to 0
				if (loadedPrimaryNodesMin == 1000)
				{
					loadedPrimaryNodesMin = 0;
				}
				if (loadedPrimaryLeafsMin == 1000)
				{
					loadedPrimaryLeafsMin = 0;
				}
				if (loadedSecondaryNodesMin == 1000)
				{
					loadedSecondaryNodesMin = 0;
				}
				if (loadedSecondaryLeafsMin == 1000)
				{
					loadedSecondaryLeafsMin = 0;
				}

				/*
				loadedPrimaryNodes /= (float)workGroupSquare;
				loadedPrimaryLeafs /= (float)workGroupSquare;
				loadedSecondaryNodes /= (float)workGroupSquare;
				loadedSecondaryLeafs /= (float)workGroupSquare;

				loadedWidePrimaryNodes /= (float)workGroupSquare;
				loadedWidePrimaryLeafs /= (float)workGroupSquare;
				loadedWideSecondaryNodes /= (float)workGroupSquare;
				loadedWideSecondaryLeafs /= (float)workGroupSquare;
				*/

				fileUniqueWork << loadedPrimaryNodes << ", ";
				fileUniqueWork << loadedPrimaryLeafs << ", ";
				fileUniqueWork << loadedPrimaryNodesMax << ", ";
				fileUniqueWork << loadedPrimaryLeafsMax << ", ";
				fileUniqueWork << loadedPrimaryNodesMin << ", ";
				fileUniqueWork << loadedPrimaryLeafsMin << ", ";

				fileUniqueWork << loadedSecondaryNodes << ", ";
				fileUniqueWork << loadedSecondaryLeafs << ", ";
				fileUniqueWork << loadedSecondaryNodesMax << ", ";
				fileUniqueWork << loadedSecondaryLeafsMax << ", ";
				fileUniqueWork << loadedSecondaryNodesMin << ", ";
				fileUniqueWork << loadedSecondaryLeafsMin << ", ";

				fileUniqueWork << loadedWidePrimaryNodes << ", ";
				fileUniqueWork << loadedWidePrimaryLeafs << ", ";
				fileUniqueWork << loadedWideSecondaryNodes << ", ";
				fileUniqueWork << loadedWideSecondaryLeafs << std::endl;
			}

			fileUniqueWork.close();
		}
	}

	void initializeVariables()
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
		shadowLeafIntersectionCount = 0;
		shadowNodeIntersectionCount = 0;

		if (wideRender)
		{
			nodeIntersectionPerPixelCountCameraSum.resize(height * width);
			shadowNodeIntersectionPerPixelCountCameraSum.resize(height * width);
			leafIntersectionPerPixelCountCameraSum.resize(height * width);
			shadowLeafIntersectionPerPixelCountCameraSum.resize(height * width);
		}

		//wide render data is initialized and reset in camrea loop since its only needed inside loop
	}

	void createDepthDebugImage(int maxDepth)
	{
		//save an image with all the aabb intersections for every depth
		for (size_t d = 0; d < maxDepth; d++)
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
	}

	void initializeWideRenderVariables()
	{
		//reset and initialize the vectors.
		int size = (width / nonTemplateWorkGroupSize) * (height / nonTemplateWorkGroupSize);

		//reset if they already exist
		if (!nodeWorkPerStep.empty())
		{
			nodeWorkPerStep.clear();
			leafWorkPerStep.clear();
			terminationsPerStep.clear();
			uniqueNodesPerStep.clear();
			uniqueLeafsPerStep.clear();

			secondaryNodeWorkPerStep.clear();
			secondaryLeafWorkPerStep.clear();
			secondaryTerminationsPerStep.clear();
			secondaryUniqueNodesPerStep.clear();
			secondaryUniqueLeafsPerStep.clear();
		}
		nodeWorkPerStep.resize(size);
		leafWorkPerStep.resize(size);
		terminationsPerStep.resize(size);
		uniqueNodesPerStep.resize(size);
		uniqueLeafsPerStep.resize(size);

		secondaryNodeWorkPerStep.resize(size);
		secondaryLeafWorkPerStep.resize(size);
		secondaryTerminationsPerStep.resize(size);
		secondaryUniqueNodesPerStep.resize(size);
		secondaryUniqueLeafsPerStep.resize(size);
	}
};