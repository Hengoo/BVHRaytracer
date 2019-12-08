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

//macros to tell cpp what template this class is used for.

//we need all version of gangsize, branchmemory, workGroupSize
#define macro1() macro2(8) macro2(16) macro2(32)

#define macro2(wGS) macro3(4 ,wGS) macro4(8, wGS)

//macro3 does gangisze 4
#define macro3(gS, wGS) macro5(gS, 4, wGS)\
macro5(gS, 8, wGS)\
macro5(gS, 12, wGS)\
macro5(gS, 16, wGS)\
macro5(gS, 20, wGS)\
macro5(gS, 24, wGS)\
macro5(gS, 28, wGS)\
macro5(gS, 32, wGS)\
macro5(gS, 36, wGS)\
macro5(gS, 40, wGS)\
macro5(gS, 44, wGS)\
macro5(gS, 48, wGS)\
macro5(gS, 52, wGS)\
macro5(gS, 56, wGS)\
macro5(gS, 60, wGS)\
macro5(gS, 64, wGS)\

//macro4 does gangsize 8
#define macro4(gS, wGS) macro5(gS, 8, wGS)\
macro5(gS, 16, wGS)\
macro5(gS, 24, wGS)\
macro5(gS, 32, wGS)\
macro5(gS, 40, wGS)\
macro5(gS, 48, wGS)\
macro5(gS, 56, wGS)\
macro5(gS, 64, wGS)

#define macro5(gS, mS, wGS) template std::tuple<float, float, float> CameraFast::renderImage(const bool saveImage, const FastNodeManager<gS, mS, wGS>& nodeManager, const unsigned ambientSampleCount, const float ambientDistance);\
template void CameraFast::renderImages(const bool saveImage, const FastNodeManager<gS, mS, wGS>& nodeManager, const unsigned ambientSampleCount, const float ambientDistance, const bool mute);

//template class FastNodeManager<4, 4, 8>;
macro1()

CameraFast::CameraFast(std::string path, std::string name, std::string problem, std::string problemPrefix, int nonTemplateWorkGroupSize, glm::vec3 position, glm::vec3 lookCenter
	, glm::vec3 upward, float focalLength, size_t height, size_t width)
	:Camera(path, name, problem, nonTemplateWorkGroupSize, position, lookCenter, upward, focalLength, height, width), problemPrefix(problemPrefix)
{
	image.resize(height * width * 4);
	timesRay.resize(height * width);
	timesTriangles.resize(height * width);
}

CameraFast::CameraFast(std::string path, std::string name, std::string problem, std::string problemPrefix, int nonTemplateWorkGroupSize, glm::mat4 transform,
	float focalLength, size_t height, size_t width)
	:Camera(path, name, problem, nonTemplateWorkGroupSize, transform, focalLength, height, width), problemPrefix(problemPrefix)
{
	image.resize(height * width * 4);
	timesRay.resize(height * width);
	timesTriangles.resize(height * width);
}

//renders 6 images, We take median of the last 5 renders
template <unsigned gangSize, unsigned nodeMemory, unsigned workGroupSize>
void CameraFast::renderImages(const bool saveImage, const FastNodeManager<gangSize, nodeMemory, workGroupSize>& nodeManager, const unsigned ambientSampleCount,
	const float ambientDistance, const bool mute)
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

template <unsigned gangSize, unsigned nodeMemory, unsigned workGroupSize>
std::tuple<float, float, float> CameraFast::renderImage(const bool saveImage, const FastNodeManager<gangSize, nodeMemory, workGroupSize>& nodeManager,
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

#pragma omp parallel for schedule(dynamic, 4)
	for (int i = 0; i < (width / workGroupSize) * (height / workGroupSize); i++)
	{
		for (int j = 0; j < workGroupSize * workGroupSize; j++)
		{
			RenderInfo info(
				((i * workGroupSize) % width - width / 2.f) + (j % workGroupSize),
				-(((i * workGroupSize) / width) * workGroupSize - height / 2.f) - (j / workGroupSize),
				i * workGroupSize * workGroupSize + j);

			auto timeBeforeRay = getTime();
			double timeTriangleTest = 0;
			glm::vec3 pos = getRayTargetPosition(info);
			FastRay ray(position, pos - position);

			uint8_t imageResult = 0;
			uint32_t leafIndex = 0;
			uint8_t triIndex = 0;
			//shoot primary ray.
			bool result = nodeManager.intersect(ray, leafIndex, triIndex, timeTriangleTest);

			if (result)
			{
				//shoot secondary ray:
				unsigned ambientResult = 0;

				//get surface normal and position from triangle index info.
				glm::vec3 surfaceNormal(0);
				glm::vec3 surfacePosition(0);
				nodeManager.getSurfaceNormalPosition(ray, surfaceNormal, surfacePosition, leafIndex, triIndex);
				//glm::vec3 surfacePosition = ray.pos + ray.direction * (ray.tMax);
				//nodeManager.getSurfaceNormalTri(ray, surfaceNormal);
				for (size_t i = 0; i < ambientSampleCount; i++)
				{
					//deterministic random direction
					auto direction = getAmbientDirection(info, surfaceNormal, i);
					ray = FastRay(surfacePosition + surfaceNormal * 0.001f, direction, ambientDistance);

					//shoot secondary ray
					if (nodeManager.intersectSecondary(ray, timeTriangleTest))
					{
						ambientResult++;
					}
				}

				float factor = 1 - (ambientResult / (float)ambientSampleCount);
				//factor = (factor + 1) / 2.f;
				imageResult = (uint8_t)(factor * 255);
			}
			//important to override previous data since we do more than one run.
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
				int id = (w / workGroupSize) * workGroupSize * workGroupSize + w % workGroupSize + (h % workGroupSize) * workGroupSize;
				id += (h / workGroupSize) * width * workGroupSize;
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