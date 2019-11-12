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


//I just want to use template in the cpp ...
template std::tuple<float, float, float> CameraFast::renderImage(bool saveImage, FastNodeManager<4, 4> nodeManager, unsigned ambientSampleCount, float ambientDistance, bool mute);
template std::tuple<float, float, float> CameraFast::renderImage(bool saveImage, FastNodeManager<4, 8> nodeManager, unsigned ambientSampleCount, float ambientDistance, bool mute);
template std::tuple<float, float, float> CameraFast::renderImage(bool saveImage, FastNodeManager<4, 12> nodeManager, unsigned ambientSampleCount, float ambientDistance, bool mute);
template std::tuple<float, float, float> CameraFast::renderImage(bool saveImage, FastNodeManager<4, 16> nodeManager, unsigned ambientSampleCount, float ambientDistance, bool mute);
template std::tuple<float, float, float> CameraFast::renderImage(bool saveImage, FastNodeManager<4, 20> nodeManager, unsigned ambientSampleCount, float ambientDistance, bool mute);
template std::tuple<float, float, float> CameraFast::renderImage(bool saveImage, FastNodeManager<4, 24> nodeManager, unsigned ambientSampleCount, float ambientDistance, bool mute);
template std::tuple<float, float, float> CameraFast::renderImage(bool saveImage, FastNodeManager<4, 28> nodeManager, unsigned ambientSampleCount, float ambientDistance, bool mute);
template std::tuple<float, float, float> CameraFast::renderImage(bool saveImage, FastNodeManager<4, 32> nodeManager, unsigned ambientSampleCount, float ambientDistance, bool mute);

template std::tuple<float, float, float>  CameraFast::renderImage(bool saveImage, FastNodeManager<8, 8> nodeManager, unsigned ambientSampleCount, float ambientDistance, bool mute);
template std::tuple<float, float, float>  CameraFast::renderImage(bool saveImage, FastNodeManager<8, 16> nodeManager, unsigned ambientSampleCount, float ambientDistance, bool mute);
template std::tuple<float, float, float>  CameraFast::renderImage(bool saveImage, FastNodeManager<8, 24> nodeManager, unsigned ambientSampleCount, float ambientDistance, bool mute);
template std::tuple<float, float, float>  CameraFast::renderImage(bool saveImage, FastNodeManager<8, 32> nodeManager, unsigned ambientSampleCount, float ambientDistance, bool mute);
template std::tuple<float, float, float>  CameraFast::renderImage(bool saveImage, FastNodeManager<8, 40> nodeManager, unsigned ambientSampleCount, float ambientDistance, bool mute);
template std::tuple<float, float, float>  CameraFast::renderImage(bool saveImage, FastNodeManager<8, 48> nodeManager, unsigned ambientSampleCount, float ambientDistance, bool mute);
template std::tuple<float, float, float>  CameraFast::renderImage(bool saveImage, FastNodeManager<8, 56> nodeManager, unsigned ambientSampleCount, float ambientDistance, bool mute);
template std::tuple<float, float, float>  CameraFast::renderImage(bool saveImage, FastNodeManager<8, 64> nodeManager, unsigned ambientSampleCount, float ambientDistance, bool mute);

template void CameraFast::renderImages(bool saveImage, FastNodeManager<4, 4> nodeManager, unsigned ambientSampleCount, float ambientDistance, bool mute);
template void CameraFast::renderImages(bool saveImage, FastNodeManager<4, 8> nodeManager, unsigned ambientSampleCount, float ambientDistance, bool mute);
template void CameraFast::renderImages(bool saveImage, FastNodeManager<4, 12> nodeManager, unsigned ambientSampleCount, float ambientDistance, bool mute);
template void CameraFast::renderImages(bool saveImage, FastNodeManager<4, 16> nodeManager, unsigned ambientSampleCount, float ambientDistance, bool mute);
template void CameraFast::renderImages(bool saveImage, FastNodeManager<4, 20> nodeManager, unsigned ambientSampleCount, float ambientDistance, bool mute);
template void CameraFast::renderImages(bool saveImage, FastNodeManager<4, 24> nodeManager, unsigned ambientSampleCount, float ambientDistance, bool mute);
template void CameraFast::renderImages(bool saveImage, FastNodeManager<4, 28> nodeManager, unsigned ambientSampleCount, float ambientDistance, bool mute);
template void CameraFast::renderImages(bool saveImage, FastNodeManager<4, 32> nodeManager, unsigned ambientSampleCount, float ambientDistance, bool mute);

