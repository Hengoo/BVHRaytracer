#pragma once
#include <iostream>
// writing on a text file
#include <fstream>

#include <algorithm>
#include <vector>

//for pi
#define _USE_MATH_DEFINES
#include <math.h>

//for the parallel for
#include <execution>

#include "lights/light.h"
#include "glmInclude.h"
#include "util.h"

struct RenderInfo
{
	float w;
	float h;
	size_t index;

	RenderInfo(float w, float h, size_t index) : w(w), h(h), index(index)
	{
	}
	RenderInfo() : w(0), h(0), index(0)
	{
	}
};

//spawns all the rays in the scene
class Camera
{
public:
	//path to write the analysis results
	std::string path;
	//model / scenario name
	std::string name;
	//description for info.txt
	std::string problem;
	glm::vec3 position;
	glm::mat4 transform;
	std::vector<uint8_t> image;

	//contains all info needed to spawn the ray for the specific pixel. Only needed because i dont know how to get the loop index into the unsequenced for_each
	std::vector<RenderInfo> renderInfos;

	size_t height;
	size_t width;

	//1.37f for 40 degree, 0.866f for 60 degree (horizontal fov)
	//This focallength has most likely nothing to do with real life focal length
	float focalLength;

	//(most likely not needed anyway)
	//float farPlane = 50;

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

	Camera(std::string path, std::string name, std::string problem, glm::vec3 position, glm::vec3 lookCenter
		, glm::vec3 upward = glm::vec3(0, 1, 0), float focalLength = 0.866f, size_t height = 1080, size_t width = 1920)
		: path(path), name(name), problem(problem), position(position), focalLength(focalLength), height(height), width(width)
	{
		transform = glm::inverse(glm::lookAt(position, lookCenter, upward));

		initializeVariables();
	}

	Camera(std::string path, std::string name, std::string problem, glm::mat4 transform,
		float focalLength = 0.866f, size_t height = 1080, size_t width = 1920)
		: path(path), name(name), problem(problem), transform(transform), focalLength(focalLength), height(height), width(width)
	{
		position = transform * glm::vec4(0, 0, 0, 1);

		initializeVariables();
	}

