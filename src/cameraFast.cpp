#include "cameraFast.h"

#include "timing.h"
#include "util.h"
#include "fastRay.h"
#include "accelerationStructure/fastNodeManager.h"

#include <iostream>
// writing on a text file
#include <fstream>

#include <algorithm>
#include <vector>

//for the parallel for
#include <execution>

#include <numeric>
// std::accumulate

template void CameraFast::renderImage(bool saveImage, FastNodeManager<4, 4> nodeManager, unsigned ambientSampleCount, float ambientDistance, bool mute);
template void CameraFast::renderImage(bool saveImage, FastNodeManager<4, 8> nodeManager, unsigned ambientSampleCount, float ambientDistance, bool mute);
template void CameraFast::renderImage(bool saveImage, FastNodeManager<4, 12> nodeManager, unsigned ambientSampleCount, float ambientDistance, bool mute);
template void CameraFast::renderImage(bool saveImage, FastNodeManager<4, 16> nodeManager, unsigned ambientSampleCount, float ambientDistance, bool mute);

template void CameraFast::renderImage(bool saveImage, FastNodeManager<8, 4> nodeManager, unsigned ambientSampleCount, float ambientDistance, bool mute);
template void CameraFast::renderImage(bool saveImage, FastNodeManager<8, 8> nodeManager, unsigned ambientSampleCount, float ambientDistance, bool mute);
template void CameraFast::renderImage(bool saveImage, FastNodeManager<8, 12> nodeManager, unsigned ambientSampleCount, float ambientDistance, bool mute);
template void CameraFast::renderImage(bool saveImage, FastNodeManager<8, 16> nodeManager, unsigned ambientSampleCount, float ambientDistance, bool mute);

CameraFast::CameraFast(std::string path, std::string name, std::string problem, glm::vec3 position, glm::vec3 lookCenter
	, glm::vec3 upward, float focalLength, size_t height, size_t width)
	:Camera(path, name, problem, position, lookCenter, upward, focalLength, height, width)
{
	image.resize(height * width * 4);
	times.resize(height * width);
	times2.resize(height * width);
}

CameraFast::CameraFast(std::string path, std::string name, std::string problem, glm::mat4 transform,
	float focalLength, size_t height, size_t width)
	:Camera(path, name, problem, transform, focalLength, height, width)
{
	image.resize(height * width * 4);
	times.resize(height * width);
}


template <size_t gangSize, size_t nodeMemory>
void CameraFast::renderImage(bool saveImage, FastNodeManager<gangSize, nodeMemory> nodeManager,
	unsigned ambientSampleCount, float ambientDistance, bool mute)
{
	fillRenderInfo();

	auto timeBeginRaytracer = getTime();

	double triangleTestTime = 0;
	std::for_each(std::execution::seq, renderInfos.begin(), renderInfos.end(),
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
	double totalTime = getTimeSpan(timeBeginRaytracer);
	if (!mute)
	{
		std::cout << "Raytracing took " << totalTime << " seconds." << std::endl;
		std::cout << "ray sum took " << timeSum << " seconds." << std::endl;
		std::cout << "triangleTests took " << timeSum2 << " seconds." << std::endl;
		std::cout << "all rays minus triangle took " << timeSum - timeSum2 << " seconds." << std::endl;
	}

	std::ofstream myfile(path + "/" + name + problem + "_Perf.txt");
	if (myfile.is_open())
	{
		myfile << "scenario " << name << " with branching factor of " << nodeManager.branchingFactor << " and leafsize of " << nodeManager.leafSize << std::endl;
		myfile << "Raytracer total time: " << totalTime << std::endl;
		myfile << "Time for all rays (SUM): " << timeSum << std::endl;
		myfile << "Time for triangle intersections (SUM): " << timeSum2 << std::endl;
		myfile << "Time all rays(sum) - triangle(sum): " << timeSum - timeSum2 << std::endl;
	}
	myfile.close();
	if (saveImage)
	{
		encodeTwoSteps(path + "/" + name + "_Perf.png", image, width, height);
	}
}