template void CameraFast::renderImages(bool saveImage, FastNodeManager<8, 8> nodeManager, unsigned ambientSampleCount, float ambientDistance, bool mute);
template void CameraFast::renderImages(bool saveImage, FastNodeManager<8, 16> nodeManager, unsigned ambientSampleCount, float ambientDistance, bool mute);
template void CameraFast::renderImages(bool saveImage, FastNodeManager<8, 24> nodeManager, unsigned ambientSampleCount, float ambientDistance, bool mute);
template void CameraFast::renderImages(bool saveImage, FastNodeManager<8, 32> nodeManager, unsigned ambientSampleCount, float ambientDistance, bool mute);
template void CameraFast::renderImages(bool saveImage, FastNodeManager<8, 40> nodeManager, unsigned ambientSampleCount, float ambientDistance, bool mute);
template void CameraFast::renderImages(bool saveImage, FastNodeManager<8, 48> nodeManager, unsigned ambientSampleCount, float ambientDistance, bool mute);
template void CameraFast::renderImages(bool saveImage, FastNodeManager<8, 56> nodeManager, unsigned ambientSampleCount, float ambientDistance, bool mute);
template void CameraFast::renderImages(bool saveImage, FastNodeManager<8, 64> nodeManager, unsigned ambientSampleCount, float ambientDistance, bool mute);


CameraFast::CameraFast(std::string path, std::string name, std::string problem, std::string problemPrefix, glm::vec3 position, glm::vec3 lookCenter
	, glm::vec3 upward, float focalLength, size_t height, size_t width)
	:Camera(path, name, problem, position, lookCenter, upward, focalLength, height, width), problemPrefix(problemPrefix)
{
	image.resize(height * width * 4);
	timesRay.resize(height * width);
	timesTriangles.resize(height * width);
}

CameraFast::CameraFast(std::string path, std::string name, std::string problem, std::string problemPrefix, glm::mat4 transform,
	float focalLength, size_t height, size_t width)
	:Camera(path, name, problem, transform, focalLength, height, width), problemPrefix(problemPrefix)
{
	image.resize(height * width * 4);
	timesRay.resize(height * width);
}

template <size_t gangSize, size_t nodeMemory>
void CameraFast::renderImages(bool saveImage, FastNodeManager<gangSize, nodeMemory> nodeManager,
	unsigned ambientSampleCount, float ambientDistance, bool mute)
{
	renderImage(saveImage, nodeManager, ambientSampleCount, ambientDistance, mute);

	//has to be an odd number for correct median
	int sampleCount = 5;
	std::vector<std::tuple<float, float, float>> results;
	results.reserve(sampleCount);
	//median. (median of overall time or of each individual time?
	for (int i = 0; i < sampleCount; i++)
	{
		results.push_back(renderImage(false, nodeManager, ambientSampleCount, ambientDistance, false));
	}
	//calculate median
	sort(results.begin(), results.end());
	std::cout << sampleCount / 2 + 1;
	auto median = results[sampleCount / 2 + 1];
	auto deb = 0;

	//TODO::: test if everyhing works !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	std::cout << "Raytracing took " << std::get<0>(median) << " seconds." << std::endl;
	std::cout << "ray sum took " << std::get<1>(median) << " seconds." << std::endl;
	std::cout << "triangleTests took " << std::get<2>(median) << " seconds." << std::endl;
	std::cout << "all rays minus triangle took " << std::get<1>(median) - std::get<2>(median) << " seconds." << std::endl;

}

template <size_t gangSize, size_t nodeMemory>
std::tuple<float, float, float> CameraFast::renderImage(bool saveImage, FastNodeManager<gangSize, nodeMemory> nodeManager,
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
					if (nodeManager.intersectSecondary(secondaryRay, timeTriangleTest))
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
			//important to override previous data
			timesTriangles[info.index] = timeTriangleTest;
			timesRay[info.index] = getTimeSpan(timeBeforeRay);
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

	double timeRaySum = std::accumulate(timesRay.begin(), timesRay.end(), 0.0);
	double timeTrianglesSum = std::accumulate(timesTriangles.begin(), timesTriangles.end(), 0.0);
	double totalTime = getTimeSpan(timeBeginRaytracer);
	if (!mute)
	{
		std::cout << "Raytracing took " << totalTime << " seconds." << std::endl;
		std::cout << "ray sum took " << timeRaySum << " seconds." << std::endl;
		std::cout << "triangleTests took " << timeTrianglesSum << " seconds." << std::endl;
		std::cout << "all rays minus triangle took " << timeRaySum - timeTrianglesSum << " seconds." << std::endl;
	}

	std::ofstream myfile(path + "/" + name + problem + problemPrefix + "_Perf.txt");
	if (myfile.is_open())
	{
		myfile << "scenario " << name << " with branching factor of " << nodeManager.branchingFactor << " and leafsize of " << nodeManager.leafSize << std::endl;
		myfile << "with a gang size of " << gangSize << ", a node meory of " << nodeMemory << ", and a leaf memory of " << nodeManager.leafMemory << std::endl;
		myfile << "Raytracer total time: " << totalTime << std::endl;
		myfile << "Time for all rays (SUM): " << timeRaySum << std::endl;
		myfile << "Time for triangle intersections (SUM): " << timeTrianglesSum << std::endl;
		myfile << "Time all rays(sum) - triangle(sum): " << timeRaySum - timeTrianglesSum << std::endl;
	}
	myfile.close();

	if (saveImage)
	{
		encodeTwoSteps(path + "/" + name + "_Perf.png", image, width, height);
	}

	return std::make_tuple(totalTime, timeRaySum, timeTrianglesSum);
}