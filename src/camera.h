#pragma once
#include <iostream>
#include <algorithm>
#include <vector>

//for the parallel for
#include <execution>

#include "glmInclude.h"
#include "lodepng/lodepng.h"
#include "global.h"

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

	//should have more camrea parameters

	glm::vec3 position;

	glm::mat4 transform;

	std::vector<unsigned char> image;

	std::vector<std::vector<unsigned int>> nodeIntersectionPerPixelCount;
	std::vector<std::vector<unsigned int>> leafIntersectionPerPixelCount;
	std::vector<std::vector<unsigned int>> childFullnessPerPixelCount;
	std::vector<std::vector<unsigned int>> primitiveFullnessPerPixelCount;
	std::vector<size_t> nodeIntersectionPerDepthCount;
	std::vector<size_t> leafIntersectionPerDepthCount;
	std::vector<size_t> primitiveIntersectionsPerPixel;
	std::vector<size_t> successfulPrimitiveIntersectionsPerPixel;
	std::vector<size_t> successfulNodeIntersectionsPerPixel;
	std::vector<size_t> successfulLeafIntersectionsPerPixel;
	std::vector<size_t> primitiveFullness;
	std::vector<size_t> childFullness;
	size_t nodeIntersectionCount;
	size_t leafIntersectionCount;
	size_t primitiveIntersections;
	size_t successfulPrimitiveIntersections;
	size_t successfulNodeIntersections;
	size_t successfulLeafIntersections;

	//contains all info needed to spawn the ray for the specific pixel. Only needed because i dont know how to get the loop index into the unsequenced for_each
	std::vector<RenderInfo> renderInfos;

	size_t height;
	size_t width;

	//1.37f for 40 degree, 0.866f for 60 degree (horizontal fov)
	float focalLength;

	//todo not implemented (most likely not needed anyway)
	float farPlane = 50;

	Camera(glm::vec3 position, glm::vec3 lookCenter, glm::vec3 upward = glm::vec3(0, 1, 0), float focalLength = 0.866f, size_t height = 1080, size_t width = 1920) : position(position), focalLength(focalLength), height(height), width(width)
	{
		transform = glm::inverse(glm::lookAt(position, lookCenter, upward));

		image.resize(height * width * 4);
		renderInfos.resize(height * width);
		nodeIntersectionPerPixelCount.resize(height * width);
		leafIntersectionPerPixelCount.resize(height * width);
		childFullnessPerPixelCount.resize(height * width);
		primitiveFullnessPerPixelCount.resize(height * width);
		primitiveIntersectionsPerPixel.resize(height * width);
		successfulPrimitiveIntersectionsPerPixel.resize(height * width);
		successfulNodeIntersectionsPerPixel.resize(height * width);
		successfulLeafIntersectionsPerPixel.resize(height * width);

		nodeIntersectionCount = 0;
		leafIntersectionCount = 0;
		primitiveIntersections = 0;
		successfulPrimitiveIntersections = 0;
		successfulNodeIntersections = 0;
		successfulLeafIntersections = 0;
	}

	Camera(glm::mat4 transform, float focalLength = 0.866f, size_t height = 1000, size_t width = 1000) : transform(transform), focalLength(focalLength), height(height), width(width)
	{
		image.resize(height * width * 4);
		renderInfos.resize(height * width);
		nodeIntersectionPerPixelCount.resize(height * width);
		leafIntersectionPerPixelCount.resize(height * width);
		childFullnessPerPixelCount.resize(height * width);
		primitiveFullnessPerPixelCount.resize(height * width);
		primitiveIntersectionsPerPixel.resize(height * width);
		successfulPrimitiveIntersectionsPerPixel.resize(height * width);
		successfulNodeIntersectionsPerPixel.resize(height * width);
		successfulLeafIntersectionsPerPixel.resize(height * width);

		position = transform * glm::vec4(0, 0, 0, 1);
		nodeIntersectionCount = 0;
		leafIntersectionCount = 0;
		primitiveIntersections = 0;
		successfulPrimitiveIntersections = 0;
		successfulNodeIntersections = 0;
		successfulLeafIntersections = 0;
	}

	//spawns rays and collects results into image. Image is written on disk
	void renderImage()
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
				//result is kinda not needed?

				//resolve shadows, ez:
				for (auto& l : lights)
				{
					float lightDistance;
					glm::vec3 lightVector;

					//todo use light color
					auto lightColor = l->getLightDirection(ray.surfacePosition, lightVector, lightDistance);

					float f = glm::dot(ray.surfaceNormal, lightVector);
					Ray shadowRay(ray.surfacePosition, lightVector, true);
					shadowRay.tMax = lightDistance;


					//only shoot ray when surface points in light direction
					if (f > 0)
					{
						if (bvh.intersect(shadowRay))
						{
							f = 0;
						}

						//add shadowRay intersection to this ones
						if (ray.nodeIntersectionCount.size() < shadowRay.nodeIntersectionCount.size())
						{
							ray.nodeIntersectionCount.resize(shadowRay.nodeIntersectionCount.size());
						}
						for (size_t i = 0; i < shadowRay.nodeIntersectionCount.size(); i++)
						{
							ray.nodeIntersectionCount[i] += shadowRay.nodeIntersectionCount[i];
						}

						if (ray.leafIntersectionCount.size() < shadowRay.leafIntersectionCount.size())
						{
							ray.leafIntersectionCount.resize(shadowRay.leafIntersectionCount.size());
						}
						for (size_t i = 0; i < shadowRay.leafIntersectionCount.size(); i++)
						{
							ray.leafIntersectionCount[i] += shadowRay.leafIntersectionCount[i];
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

						primitiveIntersectionsPerPixel[info.index] += shadowRay.primitiveIntersectionCount;
						successfulPrimitiveIntersectionsPerPixel[info.index] += shadowRay.successfulPrimitiveIntersectionCount;
						successfulNodeIntersectionsPerPixel[info.index] += shadowRay.successfulNodeIntersectionCount;
						successfulLeafIntersectionsPerPixel[info.index] += shadowRay.successfulLeafIntersectionCount;
					}

					f = std::max(0.2f, f);
					ray.surfaceColor.scale(f);
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
				successfulNodeIntersectionsPerPixel[info.index] += ray.successfulNodeIntersectionCount;
				successfulLeafIntersectionsPerPixel[info.index] += ray.successfulLeafIntersectionCount;
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

		//TODO: could seperate different ray types (primary, shadowray, ...)
		leafIntersectionCount = std::accumulate(leafIntersectionPerDepthCount.begin(), leafIntersectionPerDepthCount.end(), 0);
		nodeIntersectionCount = std::accumulate(nodeIntersectionPerDepthCount.begin(), nodeIntersectionPerDepthCount.end(), 0);

		primitiveIntersections = std::accumulate(primitiveIntersectionsPerPixel.begin(), primitiveIntersectionsPerPixel.end(), 0);
		successfulPrimitiveIntersections = std::accumulate(successfulPrimitiveIntersectionsPerPixel.begin(), successfulPrimitiveIntersectionsPerPixel.end(), 0);
		successfulNodeIntersections = std::accumulate(successfulNodeIntersectionsPerPixel.begin(), successfulNodeIntersectionsPerPixel.end(), 0);
		successfulLeafIntersections = std::accumulate(successfulLeafIntersectionsPerPixel.begin(), successfulLeafIntersectionsPerPixel.end(), 0);

		std::cout << "node intersections: " << nodeIntersectionCount << std::endl;
		std::cout << "node success ration: " << successfulNodeIntersections / (float)nodeIntersectionCount << std::endl;
		std::cout << "leaf intersections: " << leafIntersectionCount << std::endl;
		std::cout << "leaf success ration: " << successfulLeafIntersections / (float)leafIntersectionCount << std::endl;
		std::cout << "primitive intersections: " << primitiveIntersections << std::endl;
		std::cout << "primitive success ratio: " << successfulPrimitiveIntersections / (float)primitiveIntersections << std::endl;
		std::cout << std::endl;

		for (size_t i = 0; i < nodeIntersectionPerDepthCount.size(); i++)
		{
			std::cout << "node intersections at depth " << i << " : " << nodeIntersectionPerDepthCount[i] << std::endl;
		}
		std::cout << std::endl;
		for (size_t i = 0; i < leafIntersectionPerDepthCount.size(); i++)
		{
			std::cout << "leaf intersections at depth " << i << " : " << leafIntersectionPerDepthCount[i] << std::endl;
		}

		//IMPORTANT: this number is smaller than the nodecount + leafcount because the depth 0 intersections are left out
		std::cout << std::endl;
		for (size_t i = 0; i < childFullness.size(); i++)
		{
			std::cout << "intersections with nodes with " << i << " children: " << childFullness[i] << std::endl;
		}

		std::cout << std::endl;
		for (size_t i = 0; i < primitiveFullness.size(); i++)
		{
			std::cout << "intersections with nodes with " << i << " primitives: " << primitiveFullness[i] << std::endl;
		}

		encodeTwoSteps("why.png", image, width, height);
	}

private:

	// Encode from raw pixels to an in - memory PNG file first, then write it to disk
	//The image argument has width * height RGBA pixels or width * height * 4 bytes
	void encodeTwoSteps(const char* encodeFilename, std::vector<unsigned char>& encodeImage, unsigned encodeWidth, unsigned encodeHeight)
	{
		std::vector<unsigned char> png;

		unsigned error = lodepng::encode(png, encodeImage, encodeWidth, encodeHeight);
		if (!error) lodepng::save_file(png, encodeFilename);

		//if there's an error, display it
		if (error) std::cout << "encoder error " << error << ": " << lodepng_error_text(error) << std::endl;
	}

};