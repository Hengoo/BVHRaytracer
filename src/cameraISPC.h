#pragma once
#include "lights/light.h"
#include "glmInclude.h"
#include "util.h"

#include "camera.h"

#include "accelerationStructure/fastNodeManager.h"

#include <iostream>
// writing on a text file
#include <fstream>

#include <algorithm>
#include <vector>

//for the parallel for
#include <execution>

//includes for the timer
#include <ctime>
#include <ratio>
#include <chrono>



//Fast version of camera that does no intersection counters but is only for performance analysis
class CameraISPC : public Camera
{
public:
	//only black and white image for now ->  image is only height * width and not height * width * 4

	CameraISPC(std::string path, std::string name, std::string problem, glm::vec3 position, glm::vec3 lookCenter
		, glm::vec3 upward = glm::vec3(0, 1, 0), float focalLength = 0.866f, size_t height = 1080, size_t width = 1920)
		:Camera(path, name, problem, position, lookCenter, upward, focalLength, height, width)
	{
		image.resize(height * width * 4);
	}

	CameraISPC(std::string path, std::string name, std::string problem, glm::mat4 transform,
		float focalLength = 0.866f, size_t height = 1080, size_t width = 1920)
		:Camera(path, name, problem, transform, focalLength, height, width)
	{
		image.resize(height * width);
	}

	void renderImage(bool saveImage, FastNodeManager nodeManager, unsigned ambientSampleCount,
		float ambientDistance, bool mute)
	{
		fillRenderInfo();

		std::chrono::high_resolution_clock::time_point timeBegin = std::chrono::high_resolution_clock::now();

		std::for_each(std::execution::par_unseq, renderInfos.begin(), renderInfos.end(),
			[&](auto& info)
			{
				glm::vec3 pos = getRayTargetPosition(info);
				auto ray = FastRay(position, pos - position);

				uint8_t imageResult = 0;

				//shoot primary ray.
				bool result = nodeManager.intersect(ray);

				if (result)
				{
					//shoot secondary ray:
					unsigned ambientResult = 0;

					for (size_t i = 0; i < ambientSampleCount; i++)
					{
						//deterministic random direction
						auto direction = getAmbientDirection(info, i, ray.surfaceNormal);
						auto secondaryRay = FastRay(ray.surfacePosition + direction * 0.001f, direction, true);
						secondaryRay.tMax = ambientDistance;
						//shoot secondary ray
						if (nodeManager.intersect(secondaryRay))
						{
							ambientResult++;
						}
					}

					if (ambientSampleCount != 0)
					{
						float factor = 1 - (ambientResult / (float)ambientSampleCount);
						factor = (factor + 1) / 2.f;
						imageResult = (uint8_t)(factor * 255);
					}
				}

				image[info.index * 4 + 0] = imageResult;
				image[info.index * 4 + 1] = imageResult;
				image[info.index * 4 + 2] = imageResult;
				image[info.index * 4 + 3] = 255;

			});

		std::chrono::high_resolution_clock::time_point timeEnd = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> timeSpan = std::chrono::duration_cast<std::chrono::duration<double>>(timeEnd - timeBegin);

		if (!mute)
		{
			std::cout << "Raytracing took " << timeSpan.count() << " seconds." << std::endl;
		}

		if (saveImage)
		{
			encodeTwoSteps(path + "/" + name + ".png", image, width, height);
		}


	}
};