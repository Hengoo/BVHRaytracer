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
template std::tuple<float, float, float> CameraFast::renderImage(const bool saveImage, const FastNodeManager<4, 4>& const nodeManager, const unsigned ambientSampleCount, const float ambientDistance);
template std::tuple<float, float, float> CameraFast::renderImage(const bool saveImage, const FastNodeManager<4, 8>& const nodeManager, const unsigned ambientSampleCount, const float ambientDistance);
template std::tuple<float, float, float> CameraFast::renderImage(const bool saveImage, const FastNodeManager<4, 12>& const nodeManager, const  unsigned ambientSampleCount, const float ambientDistance);
template std::tuple<float, float, float> CameraFast::renderImage(const bool saveImage, const FastNodeManager<4, 16>& const nodeManager, const  unsigned ambientSampleCount, const float ambientDistance);
template std::tuple<float, float, float> CameraFast::renderImage(const bool saveImage, const FastNodeManager<4, 20>& const nodeManager, const  unsigned ambientSampleCount, const float ambientDistance);
template std::tuple<float, float, float> CameraFast::renderImage(const bool saveImage, const FastNodeManager<4, 24>& const nodeManager, const  unsigned ambientSampleCount, const float ambientDistance);
template std::tuple<float, float, float> CameraFast::renderImage(const bool saveImage, const FastNodeManager<4, 28>& const nodeManager, const  unsigned ambientSampleCount, const float ambientDistance);
template std::tuple<float, float, float> CameraFast::renderImage(const bool saveImage, const FastNodeManager<4, 32>& const nodeManager, const  unsigned ambientSampleCount, const float ambientDistance);

template std::tuple<float, float, float>  CameraFast::renderImage(const bool saveImage, const FastNodeManager<8, 8>& nodeManager, const unsigned ambientSampleCount, const float ambientDistance);
template std::tuple<float, float, float>  CameraFast::renderImage(const bool saveImage, const FastNodeManager<8, 16>& nodeManager, const unsigned ambientSampleCount, const float ambientDistance);
template std::tuple<float, float, float>  CameraFast::renderImage(const bool saveImage, const FastNodeManager<8, 24>& nodeManager, const unsigned ambientSampleCount, const float ambientDistance);
template std::tuple<float, float, float>  CameraFast::renderImage(const bool saveImage, const FastNodeManager<8, 32>& nodeManager, const unsigned ambientSampleCount, const float ambientDistance);
template std::tuple<float, float, float>  CameraFast::renderImage(const bool saveImage, const FastNodeManager<8, 40>& nodeManager, const unsigned ambientSampleCount, const float ambientDistance);
template std::tuple<float, float, float>  CameraFast::renderImage(const bool saveImage, const FastNodeManager<8, 48>& nodeManager, const unsigned ambientSampleCount, const float ambientDistance);
template std::tuple<float, float, float>  CameraFast::renderImage(const bool saveImage, const FastNodeManager<8, 56>& nodeManager, const unsigned ambientSampleCount, const float ambientDistance);
template std::tuple<float, float, float>  CameraFast::renderImage(const bool saveImage, const FastNodeManager<8, 64>& nodeManager, const unsigned ambientSampleCount, const float ambientDistance);

template void CameraFast::renderImages(const bool saveImage, const FastNodeManager<4, 4>& nodeManager, const unsigned ambientSampleCount, const float ambientDistance, const bool mute);
template void CameraFast::renderImages(const bool saveImage, const FastNodeManager<4, 8>& nodeManager, const unsigned ambientSampleCount, const float ambientDistance, const bool mute);
template void CameraFast::renderImages(const bool saveImage, const FastNodeManager<4, 12>& nodeManager, const unsigned ambientSampleCount, const float ambientDistance, const bool mute);
template void CameraFast::renderImages(const bool saveImage, const FastNodeManager<4, 16>& nodeManager, const unsigned ambientSampleCount, const float ambientDistance, const bool mute);
template void CameraFast::renderImages(const bool saveImage, const FastNodeManager<4, 20>& nodeManager, const unsigned ambientSampleCount, const float ambientDistance, const bool mute);
template void CameraFast::renderImages(const bool saveImage, const FastNodeManager<4, 24>& nodeManager, const unsigned ambientSampleCount, const float ambientDistance, const bool mute);
template void CameraFast::renderImages(const bool saveImage, const FastNodeManager<4, 28>& nodeManager, const unsigned ambientSampleCount, const float ambientDistance, const bool mute);
template void CameraFast::renderImages(const bool saveImage, const FastNodeManager<4, 32>& nodeManager, const unsigned ambientSampleCount, const float ambientDistance, const bool mute);

