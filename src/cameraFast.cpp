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
//macro5(gS, 20, wGS)\
//macro5(gS, 24, wGS)\
//macro5(gS, 28, wGS)\
//macro5(gS, 32, wGS)\
//macro5(gS, 36, wGS)\
//macro5(gS, 40, wGS)\
//macro5(gS, 44, wGS)\
//macro5(gS, 48, wGS)\
//macro5(gS, 52, wGS)\
//macro5(gS, 56, wGS)\
//macro5(gS, 60, wGS)\
//macro5(gS, 64, wGS)\

//macro4 does gangsize 8
#define macro4(gS, wGS) macro5(gS, 8, wGS)\
macro5(gS, 16, wGS)\
//macro5(gS, 24, wGS)\
//macro5(gS, 32, wGS)\
//macro5(gS, 40, wGS)\
//macro5(gS, 48, wGS)\
//macro5(gS, 56, wGS)\
//macro5(gS, 64, wGS)

#define macro5(gS, mS, wGS) template std::tuple<float, float, float> CameraFast::renderImage(const bool saveImage, const FastNodeManager<gS, mS, wGS>& nodeManager, const unsigned ambientSampleCount, const float ambientDistance);\
template void CameraFast::renderImages(const bool saveImage, const FastNodeManager<gS, mS, wGS>& nodeManager, const unsigned ambientSampleCount, const float ambientDistance, const bool mute);

//template class FastNodeManager<4, 4, 8>;
macro1()

CameraFast::CameraFast(std::string path, std::string name, std::string problem, std::string problemPrefix, int nonTemplateWorkGroupSize, bool saveDistance, bool wideRender, glm::vec3 position, glm::vec3 lookCenter
	, glm::vec3 upward, float focalLength, size_t height, size_t width)
	:Camera(path, name, problem, nonTemplateWorkGroupSize, position, lookCenter, upward, focalLength, height, width), problemPrefix(problemPrefix), saveDistance(saveDistance), wideRender(wideRender)
{
	image.resize(height * width * 4);
	if (wideRender)
	{
		int timeSize = (height / nonTemplateWorkGroupSize) * (width / nonTemplateWorkGroupSize);
		timesRay.resize(timeSize);
		timesTriangles.resize(timeSize);
	}
	else
	{
		timesRay.resize(height * width);
		timesTriangles.resize(height * width);
	}
}

