#pragma once
#include <iostream>
// writing on a text file
#include <fstream>

#include <algorithm>
#include <vector>


//for the parallel for
#include <execution>

#include "glmInclude.h"
#include "global.h"
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
	std::vector<unsigned char> image;

	//contains all info needed to spawn the ray for the specific pixel. Only needed because i dont know how to get the loop index into the unsequenced for_each
	std::vector<RenderInfo> renderInfos;

	size_t height;
	size_t width;

	//1.37f for 40 degree, 0.866f for 60 degree (horizontal fov)
	//This focallength has most likely nothing to do with real life focal length
	float focalLength;

	//todo not implemented (most likely not needed anyway)
	float farPlane = 50;

	//per pixel per depth counter:
	std::vector<std::vector<unsigned int>> nodeIntersectionPerPixelCount;
	std::vector<std::vector<unsigned int>> leafIntersectionPerPixelCount;
	std::vector<std::vector<unsigned int>> shadowNodeIntersectionPerPixelCount;
	std::vector<std::vector<unsigned int>> shadowLeafIntersectionPerPixelCount;
	std::vector<std::vector<unsigned int>> childFullnessPerPixelCount;
	std::vector<std::vector<unsigned int>> primitiveFullnessPerPixelCount;

	//per pixel counter:
	std::vector<size_t> primitiveIntersectionsPerPixel;
	std::vector<size_t> shadowPrimitiveIntersectionsPerPixel;
	std::vector<size_t> successfulPrimitiveIntersectionsPerPixel;
	std::vector<size_t> successfulAabbIntersectionsPerPixel;
	std::vector<size_t> aabbIntersectionsPerPixel;
	std::vector<size_t> shadowRayCounter;
	std::vector<size_t> shadowSuccessfulPrimitiveIntersectionsPerPixel;
	std::vector<size_t> shadowSuccessfulAabbIntersectionsPerPixel;
	std::vector<size_t> shadowAabbIntersectionsPerPixel;

	//per depth counter:
	std::vector<size_t> primitiveFullness;
	std::vector<size_t> childFullness;
	std::vector<size_t> nodeIntersectionPerDepthCount;
	std::vector<size_t> leafIntersectionPerDepthCount;
	std::vector<size_t> shadowNodeIntersectionPerDepthCount;
	std::vector<size_t> shadowLeafIntersectionPerDepthCount;

	//normal counter
	size_t nodeIntersectionCount;
	size_t leafIntersectionCount;
	size_t shadowNodeIntersectionCount;
	size_t shadowLeafIntersectionCount;
	size_t primitiveIntersections;
	size_t shadowPrimitiveIntersections;
	size_t successfulPrimitiveIntersections;
	size_t successfulAabbIntersections;
	size_t aabbIntersections;
	size_t shadowRayCount;
	size_t shadowSuccessfulPrimitiveIntersections;
	size_t shadowSuccessfulAabbIntersections;
	size_t shadowAabbIntersections;

	Camera(std::string path, std::string name, std::string problem, glm::vec3 position, glm::vec3 lookCenter, glm::vec3 upward = glm::vec3(0, 1, 0), float focalLength = 0.866f, size_t height = 1080, size_t width = 1920)
		: path(path), name(name), problem(problem), position(position), focalLength(focalLength), height(height), width(width)
	{
		transform = glm::inverse(glm::lookAt(position, lookCenter, upward));

		initializeVariables();
	}

	Camera(std::string path, std::string name, std::string problem, glm::mat4 transform, float focalLength = 0.866f, size_t height = 1000, size_t width = 1000)
		: path(path), name(name), problem(problem), transform(transform), focalLength(focalLength), height(height), width(width)
	{
		position = transform * glm::vec4(0, 0, 0, 1);

		initializeVariables();
	}

	//spawns rays and collects results into image. Image is written on disk
	void renderImage(bool saveImage, bool saveDepthDebugImage)
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
				//orthographic camera, dont think i will use it anytime soon
				//glm::vec4 centerOffset = (glm::vec4(0, 1, 0, 0) * (float)info.h + glm::vec4(1, 0, 0, 0) * (float)info.w) * (1.0f / 100);
				//glm::vec3 pos = position + glm::vec3(transform * centerOffset);
				//auto forward = glm::vec3(transform * glm::vec4(0, 0, -1, 0) );
				//auto ray = Ray(pos, forward);


				//glm uses x = right, y = up , -z = forward ...

				glm::vec4 centerOffset = (glm::vec4(0, 1, 0, 0) * (float)info.h + glm::vec4(1, 0, 0, 0) * (float)info.w) * (1.0f / width) + glm::vec4(0, 0, -focalLength, 0);
				glm::vec3 pos = position + glm::vec3(transform * centerOffset);
				auto ray = Ray(position, pos - position);

				auto result = bvh.intersect(ray);

				//check shadows if ray hit something
				if (result)
				{
					//resolve shadows, ez:
					for (auto& l : lights)
					{
						float lightDistance;
						glm::vec3 lightVector;

						//todo use light color
						auto lightColor = l->getLightDirection(ray.surfacePosition, lightVector, lightDistance);

						float f = glm::dot(ray.surfaceNormal, lightVector);
						//add bias to vector to prevent shadow rays hitting the surface they where created for
						Ray shadowRay(ray.surfacePosition + lightVector * 0.001f, lightVector, true);
						shadowRay.tMax = lightDistance;

						//only shoot ray when surface points in light direction
						if (f > 0)
						{
							shadowRayCounter[info.index] ++;
							if (bvh.intersect(shadowRay))
							{
								f = 0;
							}

							//add shadowRay intersection to this ones
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
								ray.childFullness[i] += shadowRay.childFullness[i];
							}

							if (ray.primitiveFullness.size() < shadowRay.primitiveFullness.size())
							{
								ray.primitiveFullness.resize(shadowRay.primitiveFullness.size());
							}
							for (size_t i = 0; i < shadowRay.primitiveFullness.size(); i++)
							{
								ray.primitiveFullness[i] += shadowRay.primitiveFullness[i];
							}

							shadowPrimitiveIntersectionsPerPixel[info.index] += shadowRay.primitiveIntersectionCount;
							if (shadowRay.successfulPrimitiveIntersectionCount > 1)
							{
								std::cout << "error: more than 1 successful shadowray primitive intersection for one light" << std::endl;
							}
							shadowSuccessfulPrimitiveIntersectionsPerPixel[info.index] += shadowRay.successfulPrimitiveIntersectionCount;
							shadowSuccessfulAabbIntersectionsPerPixel[info.index] += shadowRay.successfulAabbIntersectionCount;
							shadowAabbIntersectionsPerPixel[info.index] += shadowRay.aabbIntersectionCount;

						}

						f = std::max(0.2f, f);
						ray.surfaceColor.scale(f);
					}

				}

				image[info.index * 4 + 0] = (unsigned char)(ray.surfaceColor.r * 255);
				image[info.index * 4 + 1] = (unsigned char)(ray.surfaceColor.g * 255);
				image[info.index * 4 + 2] = (unsigned char)(ray.surfaceColor.b * 255);
				image[info.index * 4 + 3] = (unsigned char)(ray.surfaceColor.a * 255);

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

		for (auto& perPixel : leafIntersectionPerPixelCount)
		{
			for (size_t i = 0; i < perPixel.size(); i++)
			{
				if (leafIntersectionPerDepthCount.size() < perPixel.size())
				{
					leafIntersectionPerDepthCount.resize(perPixel.size());
				}
				leafIntersectionPerDepthCount[i] += perPixel[i];
			}
		}

		for (auto& perPixel : nodeIntersectionPerPixelCount)
		{
			for (size_t i = 0; i < perPixel.size(); i++)
			{
				if (nodeIntersectionPerDepthCount.size() < perPixel.size())
				{
					nodeIntersectionPerDepthCount.resize(perPixel.size());
				}
				nodeIntersectionPerDepthCount[i] += perPixel[i];
			}
		}

		for (auto& perPixel : shadowLeafIntersectionPerPixelCount)
		{
			for (size_t i = 0; i < perPixel.size(); i++)
			{
				if (shadowLeafIntersectionPerDepthCount.size() < perPixel.size())
				{
					shadowLeafIntersectionPerDepthCount.resize(perPixel.size());
				}
				shadowLeafIntersectionPerDepthCount[i] += perPixel[i];
			}
		}

		for (auto& perPixel : shadowNodeIntersectionPerPixelCount)
		{
			for (size_t i = 0; i < perPixel.size(); i++)
			{
				if (shadowNodeIntersectionPerDepthCount.size() < perPixel.size())
				{
					shadowNodeIntersectionPerDepthCount.resize(perPixel.size());
				}
				shadowNodeIntersectionPerDepthCount[i] += perPixel[i];
			}
		}

		for (auto& perPixel : childFullnessPerPixelCount)
		{
			for (size_t i = 0; i < perPixel.size(); i++)
			{
				if (childFullness.size() < perPixel.size())
				{
					childFullness.resize(perPixel.size());
				}
				childFullness[i] += perPixel[i];
			}
		}

		for (auto& perPixel : primitiveFullnessPerPixelCount)
		{
			for (size_t i = 0; i < perPixel.size(); i++)
			{
				if (primitiveFullness.size() < perPixel.size())
				{
					primitiveFullness.resize(perPixel.size());
				}
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
		std::cout << "intersections counts are normalized per Ray" << std::endl << std::endl;
		std::cout << "node intersections: " << nodeIntersectionCount * factor << std::endl;
		std::cout << "aabb intersections: " << aabbIntersections * factor << std::endl;
		std::cout << "aabb success ration: " << successfulAabbIntersections / (float)aabbIntersections << std::endl;
		std::cout << "leaf intersections: " << leafIntersectionCount * factor << std::endl;
		std::cout << "primitive intersections: " << primitiveIntersections * factor << std::endl;
		std::cout << "primitive success ratio: " << successfulPrimitiveIntersections / (float)primitiveIntersections << std::endl;
		std::cout << std::endl;
		std::cout << "shadow node intersections: " << shadowNodeIntersectionCount * shadowFactor << std::endl;
		std::cout << "shadow aabb intersections: " << shadowAabbIntersections * shadowFactor << std::endl;
		std::cout << "shadow aabb success ration: " << shadowSuccessfulAabbIntersections / (float)shadowAabbIntersections << std::endl;
		std::cout << "shadow leaf intersections: " << shadowLeafIntersectionCount * shadowFactor << std::endl;
		std::cout << "shadow primitive intersections: " << shadowPrimitiveIntersections * shadowFactor << std::endl;
		std::cout << "shadow primitive success ratio: " << shadowSuccessfulPrimitiveIntersections / (float)shadowPrimitiveIntersections << std::endl;

		std::ofstream myfile(path + "/" + name + problem + "_Info.txt");
		if (myfile.is_open())
		{
			myfile << "scenario " << name << " with branching factor of " << std::to_string(bvh.branchingFactor) << " and leafsize of " << bvh.leafCount << std::endl;
			myfile << "intersections counts are normalized per Ray" << std::endl << std::endl;

			myfile << "node intersections: " << nodeIntersectionCount * factor << std::endl;
			myfile << "aabb intersections: " << aabbIntersections * factor << std::endl;
			myfile << "aabb success ration: " << successfulAabbIntersections / (float)aabbIntersections << std::endl;
			myfile << "leaf intersections: " << leafIntersectionCount * factor << std::endl;
			myfile << "primitive intersections: " << primitiveIntersections * factor << std::endl;
			myfile << "primitive success ratio: " << successfulPrimitiveIntersections / (float)primitiveIntersections << std::endl;
			myfile << std::endl;
			myfile << "shadow node intersections: " << shadowNodeIntersectionCount * shadowFactor << std::endl;
			myfile << "shadow aabb intersections: " << shadowAabbIntersections * shadowFactor << std::endl;
			myfile << "shadow aabb success ration: " << shadowSuccessfulAabbIntersections / (float)shadowAabbIntersections << std::endl;
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
			myfile << "average: : " << std::to_string(sum) << std::endl;
			for (size_t i = 0; i < childFullness.size(); i++)
			{
				myfile << i << " : " << childFullness[i] * bothFactor << std::endl;
			}

			myfile << std::endl;
			myfile << "intersections with leaf nodes with x primitives :" << std::endl;
			for (size_t i = 0; i < primitiveFullness.size(); i++)
			{
				sum += primitiveFullness[i] * i;
			}
			sum /= std::accumulate(primitiveFullness.begin(), primitiveFullness.end(), 0);
			myfile << "average: : " << std::to_string(sum) << std::endl;
			for (size_t i = 0; i < primitiveFullness.size(); i++)
			{
				myfile << i << " : " << primitiveFullness[i] * bothFactor << std::endl;
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
			myfile.close();
		}
		else std::cout << "Unable to open file" << std::endl;


		if (saveImage)
		{
			encodeTwoSteps(path + "/" + name + ".png", image, width, height);
		}
		if (saveDepthDebugImage)
		{
			//save an image with all the aabb intersections for every depth
			for (size_t d = 0; d < nodeIntersectionPerDepthCount.size(); d++)
			{
				std::vector<unsigned int> maxDepth;
				//find max element first to normalise to;
				std::for_each(std::execution::seq, renderInfos.begin(), renderInfos.end(),
					[&](auto& info)
					{
						if (d < nodeIntersectionPerPixelCount[info.index].size())
						{
							if (maxDepth.size() < d + 1)
							{
								maxDepth.resize(d + 1);
							}
							maxDepth[d] = std::max(nodeIntersectionPerPixelCount[info.index][d], maxDepth[d]);
						}
					});
				//go trough RenderInfo vector and use the stored nodeIntersectionPerPixelCount
				std::for_each(std::execution::par_unseq, renderInfos.begin(), renderInfos.end(),
					[&](auto& info)
					{
						int sum = 0;
						if (d < nodeIntersectionPerPixelCount[info.index].size())
						{
							sum = nodeIntersectionPerPixelCount[info.index][d];
						}

						//Color c(sum * 0.01f);
						Color c(sum * (1 / (float)maxDepth[d]));
						image[info.index * 4 + 0] = (unsigned char)(c.r * 255);
						image[info.index * 4 + 1] = (unsigned char)(c.g * 255);
						image[info.index * 4 + 2] = (unsigned char)(c.b * 255);
						image[info.index * 4 + 3] = (unsigned char)(c.a * 255);
					});
				encodeTwoSteps(path + "/" + name + problem + "_NodeDepth" + std::to_string(d) + ".png", image, width, height);
			}

			float normalisation = nodeIntersectionPerDepthCount.size();
			int minDepth = normalisation;
			//find minimum:
			std::for_each(std::execution::seq, renderInfos.begin(), renderInfos.end(),
				[&](auto& info)
				{
					//int sum = std::accumulate(nodeIntersectionPerPixelCount[info.index].begin(), nodeIntersectionPerPixelCount[info.index].end(), 0);
					minDepth = std::min(minDepth, (int)nodeIntersectionPerPixelCount[info.index].size());
				});
			normalisation = 1 / (normalisation - minDepth);
			//pixel bvh depth
			std::cout << normalisation << std::endl;
			std::cout << minDepth << std::endl;
			std::for_each(std::execution::par_unseq, renderInfos.begin(), renderInfos.end(),
				[&](auto& info)
				{
					//int sum = std::accumulate(nodeIntersectionPerPixelCount[info.index].begin(), nodeIntersectionPerPixelCount[info.index].end(), 0);
					int sum = nodeIntersectionPerPixelCount[info.index].size() - minDepth;
					//Color c(sum * 0.01f);
					Color c(sum * normalisation);
					image[info.index * 4 + 0] = (unsigned char)(c.r * 255);
					image[info.index * 4 + 1] = (unsigned char)(c.g * 255);
					image[info.index * 4 + 2] = (unsigned char)(c.b * 255);
					image[info.index * 4 + 3] = (unsigned char)(c.a * 255);
				});
			encodeTwoSteps(path + "/" + name + problem + "_PixelDepth.png", image, width, height);
		}
	}

private:

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