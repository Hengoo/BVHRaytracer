#pragma once
#include <iostream>
#include <algorithm>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> //https://glm.g-truc.net/0.9.4/api/a00158.html  for make quat from a pointer to a array
#include <glm/gtx/hash.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "accelerationStructure/bvh.h"
#include "lodepng/lodepng.h"

//spawns all the rays in the scene
class Camera
{
public:

	//should have more camrea parameters

	glm::vec3 position;
	glm::vec3 forward;
	glm::vec3 upward;
	std::vector<unsigned char> image;

	int height;
	int width;

	std::unique_ptr<Bvh> bvh;

	Camera(std::unique_ptr<Bvh> bvh, glm::vec3 position, glm::vec3 forward, glm::vec3 upward = glm::vec3(0, 1, 0), int height = 30, int width = 60) : position(position), forward(forward), upward(upward), height(height), width(width)
	{
		this->bvh = std::move(bvh);
		forward = glm::normalize(forward);
		upward = glm::normalize(upward);
		image.resize(height * width * 4);
	}

	//spawns rays and collects results into image. Image is written on disk
	void renderImage()
	{
		float sizeFactor = 0.1f;
		float farPlane = 20;
		//simplified ortho version for now:
		for (int i = 0; i < width * height; i++)
		{
			int w = i % width - width / 2.f;
			int h = i / width - height / 2.f;
			glm::vec3 pos = position + (upward * (float)h - glm::cross(upward, forward) * (float)w) * sizeFactor;
			auto ray = std::make_shared<Ray>(pos, forward);
			auto result = bvh->intersect(ray);
			//image[i * 4] = ray->distance / farPlane * 255;
			//image[i * 4 + 1] = ray->distance / farPlane * 255;
			//image[i * 4 + 2] = ray->distance / farPlane * 255;
			image[i * 4] = ray->result[0];
			image[i * 4 + 1] = ray->result[1];
			image[i * 4 + 2] = ray->result[2];
			image[i * 4 + 3] = 255;

			if (i % width == i / width)
			{
				image[i * 4 + 1] = 255;
			}
		}

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
	void encodeTwoSteps(const char* filename, std::vector<unsigned char>& image, unsigned width, unsigned height)
	{
		std::vector<unsigned char> png;

		unsigned error = lodepng::encode(png, image, width, height);
		if (!error) lodepng::save_file(png, filename);

		//if there's an error, display it
		if (error) std::cout << "encoder error " << error << ": " << lodepng_error_text(error) << std::endl;
	}

};