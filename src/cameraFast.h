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

#include <numeric>      // std::accumulate

#include "timing.h"



//Fast version of camera that does no intersection counters but is only for performance analysis
class CameraFast : public Camera
{
private:
	std::vector<double> times;
	std::vector<double> times2;
public:
	//only black and white image for now ->  image is only height * width and not height * width * 4

	CameraFast(std::string path, std::string name, std::string problem, glm::vec3 position, glm::vec3 lookCenter
		, glm::vec3 upward = glm::vec3(0, 1, 0), float focalLength = 0.866f, size_t height = 1080, size_t width = 1920)
		:Camera(path, name, problem, position, lookCenter, upward, focalLength, height, width)
	{
		image.resize(height * width * 4);
		times.resize(height * width);
		times2.resize(height * width);
	}

	CameraFast(std::string path, std::string name, std::string problem, glm::mat4 transform,
		float focalLength = 0.866f, size_t height = 1080, size_t width = 1920)
		:Camera(path, name, problem, transform, focalLength, height, width)
	{
		image.resize(height * width * 4);
		times.resize(height * width);
	}

	void renderImage(bool saveImage, FastNodeManager nodeManager, unsigned ambientSampleCount,
		float ambientDistance, bool mute)
	{
		fillRenderInfo();

		auto timeBeginRaytracer = getTime();

		double triangleTestTime = 0;

		std::for_each(std::execution::par_unseq, renderInfos.begin(), renderInfos.end(),
			[&](auto& info)
			{
				auto timeBeforeRay = getTime();
				double timeTriangleTest = 0;
				glm::vec3 pos = getRayTargetPosition(info);
				auto ray = FastRay(position, pos - position);

				uint8_t imageResult = 0;

				//shoot primary ray.
				bool result = nodeManager.intersect(ray, timeTriangleTest);

				//glm::vec3 ambientSum = glm::vec3(0);

				if (result)
				{
					//shoot secondary ray:
					unsigned ambientResult = 0;

					for (size_t i = 0; i < ambientSampleCount; i++)
					{
						//deterministic random direction
						auto direction = getAmbientDirection(info, i, ray.surfaceNormal);
						//ambientSum += direction;
						auto secondaryRay = FastRay(ray.surfacePosition + direction * 0.001f, direction, true);
						secondaryRay.tMax = ambientDistance;
						//shoot secondary ray
						if (nodeManager.intersect(secondaryRay, timeTriangleTest))
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
				times2[info.index] = timeTriangleTest;
				times[info.index] = getTimeSpan(timeBeforeRay);
				//distance render version (for large scenes)
				//float distanceToCamera =  glm::distance(ray.surfacePosition, ray.pos);
				//imageResult = (uint8_t)(distanceToCamera / 50.f);

				image[info.index * 4 + 0] = imageResult;
				image[info.index * 4 + 1] = imageResult;
				image[info.index * 4 + 2] = imageResult;
				image[info.index * 4 + 3] = 255;

				//render ambient sum average:
				//ambientSum = glm::normalize(ambientSum);
				//image[info.index * 4 + 0] = (uint8_t)(ambientSum.x * 127 + 127);
				//image[info.index * 4 + 1] = (uint8_t)(ambientSum.y * 127 + 127);
				//image[info.index * 4 + 2] = (uint8_t)(ambientSum.z * 127 + 127);

				//surface normal render version:
				//image[info.index * 4 + 0] = (uint8_t)(ray.surfaceNormal.x * 127 + 127);
				//image[info.index * 4 + 1] = (uint8_t)(ray.surfaceNormal.y * 127 + 127);
				//image[info.index * 4 + 2] = (uint8_t)(ray.surfaceNormal.z * 127 + 127);



			});

		double timeSum = std::accumulate(times.begin(), times.end(), 0.0);
		double timeSum2 = std::accumulate(times2.begin(), times2.end(), 0.0);
		if (!mute)
		{
			std::cout << "Raytracing took " << getTimeSpan(timeBeginRaytracer) << " seconds." << std::endl;
			std::cout << "ray sum took " << timeSum<< " seconds." << std::endl;
			std::cout << "triangleTests took " << timeSum2 << " seconds." << std::endl;
		}

		if (saveImage)
		{
			encodeTwoSteps(path + "/" + name + ".png", image, width, height);
		}


	}
};