CameraFast::CameraFast(std::string path, std::string name, std::string problem, std::string problemPrefix, int nonTemplateWorkGroupSize, bool saveDistance, bool wideRender, glm::mat4 transform,
	float focalLength, size_t height, size_t width)
	:Camera(path, name, problem, nonTemplateWorkGroupSize, transform, focalLength, height, width), problemPrefix(problemPrefix), saveDistance(saveDistance), wideRender(wideRender)
{
	image.resize(height * width * 4);
	if (wideRender)
	{
		int timeSize = (height / nonTemplateWorkGroupSize) * (width / nonTemplateWorkGroupSize);
		timesRay.resize(timeSize);
		timesTriangles.resize(timeSize);
	}
	else
	{
		timesRay.resize(height * width);
		timesTriangles.resize(height * width);
	}
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
	for (int i = 0; i < 1; i++)
	{
		renderImage(false, nodeManager, ambientSampleCount, ambientDistance);
	}

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

	if (!wideRender)
	{
#pragma omp parallel for schedule(dynamic, 4)
		for (int i = 0; i < (width / workGroupSize) * (height / workGroupSize); i++)
		{
			for (int j = 0; j < workGroupSquare; j++)
			{
				RenderInfo info(
					((i * workGroupSize) % width - width / 2.f) + (j % workGroupSize),
					-(((i * workGroupSize) / width) * workGroupSize - height / 2.f) - (j / workGroupSize),
					i * workGroupSquare + j);

				auto timeBeforeRay = getTime();
				nanoSec timeTriangleTest(0);
				FastRay ray(position, getRayTargetPosition(info));

				uint8_t imageResult = 0;
				uint32_t leafIndex = 0;
				uint8_t triIndex = 0;
				//shoot primary ray.
				bool result;
				if (saveDistance)
				{
					result = nodeManager.intersectSaveDistance(ray, leafIndex, triIndex, timeTriangleTest);
				}
				else
				{
					result = nodeManager.intersect(ray, leafIndex, triIndex, timeTriangleTest);
				}

				if (result)
				{
					//shoot secondary ray:
					unsigned ambientResult = 0;

					//get surface normal and position from triangle index info.
					glm::vec3 surfaceNormal(0);
					glm::vec3 surfacePosition(0);
					nodeManager.getSurfaceNormalPosition(ray, surfaceNormal, surfacePosition, leafIndex, triIndex);
					ray.pos = surfacePosition + surfaceNormal * 0.001f;
					//glm::vec3 surfacePosition = ray.pos + ray.direction * (ray.tMax);
					//nodeManager.getSurfaceNormalTri(ray, surfaceNormal);
					for (size_t i = 0; i < ambientSampleCount; i++)
					{
						//deterministic random direction
						auto direction = getAmbientDirection(info.index, surfaceNormal, i);
						ray.updateDirection(direction);
						ray.tMax = ambientDistance;
						//shoot secondary ray
						if (nodeManager.intersectSecondary(ray, timeTriangleTest))
						{
							ambientResult++;
						}
					}

					float factor = 1 - (ambientResult / (float)ambientSampleCount);
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
	}
	else
	{
		//ultra wide version
#pragma omp parallel for schedule(dynamic, 4)
		for (int i = 0; i < (width / workGroupSize) * (height / workGroupSize); i++)
		{
			auto timeBeforeRay = getTime();

			//prepare primary ray
			std::array<FastRay, workGroupSquare > rays;
			//initialization doesnt matter since we get bool result;
			std::array<uint32_t, workGroupSquare> leafIndex;
			//leafIndex.fill(0);
			//hit is encoded in triIndex (when its not -1)
			std::array<int8_t, workGroupSquare> triIndex;
			triIndex.fill(-1);
			nanoSec timeTriangleTest(0);

			int tmp0 = ((i * workGroupSize) / width) * workGroupSize - height / 2;
			int tmp1 = ((i * workGroupSize) % width - width / 2);
			for (int j = 0; j < workGroupSquare; j++)
			{
				glm::vec3 targetPos = getRayTargetPosition(
					(-tmp0 - (j / workGroupSize)),
					(tmp1 + (j % workGroupSize)));
				rays[j] = FastRay(position, targetPos);
			}

			//shoot primary ray
			nodeManager.intersectWide(rays, leafIndex, triIndex, timeTriangleTest);

			//prepare secondary rays:
			std::array<uint8_t, workGroupSquare> ambientResult;
			ambientResult.fill(0);
			std::array<glm::vec3, workGroupSquare> surfaceNormal;
			//surfaceNormal.fill(glm::vec3(0));

			for (int j = 0; j < workGroupSquare; j++)
			{
				if (triIndex[j] != -1)
				{
					//get surface normal and position from triangle index info.
					glm::vec3 surfacePosition(0);
					nodeManager.getSurfaceNormalPosition(rays[j], surfaceNormal[j], surfacePosition, leafIndex[j], triIndex[j]);
					//this way of "fixing" the position allows us to keep it for different ambient rays
					rays[j].pos = surfacePosition + surfaceNormal[j] * 0.001f;
				}
				else
				{
					//no primary hit -> in the end it will spawn a ray with
					//spawn position is outside of what i would expect as a scene so it never hits anything
					rays[j].pos = glm::vec3(1000000000.f);
					surfaceNormal[j] = glm::vec3(1.f);
				}
			}

			for (int a = 0; a < ambientSampleCount; a++)
			{
				//final prepare of rays
				for (int j = 0; j < workGroupSquare; j++)
				{
					int index = i * workGroupSquare + j;
					//get surface normal and position from triangle index info.
					//looks cool if wrong
					//auto direction = getAmbientDirection(index, surfaceNormal[j], i);
					auto direction = getAmbientDirection(index, surfaceNormal[j], a);
					rays[j].updateDirection(direction);
					rays[j].tMax = ambientDistance;
				}

				//shoot secondary ray:
				nodeManager.intersectSecondaryWide(rays, ambientResult, timeTriangleTest);
			}
			for (int j = 0; j < workGroupSquare; j++)
			{
				int index = i * workGroupSquare + j;

				float factor = 1 - (ambientResult[j] / (float)ambientSampleCount);
				//factor = (factor + 1) / 2.f;
				uint8_t imageResult = (uint8_t)(factor * 255);
				image[index * 4 + 0] = imageResult;
				image[index * 4 + 1] = imageResult;
				image[index * 4 + 2] = imageResult;
				image[index * 4 + 3] = 255;
			}

			//time for rays:
			timesRay[i] = getTimeSpan(timeBeforeRay);
			timesTriangles[i] = timeTriangleTest;
		}
	}
	nanoSec totalTime = getTimeSpan(timeBeginRaytracer);
	nanoSec timeRaySum = std::accumulate(timesRay.begin(), timesRay.end(), nanoSec(0));
	nanoSec timeTrianglesSum = std::accumulate(timesTriangles.begin(), timesTriangles.end(), nanoSec(0));

	if (saveImage)
	{
		//reorder image before we can save it.
		std::vector<uint8_t> imageCorrect;
		imageCorrect.resize(height * width * 4);

		for (int h = 0; h < height; h++)
			for (int w = 0; w < width; w++)
			{
				//one line of smallsize
				int id = (w / workGroupSize) * workGroupSquare + w % workGroupSize + (h % workGroupSize) * workGroupSize;
				id += (h / workGroupSize) * width * workGroupSize;
				int idOrig = w + h * width;
				imageCorrect[idOrig * 4 + 0] = image[id * 4 + 0];
				imageCorrect[idOrig * 4 + 1] = image[id * 4 + 1];
				imageCorrect[idOrig * 4 + 2] = image[id * 4 + 2];
				imageCorrect[idOrig * 4 + 3] = image[id * 4 + 3];
			}
		encodeTwoSteps(path + "/" + name + "_Perf.png", imageCorrect, width, height);
	}
	return std::make_tuple(getTimeFloat(totalTime), getTimeFloat(timeRaySum), getTimeFloat(timeTrianglesSum));
}