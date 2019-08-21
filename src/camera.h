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
	int index;

	RenderInfo(int w, int h, int index) : w(w), h(h), index(index)
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

	int height;
	int width;

	float sizeFactor = 0.1f;
	float farPlane = 50;

	std::unique_ptr<Bvh> bvh;

	Camera(std::unique_ptr<Bvh> bvh, glm::vec3 position, glm::vec3 forward, glm::vec3 upward = glm::vec3(0, 1, 0), int height = 300, int width = 600) : position(position), forward(forward), upward(upward), height(height), width(width)
	{
		this->bvh = std::move(bvh);


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
		std::for_each(std::execution::seq, renderInfos.begin(), renderInfos.end(),
			[&](auto& info)
			{
				glm::vec3 pos = position + (upward * (float)info.h - right * (float)info.w) * sizeFactor;

				//todo: move to some method for sampling?

				auto ray = std::make_shared<Ray>(pos, forward);

				auto result = bvh->intersect(ray);
				image[info.index * 4 + 0] = ray->result[0];
				image[info.index * 4 + 1] = ray->result[1];
				image[info.index * 4 + 2] = ray->result[2];
				image[info.index * 4 + 3] = 255;
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
	void encodeTwoSteps(const char* filename, std::vector<unsigned char>& image, unsigned width, unsigned height)
	{
		std::vector<unsigned char> png;

		unsigned error = lodepng::encode(png, image, width, height);
		if (!error) lodepng::save_file(png, filename);

		//if there's an error, display it
		if (error) std::cout << "encoder error " << error << ": " << lodepng_error_text(error) << std::endl;
	}

};