	//spawns rays and collects results into image. Image is written on disk
	template<typename T>
	void renderImage(bool saveImage, bool saveDepthDebugImage, CompactNodeManager<T>& nodeManager
		, Bvh& bvh, std::vector<std::unique_ptr<Light>>& lights, unsigned ambientSampleCount, bool castShadows, int renderType, bool mute)
	{
		glm::vec3 decScale;
		glm::quat decOrientation;
		glm::vec3 decTranslation;
		glm::vec3 decSkew;
		glm::vec4 decPerspective;
		glm::decompose(transform, decScale, decOrientation, decTranslation, decSkew, decPerspective);

		//simplified ortho version for now:

		//fill RenderInfo array.
		for (int i = 0; i < width * height; i++)
		{
			//todo: move to some method for sampling(and make RenderInfo save more than one w,h combination per pixel)?
			float w = (i % width - width / 2.f);
			float h = -(i / width - height / 2.f);

			renderInfos[i] = RenderInfo(w, h, i);
		}

		//array for 

		std::for_each(std::execution::par_unseq, renderInfos.begin(), renderInfos.end(),
			[&](auto& info)
			{
				//std::cout << "test" << std::endl;

				//orthographic camera, dont think i will use it anytime soon
				//glm::vec4 centerOffset = (glm::vec4(0, 1, 0, 0) * (float)info.h + glm::vec4(1, 0, 0, 0) * (float)info.w) * (1.0f / 100);
				//glm::vec3 pos = position + glm::vec3(transform * centerOffset);
				//auto forward = glm::vec3(transform * glm::vec4(0, 0, -1, 0) );
				//auto ray = Ray(pos, forward);


				//glm uses x = right, y = up , -z = forward ...

				glm::vec4 centerOffset = (glm::vec4(0, 1, 0, 0) * (float)info.h + glm::vec4(1, 0, 0, 0) * (float)info.w) * (1.0f / width) + glm::vec4(0, 0, -focalLength, 0);
				//next line to get perfect forward ray
				//centerOffset = glm::vec4(0, 0, -1, 0);
				glm::vec3 pos = position + glm::vec3(transform * centerOffset);

				auto ray = Ray(position, pos - position, bvh);

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
						//i need two deterministic random values. Currently pixel index but could also be intersect position
						auto test = std::hash<size_t>();
						float u = test(info.index * 37 + i * 13) / (float)(std::numeric_limits<size_t>::max());
						float v = test(info.index * 61 + i * 7) / (float)(std::numeric_limits<size_t>::max());

						auto direction = sampleHemisphere(u, v, ray.surfaceNormal);
						Ray shadowRay(ray.surfacePosition + direction * 0.001f, direction, bvh, true);


						if (shootShadowRay(shadowRay, ray, info, bvh, nodeManager, renderType))
						{
							ambientResult++;
						}
					}
					if (ambientSampleCount != 0)
					{
						float factor = 1 - ambientResult / (float)ambientSampleCount;
						factor = (factor + 1) / 2.f;
						ray.surfaceColor.scale(factor);
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
						Ray shadowRay(ray.surfacePosition + lightVector * 0.001f, lightVector, bvh, true);
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

				//copy vectors
				nodeIntersectionPerPixelCount[info.index] = ray.nodeIntersectionCount;
				leafIntersectionPerPixelCount[info.index] = ray.leafIntersectionCount;
				childFullnessPerPixelCount[info.index] = ray.childFullness;
				primitiveFullnessPerPixelCount[info.index] = ray.primitiveFullness;

				primitiveIntersectionsPerPixel[info.index] += ray.primitiveIntersectionCount;
				successfulPrimitiveIntersectionsPerPixel[info.index] += ray.successfulPrimitiveIntersectionCount;
				successfulAabbIntersectionsPerPixel[info.index] += ray.successfulAabbIntersectionCount;
				aabbIntersectionsPerPixel[info.index] += ray.aabbIntersectionCount;
			});

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

		//normalize by pixel
		float factor = 1 / (float)(width * height);
		//shadowfactor for now the same. Could normalize it per shadowrays but that would ruin the scale and fullness
		float shadowFactor = 1 / (float)shadowRayCount;
		float bothFactor = 1 / (float)(width * height + shadowRayCount);

		if (!mute)
		{
			std::cout << "intersections counts are normalized per Ray" << std::endl << std::endl;
			std::cout << "node intersections: " << nodeIntersectionCount * factor << std::endl;
			std::cout << "aabb intersections: " << aabbIntersections * factor << std::endl;
			std::cout << "aabb success ration: " << successfulAabbIntersections / (float)aabbIntersections << std::endl;
			//wastefactor = "verschwendungsgrad".
			// basically number of nodes visited / number of aabb tested
			//float wasteFactor = (leafIntersectionCount + nodeIntersectionCount * bvh.branchingFactor) / (float)aabbIntersections;
			//the minus width*height is basically -1 for each ray -> needed to get right values
			float wasteFactor = (leafIntersectionCount + nodeIntersectionCount - width * height) / (float)(nodeIntersectionCount * bvh.branchingFactor);
			std::cout << "waste factor: " << 1 - wasteFactor << std::endl;
			std::cout << "leaf intersections: " << leafIntersectionCount * factor << std::endl;
			std::cout << "primitive intersections: " << primitiveIntersections * factor << std::endl;
			std::cout << "primitive success ratio: " << successfulPrimitiveIntersections / (float)primitiveIntersections << std::endl;
			std::cout << std::endl;
			std::cout << "shadow node intersections: " << shadowNodeIntersectionCount * shadowFactor << std::endl;
			std::cout << "shadow aabb intersections: " << shadowAabbIntersections * shadowFactor << std::endl;
			std::cout << "shadow aabb success ration: " << shadowSuccessfulAabbIntersections / (float)shadowAabbIntersections << std::endl;
			wasteFactor = (shadowLeafIntersectionCount + shadowNodeIntersectionCount - shadowRayCount) / (float)(shadowNodeIntersectionCount * bvh.branchingFactor);
			std::cout << "shadow waste factor: " << 1 - wasteFactor << std::endl;
			std::cout << "shadow leaf intersections: " << shadowLeafIntersectionCount * shadowFactor << std::endl;
			std::cout << "shadow primitive intersections: " << shadowPrimitiveIntersections * shadowFactor << std::endl;
			std::cout << "shadow primitive success ratio: " << shadowSuccessfulPrimitiveIntersections / (float)shadowPrimitiveIntersections << std::endl;
		}
		std::ofstream myfile(path + "/" + name + problem + "_Info.txt");
		if (myfile.is_open())
		{
			myfile << "scenario " << name << " with branching factor of " << std::to_string(bvh.branchingFactor) << " and leafsize of " << bvh.leafSize << std::endl;
			myfile << "intersections counts are normalized per Ray" << std::endl << std::endl;

			myfile << "node intersections: " << nodeIntersectionCount * factor << std::endl;
			myfile << "aabb intersections: " << aabbIntersections * factor << std::endl;
			myfile << "aabb success ration: " << successfulAabbIntersections / (float)aabbIntersections << std::endl;
			float wasteFactor = (leafIntersectionCount + nodeIntersectionCount - width * height) / (float)(nodeIntersectionCount * bvh.branchingFactor);
			myfile << "waste factor: " << 1 - wasteFactor << std::endl;
			myfile << "leaf intersections: " << leafIntersectionCount * factor << std::endl;
			myfile << "primitive intersections: " << primitiveIntersections * factor << std::endl;
			myfile << "primitive success ratio: " << successfulPrimitiveIntersections / (float)primitiveIntersections << std::endl;
			myfile << std::endl;
			myfile << "shadow node intersections: " << shadowNodeIntersectionCount * shadowFactor << std::endl;
			myfile << "shadow aabb intersections: " << shadowAabbIntersections * shadowFactor << std::endl;
			myfile << "shadow aabb success ration: " << shadowSuccessfulAabbIntersections / (float)shadowAabbIntersections << std::endl;
			wasteFactor = (shadowLeafIntersectionCount + shadowNodeIntersectionCount - shadowRayCount) / (float)(shadowNodeIntersectionCount * bvh.branchingFactor);
			myfile << "shadow waste factor: " << 1 - wasteFactor << std::endl;
			myfile << "shadow leaf intersections: " << shadowLeafIntersectionCount * shadowFactor << std::endl;
			myfile << "shadow primitive intersections: " << shadowPrimitiveIntersections * shadowFactor << std::endl;
			myfile << "shadow primitive success ratio: " << shadowSuccessfulPrimitiveIntersections / (float)shadowPrimitiveIntersections << std::endl;

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
			myfile << "averag leaf fullness: " << std::to_string(sum) << std::endl;
			for (size_t i = 0; i < primitiveFullness.size(); i++)
			{
				//myfile << i << " : " << primitiveFullness[i] * bothFactor << std::endl;
				myfile << i << " : " << primitiveFullness[i] * factor << std::endl;
			}

			myfile << std::endl;
			myfile << "primaryRay node intersections at depth x :" << std::endl;
			for (size_t i = 0; i < nodeIntersectionPerDepthCount.size(); i++)
			{
				myfile << i << " : " << nodeIntersectionPerDepthCount[i] * factor << std::endl;
			}
			myfile << std::endl;
			myfile << "primaryRay leaf intersections at depth x :" << std::endl;
			for (size_t i = 0; i < leafIntersectionPerDepthCount.size(); i++)
			{
				myfile << i << " : " << leafIntersectionPerDepthCount[i] * factor << std::endl;
			}
			myfile << std::endl;
			myfile << "shadowRay node intersections at depth x :" << std::endl;
			for (size_t i = 0; i < shadowNodeIntersectionPerDepthCount.size(); i++)
			{
				myfile << i << " : " << shadowNodeIntersectionPerDepthCount[i] * shadowFactor << std::endl;
			}
			myfile << std::endl;
			myfile << "shadowRay leaf intersections at depth x :" << std::endl;
			for (size_t i = 0; i < shadowLeafIntersectionPerDepthCount.size(); i++)
			{
				myfile << i << " : " << shadowLeafIntersectionPerDepthCount[i] * shadowFactor << std::endl;
			}
			if (!saveDepthDebugImage)
			{
				myfile.close();
			}
		}
		else std::cerr << "Unable to open file" << std::endl;


		if (saveImage)
		{
			encodeTwoSteps(path + "/" + name + ".png", image, width, height);
		}
		if (saveDepthDebugImage)
		{
			myfile << std::endl;
			myfile << " max node intersections at depth x :" << std::endl;
			//save an image with all the aabb intersections for every depth
			for (size_t d = 0; d < nodeIntersectionPerDepthCount.size(); d++)
			{
				uint16_t depthIntersections = 0;
				//find max element first to normalise to;
				std::for_each(std::execution::seq, renderInfos.begin(), renderInfos.end(),
					[&](auto& info)
					{
						if (d < nodeIntersectionPerPixelCount[info.index].size())
						{
							depthIntersections = std::max(nodeIntersectionPerPixelCount[info.index][d], depthIntersections);
						}
					});

				myfile << d << " : " << depthIntersections << std::endl;

				//go trough RenderInfo vector and use the stored nodeIntersectionPerPixelCount
				std::for_each(std::execution::par_unseq, renderInfos.begin(), renderInfos.end(),
					[&](auto& info)
					{
						uint16_t sum = 0;
						if (d < nodeIntersectionPerPixelCount[info.index].size())
						{
							sum = nodeIntersectionPerPixelCount[info.index][d];
						}

						//Color c(sum * 0.01f);
						Color c(sum * (1 / (float)depthIntersections));
						image[info.index * 4 + 0] = (uint8_t)(c.r * 255);
						image[info.index * 4 + 1] = (uint8_t)(c.g * 255);
						image[info.index * 4 + 2] = (uint8_t)(c.b * 255);
						image[info.index * 4 + 3] = (uint8_t)(c.a * 255);
					});
				encodeTwoSteps(path + "/" + name + problem + "_NodeDepth" + std::to_string(d) + ".png", image, width, height);
			}

			unsigned maxNodeSum = 0;
			unsigned maxLeafSum = 0;
			//find min and max for nodeintersection and leafintersection: (and number of different depth intersections
			std::for_each(std::execution::seq, renderInfos.begin(), renderInfos.end(),
				[&](auto& info)
				{
					unsigned sum = std::accumulate(nodeIntersectionPerPixelCount[info.index].begin(), nodeIntersectionPerPixelCount[info.index].end(), 0);
					maxNodeSum = std::max(sum, maxNodeSum);
					sum = std::accumulate(leafIntersectionPerPixelCount[info.index].begin(), leafIntersectionPerPixelCount[info.index].end(), 0);
					maxLeafSum = std::max(sum, maxLeafSum);
				});
			float normalisation = 1 / (float)maxNodeSum;
			//pixel bvh depth
			//std::cout << normalisation << std::endl;
			//std::cout << minSum << std::endl;
			//std::cout << maxSum << std::endl;
			std::for_each(std::execution::par_unseq, renderInfos.begin(), renderInfos.end(),
				[&](auto& info)
				{
					unsigned sum = std::accumulate(nodeIntersectionPerPixelCount[info.index].begin(), nodeIntersectionPerPixelCount[info.index].end(), 0);
					Color c(sum * normalisation);
					image[info.index * 4 + 0] = (uint8_t)(c.r * 255);
					image[info.index * 4 + 1] = (uint8_t)(c.g * 255);
					image[info.index * 4 + 2] = (uint8_t)(c.b * 255);
					image[info.index * 4 + 3] = (uint8_t)(c.a * 255);
				});
			encodeTwoSteps(path + "/" + name + problem + "_NodeIntersectionCount.png", image, width, height);

			myfile << std::endl;
			myfile << "max leaf intersections :" << maxLeafSum << std::endl;
			normalisation = 1.0 / maxLeafSum;
			//std::cout << maxLeafSum << std::endl;
			std::for_each(std::execution::par_unseq, renderInfos.begin(), renderInfos.end(),
				[&](auto& info)
				{
					unsigned sum = std::accumulate(leafIntersectionPerPixelCount[info.index].begin(), leafIntersectionPerPixelCount[info.index].end(), 0);
					Color c(sum * normalisation);
					image[info.index * 4 + 0] = (uint8_t)(c.r * 255);
					image[info.index * 4 + 1] = (uint8_t)(c.g * 255);
					image[info.index * 4 + 2] = (uint8_t)(c.b * 255);
					image[info.index * 4 + 3] = (uint8_t)(c.a * 255);
				});
			encodeTwoSteps(path + "/" + name + problem + "_LeafIntersectionCount.png", image, width, height);
			myfile.close();
		}
	}




private:
	template<typename T>
	bool shootShadowRay(Ray& shadowRay, Ray& ray, RenderInfo& info, Bvh& bvh, CompactNodeManager<T>& nodeManager, int& renderType)
	{
		bool result;
		shadowRayCounter[info.index] ++;
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

		if (shadowNodeIntersectionPerPixelCount[info.index].size() < shadowRay.nodeIntersectionCount.size())
		{
			shadowNodeIntersectionPerPixelCount[info.index].resize(shadowRay.nodeIntersectionCount.size());
		}
		for (size_t i = 0; i < shadowRay.nodeIntersectionCount.size(); i++)
		{
			shadowNodeIntersectionPerPixelCount[info.index][i] += shadowRay.nodeIntersectionCount[i];
		}

		if (shadowLeafIntersectionPerPixelCount[info.index].size() < shadowRay.leafIntersectionCount.size())
		{
			shadowLeafIntersectionPerPixelCount[info.index].resize(shadowRay.leafIntersectionCount.size());
		}
		for (size_t i = 0; i < shadowRay.leafIntersectionCount.size(); i++)
		{
			shadowLeafIntersectionPerPixelCount[info.index][i] += shadowRay.leafIntersectionCount[i];
		}

		if (ray.childFullness.size() < shadowRay.childFullness.size())
		{
			ray.childFullness.resize(shadowRay.childFullness.size());
		}
		for (size_t i = 0; i < shadowRay.childFullness.size(); i++)
		{
			//ray.childFullness[i] += shadowRay.childFullness[i];
		}

		if (ray.primitiveFullness.size() < shadowRay.primitiveFullness.size())
		{
			ray.primitiveFullness.resize(shadowRay.primitiveFullness.size());
		}
		for (size_t i = 0; i < shadowRay.primitiveFullness.size(); i++)
		{
			//ray.primitiveFullness[i] += shadowRay.primitiveFullness[i];
		}

		shadowPrimitiveIntersectionsPerPixel[info.index] += shadowRay.primitiveIntersectionCount;
		if (shadowRay.successfulPrimitiveIntersectionCount > 1)
		{
			std::cerr << "error: more than 1 successful shadowray primitive intersection for one light" << std::endl;
		}
		shadowSuccessfulPrimitiveIntersectionsPerPixel[info.index] += shadowRay.successfulPrimitiveIntersectionCount;
		shadowSuccessfulAabbIntersectionsPerPixel[info.index] += shadowRay.successfulAabbIntersectionCount;
		shadowAabbIntersectionsPerPixel[info.index] += shadowRay.aabbIntersectionCount;
		return result;
	}

	glm::vec3 sampleHemisphere(float u, float v, glm::vec3 normal, int m = 1)
	{
		//https://blog.thomaspoulet.fr/uniform-sampling-on-unit-hemisphere/
		float theta = acos(pow(1 - u, 1 / (float)(1 + m)));
		float phi = 2 * M_PI * v;

		float x = sin(theta) * cos(phi);
		float y = sin(theta) * sin(phi);
		float z = -cos(theta);

		//i dont like this approach but for now it has to do
		glm::vec4 tmp(x, y, z, 0);
		glm::vec3 up(0);
		if (normal.y != 1)
		{
			up.y = 1;
		}
		else
		{
			up.x = 1;
		}
		auto matrix = glm::lookAt(glm::vec3(0), normal, up);

		auto res2 = tmp * matrix;
		return glm::vec3(res2);
	}

	void initializeVariables()
	{
		image.resize(height * width * 4);
		renderInfos.resize(height * width);
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
	}
};