template void CameraFast::renderImages(const bool saveImage, const FastNodeManager<8, 8>& nodeManager, const unsigned ambientSampleCount, const float ambientDistance, const bool mute);
template void CameraFast::renderImages(const bool saveImage, const FastNodeManager<8, 16>& nodeManager, const unsigned ambientSampleCount, const float ambientDistance, const bool mute);
template void CameraFast::renderImages(const bool saveImage, const FastNodeManager<8, 24>& nodeManager, const unsigned ambientSampleCount, const float ambientDistance, const bool mute);
template void CameraFast::renderImages(const bool saveImage, const FastNodeManager<8, 32>& nodeManager, const unsigned ambientSampleCount, const float ambientDistance, const bool mute);
template void CameraFast::renderImages(const bool saveImage, const FastNodeManager<8, 40>& nodeManager, const unsigned ambientSampleCount, const float ambientDistance, const bool mute);
template void CameraFast::renderImages(const bool saveImage, const FastNodeManager<8, 48>& nodeManager, const unsigned ambientSampleCount, const float ambientDistance, const bool mute);
template void CameraFast::renderImages(const bool saveImage, const FastNodeManager<8, 56>& nodeManager, const unsigned ambientSampleCount, const float ambientDistance, const bool mute);
template void CameraFast::renderImages(const bool saveImage, const FastNodeManager<8, 64>& nodeManager, const unsigned ambientSampleCount, const float ambientDistance, const bool mute);


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
	timesTriangles.resize(height * width);
}

template <size_t gangSize, size_t nodeMemory>
void CameraFast::renderImages(const bool saveImage, const FastNodeManager<gangSize, nodeMemory>& nodeManager,
	const unsigned ambientSampleCount, const float ambientDistance, const bool mute)
{
	//if image should be saved one first run to save the image. Just to be sure we still do one render we trough away before performance measurements
	if (saveImage)
	{
		renderImage(saveImage, nodeManager, ambientSampleCount, ambientDistance);
	}

	//"first" render to load everything in cache
	renderImage(false, nodeManager, ambientSampleCount, ambientDistance);

	//has to be an odd number for correct median
	int sampleCount = 5;
	//total raytracer time, time sum each ray, time sum triangle cost
	std::vector<std::tuple<float, float, float>> results;
	results.reserve(sampleCount);
	//median. (median of overall time or of each individual time?
	for (int i = 0; i < sampleCount; i++)
	{
		results.push_back(renderImage(false, nodeManager, ambientSampleCount, ambientDistance));
	}
	//calculate median
	//this sorts by the first value of the tupel (the total raytracer time)
	sort(results.begin(), results.end());
	auto median = results[sampleCount / 2];

	if (!mute)
	{
		bool detailed = false;
		if (!detailed)
		{
			std::cout << "Raytracing took " << std::get<0>(median) << " seconds." << std::endl;
			std::cout << "ray sum took " << std::get<1>(median) << " seconds." << std::endl;
			std::cout << "triangleTests took " << std::get<2>(median) << " seconds." << std::endl;
			std::cout << "all rays minus triangle took " << std::get<1>(median) - std::get<2>(median) << " seconds." << std::endl;
		}
		else
		{
			float totalDiff0 = 0;
			float totalDiff1 = 0;
			float totalDiff2 = 0;
			for (auto& i : results)
			{
				totalDiff0 += std::abs(std::get<0>(i) - std::get<0>(median));
				totalDiff1 += std::abs(std::get<1>(i) - std::get<1>(median));
				totalDiff2 += std::abs(std::get<2>(i) - std::get<2>(median));
			}
			std::cout << "Raytracing took " << std::get<0>(median) << " seconds. Difference between tries: " << totalDiff0 << std::endl;
			std::cout << "ray sum took " << std::get<1>(median) << " seconds. Difference between tries: " << totalDiff1 << std::endl;
			std::cout << "triangleTests took " << std::get<2>(median) << " seconds. Difference between tries: " << totalDiff2 << std::endl;
			std::cout << "all rays minus triangle took " << std::get<1>(median) - std::get<2>(median) << " seconds." << std::endl;
		}
	}

	std::ofstream myfile(path + "/" + name + problem + problemPrefix + "_Perf.txt");
	if (myfile.is_open())
	{
		myfile << "scenario " << name << " with branching factor of " << nodeManager.branchingFactor << " and leafsize of " << nodeManager.leafSize << std::endl;
		myfile << "with a gang size of " << gangSize << ", a node meory of " << nodeMemory << ", and a leaf memory of " << nodeManager.leafMemory << std::endl;
		myfile << "Raytracer total time: " << std::get<0>(median) << std::endl;
		myfile << "Time for all rays (SUM): " << std::get<1>(median) << std::endl;
		myfile << "Time for triangle intersections (SUM): " << std::get<2>(median) << std::endl;
		myfile << "Time all rays(sum) - triangle(sum): " << std::get<1>(median) - std::get<2>(median) << std::endl;

		//add some extra information needed for final analysis (tri count and average tree depth)

		myfile << "Average BVH depth: " << nodeManager.averageBvhDepth << std::endl;
		myfile << "Triangle Count: " << nodeManager.triangleCount << std::endl;
	}
	else
	{
		std::cerr << "unable to open perf output file" << std::endl;
	}
	myfile.close();
}

