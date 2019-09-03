#pragma once
#include <iostream>
#include <algorithm>
#include <vector>

//for the parallel for
#include <execution>

#include "glmInclude.h"
#include "lodepng/lodepng.h"

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
	std::vector<std::vector<unsigned int>> primitiveIntersectionPerPixelCount;
	std::vector<size_t> nodeIntersectionPerDepthCount;
	std::vector<size_t> primitiveIntersectionPerDepthCount;
	size_t nodeIntersectionCount;
	size_t primitiveIntersectionCount;

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
		nodeIntersectionPerPixelCount.resize(height * width);
		primitiveIntersectionPerPixelCount.resize(height * width);
		renderInfos.resize(height * width);
		nodeIntersectionCount = 0;
		primitiveIntersectionCount = 0;

	}

	Camera(glm::mat4 transform, float focalLength = 0.866f, size_t height = 1000, size_t width = 1000) : transform(transform), focalLength(focalLength), height(height), width(width)
	{
		image.resize(height * width * 4);
		renderInfos.resize(height * width);
		nodeIntersectionPerPixelCount.resize(height * width);
		primitiveIntersectionPerPixelCount.resize(height * width);
		position = transform * glm::vec4(0, 0, 0, 1);
		nodeIntersectionCount = 0;
		primitiveIntersectionCount = 0;
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
				if (result)
				{
					auto debugOnly = 0;
				}
				image[info.index * 4 + 0] = (unsigned char)(ray.result.r * 255);
				image[info.index * 4 + 1] = (unsigned char)(ray.result.g * 255);
				image[info.index * 4 + 2] = (unsigned char)(ray.result.b * 255);
				image[info.index * 4 + 3] = (unsigned char)(ray.result.a * 255);

				nodeIntersectionPerPixelCount[info.index] = ray.nodeIntersectionCount;
				primitiveIntersectionPerPixelCount[info.index] = ray.primitiveIntersectionCount;
			});

		for (auto& perPixel : primitiveIntersectionPerPixelCount)
		{
			for (size_t i = 0; i < perPixel.size(); i++)
			{
				if (primitiveIntersectionPerDepthCount.size() < perPixel.size())
				{
					primitiveIntersectionPerDepthCount.resize(perPixel.size());
				}
				primitiveIntersectionPerDepthCount[i] += perPixel[i];
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

		//TODO: could seperate different ray types (primary, shadowray, ...)
		primitiveIntersectionCount = std::accumulate(primitiveIntersectionPerDepthCount.begin(), primitiveIntersectionPerDepthCount.end(), 0);
		nodeIntersectionCount = std::accumulate(nodeIntersectionPerDepthCount.begin(), nodeIntersectionPerDepthCount.end(), 0);
		std::cout << "node      intersections: " << nodeIntersectionCount << std::endl;
		std::cout << "primitive intersections: " << primitiveIntersectionCount << std::endl ;
		std::cout << std::endl;
		
		for (size_t i = 0; i < nodeIntersectionPerDepthCount.size(); i++)
		{
			std::cout << "node      intersections at depth " << i << " : " << nodeIntersectionPerDepthCount[i] << std::endl;
		}
		std::cout << std::endl;
		for (size_t i = 0; i < primitiveIntersectionPerDepthCount.size(); i++)
		{
			std::cout << "primitive intersections at depth " << i << " : " << primitiveIntersectionPerDepthCount[i] << std::endl;
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