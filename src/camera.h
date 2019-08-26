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
	glm::vec3 forward;
	glm::vec3 upward;
	glm::vec3 right;

	std::vector<unsigned char> image;

	//contains all info needed to spawn the ray for the specific pixel. Only needed because i dont know how to get the loop index into the unsequenced for_each
	std::vector<RenderInfo> renderInfos;

	size_t height;
	size_t width;

	float sizeFactor;

	//todo not implemented (most likely not needed anyway)
	float farPlane = 50;

	Camera(float pixelSize, glm::vec3 position, glm::vec3 forward, glm::vec3 upward = glm::vec3(0, 1, 0), size_t height = 1000, size_t width = 1000) : position(position), sizeFactor(pixelSize), forward(forward), upward(upward), height(height), width(width)
	{
		right = glm::cross(forward, upward);
		this->upward = glm::cross(right, forward);
		right = glm::normalize(right);
		this->forward = glm::normalize(this->forward);
		this->upward = glm::normalize(this->upward);

		image.resize(height * width * 4);
		renderInfos.resize(height * width);

	}

	//spawns rays and collects results into image. Image is written on disk
	void renderImage()
	{
		//simplified ortho version for now:

		//fill RenderInfo array.
		for (int i = 0; i < width * height; i++)
		{
			float w = -(i % width - width / 2.f);
			float h = -(i / width - height / 2.f);

			renderInfos[i] = RenderInfo(w, h, i);
		}

		//todo: 
		//not a lambda expert: [this] seems to be needed so i can access object variables
		std::for_each(std::execution::par_unseq, renderInfos.begin(), renderInfos.end(),
			[&](auto& info)
			{
				glm::vec3 pos = position + (upward * (float)info.h - right * (float)info.w) * sizeFactor;

				//todo: move to some method for sampling?

				auto ray = Ray(pos, forward);

				auto result = bvh.intersect(ray);
				if (result)
				{
					auto debugOnly = 0;
				}
				image[info.index * 4 + 0] = (unsigned char)(ray.result.r * 255);
				image[info.index * 4 + 1] = (unsigned char)(ray.result.g * 255);
				image[info.index * 4 + 2] = (unsigned char)(ray.result.b * 255);
				image[info.index * 4 + 3] = (unsigned char)(ray.result.a * 255);
			});

		/*
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				if (i == j)
				{
					image[4 * (i * width + j) + 2] = 255;
				}
			}
		}*/

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