template <size_t gangSize, size_t nodeMemory>
std::tuple<float, float, float> CameraFast::renderImage(const bool saveImage, const FastNodeManager<gangSize, nodeMemory>& nodeManager,
	const unsigned ambientSampleCount, const float ambientDistance)
{
	//fillRenderInfo();
	auto timeBeginRaytracer = getTime();

	//similar performance compared to openMp versions, but openMp is more consistent (large difference)
	//	std::for_each(std::execution::par_unseq, renderInfos.begin(), renderInfos.end(),
	//		[&](auto& info)

	//dynamic 4 is the most consistent but the difference is not that large (test where done with the exe and windwos realtime mode, 30 samples)
	//This result is good because dynamic is the shedule that is most suited for for out problem since every ray can take differently long
//#pragma omp parallel for schedule(dynamic, 4)

	unsigned smallSize = 8;
#pragma omp parallel for schedule(dynamic, 4)
	for (int i = 0; i < (width / smallSize) * (height / smallSize); i++)
	{
		for (int j = 0; j < smallSize * smallSize; j++)
		{
			RenderInfo info(
				((i * smallSize) % width - width / 2.f) + (j % smallSize),
				-(((i * smallSize) / width) * smallSize - height / 2.f) - (j / smallSize),
				i * smallSize * smallSize + j);

			auto timeBeforeRay = getTime();
			double timeTriangleTest = 0;
			glm::vec3 pos = getRayTargetPosition(info);
			auto ray = FastRay(position, pos - position);

			uint8_t imageResult = 0;

			//shoot primary ray.
			bool result = nodeManager.intersect(ray, timeTriangleTest);

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
		}
	}
	double totalTime = getTimeSpan(timeBeginRaytracer);
	double timeRaySum = std::accumulate(timesRay.begin(), timesRay.end(), 0.0);
	double timeTrianglesSum = std::accumulate(timesTriangles.begin(), timesTriangles.end(), 0.0);

	if (saveImage)
	{
		//reorder image before we can save it.
		std::vector<uint8_t> imageCorrect;
		imageCorrect.resize(height * width * 4);

		for (int h = 0; h < height; h++)
			for (int w = 0; w < width; w++)
			{
				//one line of smallsize
				int id = (w / smallSize) * smallSize * smallSize + w % smallSize + (h % smallSize) * smallSize;
				id += (h / smallSize) * width * smallSize;
				int idOrig = w + h * width;
				imageCorrect[idOrig * 4 + 0] = image[id * 4 + 0];
				imageCorrect[idOrig * 4 + 1] = image[id * 4 + 1];
				imageCorrect[idOrig * 4 + 2] = image[id * 4 + 2];
				imageCorrect[idOrig * 4 + 3] = image[id * 4 + 3];
			}
		encodeTwoSteps(path + "/" + name + "_Perf.png", imageCorrect, width, height);
	}
	return std::make_tuple(totalTime, timeRaySum, timeTrianglesSum);
}