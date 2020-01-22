#pragma once

#include "accelerationStructure/fastNodeManager.h"

#include "camera.h"
#include "glmInclude.h"
#include "timing.h"
#include "cacheSimulator.h"
#include "timing.h"
#include "util.h"
#include "fastRay.h"

#include <iostream>// writing on a text file
#include <fstream>
#include <algorithm>
#include <vector>
#include <execution>//for the parallel for
#include <numeric>// std::accumulate
#include <omp.h>//openMp


template <unsigned gangSize, unsigned nodeMemory, unsigned workGroupSize >
class FastNodeManager;

//Fast version of camera that does no intersection counters but is only for performance analysis
class CameraFast : public Camera
{
private:
	std::vector<std::chrono::nanoseconds> timesRay;
	std::vector<std::chrono::nanoseconds> timesTriangles;
	std::string problemPrefix;
	bool saveDistance;
	bool wideRender;

public:
	//only black and white image for now ->  image is only height * width and not height * width * 4

	CameraFast(std::string path, std::string name, std::string problem, std::string problemPrefix, int nonTemplateWorkGroupSize,
		bool saveDistance, bool wideRender, std::vector<glm::vec3>& positions, std::vector<glm::vec3>& lookCenters,
		size_t width = 1920, size_t height = 1088, glm::vec3 upward = glm::vec3(0, 1, 0), float focalLength = 0.866f);

	CameraFast(std::string path, std::string name, std::string problem, std::string problemPrefix, int nonTemplateWorkGroupSize,
		bool saveDistance, bool wideRender, std::vector<glm::mat4>& transforms,
		size_t width = 1920, size_t height = 1088, float focalLength = 0.866f);

	//renders 6 images, We take median of the last 5 renders
	template <unsigned gangSize, unsigned nodeMemory, unsigned workGroupSize>
	void renderImages(const bool saveImage, FastNodeManager<gangSize, nodeMemory, workGroupSize>& nodeManager, const unsigned ambientSampleCount,
		const float ambientDistance, const bool mute, const bool wideAlternative, bool saveRayTimes, bool doCacheAnalysis)
	{
		int cameraCount = positions.size();
		//all values we store:
		float totalTime = 0;
		float timeSumEachRay = 0;
		float timeSumTriangle = 0;

		//has to be an odd number for correct median
		int sampleCount = 5;
		//total raytracer time, time sum each ray, time sum triangle cost
		std::vector<std::tuple<float, float, float>> results;
		results.reserve(sampleCount);

		if (doCacheAnalysis == true)
		{
			for (int cameraId = 0; cameraId < cameraCount; cameraId++)
			{
				renderImage<gangSize, nodeMemory, workGroupSize, true>(saveImage, nodeManager, ambientSampleCount, ambientDistance, cameraId, wideAlternative, false);
				nodeManager.cache.writeAllResult();
			}
		}
		//image render:
		if (saveImage)
		{
			for (int cameraId = 0; cameraId < cameraCount; cameraId++)
			{
				//if image should be saved one first run to save the image. Just to be sure we still do one render we trough away before performance measurements
				renderImage<gangSize, nodeMemory, workGroupSize, false>(saveImage, nodeManager, ambientSampleCount, ambientDistance, cameraId, wideAlternative, false);
			}
		}

		//loop over the different camera positions
		for (int cameraId = 0; cameraId < cameraCount; cameraId++)
		{
			//"first" render to load everything in cache
			renderImage<gangSize, nodeMemory, workGroupSize, false>(false, nodeManager, ambientSampleCount, ambientDistance, cameraId, wideAlternative, false);

			//median. (median of overall time or of each individual time?
			for (int i = 0; i < sampleCount; i++)
			{
				results.push_back(renderImage<gangSize, nodeMemory, workGroupSize, false>(false, nodeManager, ambientSampleCount, ambientDistance, cameraId, wideAlternative, false));
			}
			//calculate median
			//this sorts by the first value of the tupel (the total raytracer time)
			sort(results.begin(), results.end());
			auto median = results[sampleCount / 2];
			totalTime += std::get<0>(median);
			timeSumEachRay += std::get<1>(median);
			timeSumTriangle += std::get<2>(median);

			results.clear();
		}

		//doDetailedTimings:
		if (saveRayTimes)
		{
			for (int cameraId = 0; cameraId < cameraCount; cameraId++)
			{
				renderImage<gangSize, nodeMemory, workGroupSize, false>(false, nodeManager, ambientSampleCount, ambientDistance, cameraId, wideAlternative, true);
			}
		}

		totalTime /= (float)cameraCount;
		timeSumEachRay /= (float)cameraCount;
		timeSumTriangle /= (float)cameraCount;
		//output
		if (!mute)
		{
			std::cout << "Raytracing took " << totalTime << " seconds." << std::endl;
			std::cout << "ray sum took " << timeSumEachRay << " seconds." << std::endl;
			std::cout << "triangleTests took " << timeSumTriangle << " seconds." << std::endl;
			std::cout << "all rays minus triangle took " << timeSumEachRay - timeSumTriangle << " seconds." << std::endl;
		}

		std::string workGroupName = "";
		if (wideRender)
		{
			workGroupName = "WorkGroupSize_" + std::to_string(workGroupSize) + "_Version_" + std::to_string(wideAlternative);;
		}

		std::ofstream myfile(path + "/" + workGroupName + "/" + name + problem + problemPrefix + "_Perf.txt");
		if (myfile.is_open())
		{
			myfile << "scenario " << name << " with branching factor of " << nodeManager.branchingFactor << " and leafsize of " << nodeManager.leafSize << std::endl;
			myfile << "with a gang size of " << gangSize << ", a node meory of " << nodeMemory << ", and a leaf memory of " << nodeManager.leafMemory << std::endl;
			myfile << "Raytracer total time: " << totalTime << std::endl;
			myfile << "Time for all rays (SUM): " << timeSumEachRay << std::endl;
			myfile << "Time for triangle intersections (SUM): " << timeSumTriangle << std::endl;
			myfile << "Time all rays(sum) - triangle(sum): " << timeSumEachRay - timeSumTriangle << std::endl;

			//add some extra information needed for final analysis (tri count and average tree depth)

			myfile << "Average BVH depth: " << nodeManager.averageBvhDepth << std::endl;
			myfile << "Triangle Count: " << nodeManager.triangleCount << std::endl;
		}
		else
		{
			std::cerr << "unable to open perf output file!" << std::endl;
		}
		myfile.close();
	}

	template <unsigned gangSize, unsigned nodeMemory, unsigned workGroupSize, bool doCache>
	std::tuple<float, float, float> renderImage(const bool saveImage, FastNodeManager<gangSize, nodeMemory, workGroupSize>& nodeManager,
		const unsigned ambientSampleCount, const float ambientDistance, int cameraId, bool wideAlternative, bool saveRayTimes)
	{
		//fillRenderInfo();
		auto timeBeginRaytracer = getTime();

		bool wideRefill = false;

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
#if doTimer
					auto timeBeforeRay = getTime();
#endif
					nanoSec timeTriangleTest(0);
					FastRay ray(positions[cameraId], getRayTargetPosition(info, cameraId));

					uint32_t leafIndex = 0;
					uint8_t triIndex = 0;
					//shoot primary ray.
					bool result;
					if (saveDistance)
					{
						result = nodeManager.intersectSaveDistance<doCache>(ray, leafIndex, triIndex, timeTriangleTest);
					}
					else
					{
						result = nodeManager.intersect<doCache>(ray, leafIndex, triIndex, timeTriangleTest);
					}
					uint8_t imageResult = 0;
					if (result)
					{
						//shoot secondary ray:
						unsigned ambientResult = 0;

						//get surface normal and position from triangle index info.
						glm::vec3 surfaceNormal(0);
						glm::vec3 surfacePosition(0);
						nodeManager.getSurfaceNormalPosition(ray, surfaceNormal, surfacePosition, leafIndex, triIndex);
						ray.pos = surfacePosition + surfaceNormal * 0.001f;
						//nodeManager.getSurfaceNormalTri(ray, surfaceNormal);

						for (size_t i = 0; i < ambientSampleCount; i++)
						{
							//deterministic random direction
							auto direction = getAmbientDirection(info.index, surfaceNormal, i);
							ray.updateDirection(direction);
							ray.tMax = ambientDistance;
							//shoot secondary ray
							if (nodeManager.intersectSecondary<doCache>(ray, timeTriangleTest))
							{
								ambientResult++;
							}
						}

						float factor = 1 - (ambientResult / (float)ambientSampleCount);
						imageResult = (uint8_t)(factor * 255);
					}
					//important to override previous data since we do more than one run.
					timesTriangles[info.index] = timeTriangleTest;
#if doTimer
					timesRay[info.index] = getTimeSpan(timeBeforeRay);
#endif
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
				//if (omp_get_thread_num() == 0)
				//{
				//	nodeManager.cache.writeThreadResult();
				//}
			}
		}
		else if (!wideRefill)
		{
			//ultra wide version
#pragma omp parallel for schedule(dynamic, 4)
			for (int i = 0; i < (width / workGroupSize) * (height / workGroupSize); i++)
			{
#if doTimer
				auto timeBeforeRay = getTime();
#endif
				//prepare primary ray
				std::array<FastRay, workGroupSquare> rays;
				//initialization doesnt matter since we get hit result from triIndex
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
						(tmp1 + (j % workGroupSize)), cameraId);
					rays[j] = FastRay(positions[cameraId], targetPos);
				}

				//shoot primary ray
				if (wideAlternative)
				{
					nodeManager.intersectWideAlternative<doCache>(rays, leafIndex, triIndex, timeTriangleTest);
				}
				else
				{
					nodeManager.intersectWide<doCache>(rays, leafIndex, triIndex, timeTriangleTest);
				}

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
						//no hit for primary ray, deosnt happen that often.
						rays[j].pos = glm::vec3(NAN);
						surfaceNormal[j] = glm::vec3(1.f);
					}
				}

				for (int a = 0; a < ambientSampleCount; a++)
				{
					//final preparation of secondary rays
					for (int j = 0; j < workGroupSquare; j++)
					{
						int index = i * workGroupSquare + j;
						//get deterministic random direction from ray index
						auto direction = getAmbientDirection(index, surfaceNormal[j], a);
						rays[j].updateDirection(direction);
						rays[j].tMax = ambientDistance;
					}

					//shoot secondary ray:
					if (wideAlternative)
					{
						nodeManager.intersectSecondaryWideAlternative<doCache>(rays, ambientResult, timeTriangleTest);
					}
					else
					{
						nodeManager.intersectSecondaryWide<doCache>(rays, ambientResult, timeTriangleTest);
					}
				}

				for (int j = 0; j < workGroupSquare; j++)
				{
					int index = i * workGroupSquare + j;

					float factor = 1 - (ambientResult[j] / (float)ambientSampleCount);
					//factor = (factor + 1) / 2.f;
					uint8_t imageResult = (uint8_t)(factor * 255);
					//imageResult = i;
					image[index * 4 + 0] = imageResult;
					image[index * 4 + 1] = imageResult;
					image[index * 4 + 2] = imageResult;
					image[index * 4 + 3] = 255;
				}

				//time for rays:
#if doTimer
				timesRay[i] = getTimeSpan(timeBeforeRay);
#endif
				timesTriangles[i] = timeTriangleTest;
		}
	}
		else
		{
			//first "lazy" version, work for each thread is predetermined determined by threadId and max threadCount
			int maxThreadCount = omp_get_max_threads();

			//not sure if i use openMp correctly. It might be possible that it doesnt spawn max threads?
			//in debug it also only uses thread 0...
#pragma omp parallel
			{
				//TODO: remove debug prints
				int tId = omp_get_thread_num(); //<- this implementation doesnt work in debug because tId is always  0
				int threadSum = (width / workGroupSize) * (height / workGroupSize);
				int beginId = threadSum / maxThreadCount * tId;
				int endId = threadSum / maxThreadCount * (tId + 1);
				if (tId == maxThreadCount - 1)
				{
					endId = threadSum;
				}
				int currentId = beginId;


				RefillStructure<nodeMemory, workGroupSize> dataStruct;

				int tmp0 = ((currentId * workGroupSize) / width) * workGroupSize - height / 2;
				int tmp1 = ((currentId * workGroupSize) % width - width / 2);
				for (int j = 0; j < workGroupSquare; j++)
				{
					glm::vec3 targetPos = getRayTargetPosition(
						(-tmp0 - (j / workGroupSize)),
						(tmp1 + (j % workGroupSize)), cameraId);
					dataStruct.rays[j] = FastRay(positions[cameraId], targetPos);
					dataStruct.rayId[j] = currentId * workGroupSquare + j;
				}
				dataStruct.fillRayMajorAxis();

				currentId++;
				//half tells if we already have loaded half of the current block
				uint8_t half = 0;
				bool lastRun = false;
				//primary rays:
				while (true)
				{
					//structure:
					//call traversal, traversal stops when it reaches 50% utilization -> we refill here and start next traversal

					//for refill we need to save the result of finished rays and then fill in the new rays.
					//could also reorder unfinished rays but i dont think that is necessary

					//in order to store results i need to keep track of ray id. (will do this with a separate array)

					if (currentId >= endId && !half)
					{
						lastRun = true;
					}

					nodeManager.intersectRefillWideAlternative<doCache>(dataStruct, lastRun);

					//refill dataStruct

					int tmp0 = ((currentId * workGroupSize) / width) * workGroupSize - height / 2;
					int tmp1 = ((currentId * workGroupSize) % width - width / 2);
					int fillCount = 0;
					for (int j = 0; j < workGroupSquare; j++)
					{
						if (isnan(dataStruct.rays[j].tMax))
						{
							//check if not already written
							if (image[dataStruct.rayId[j] * 4 + 0] != 0)
							{
								std::cerr << "why" << std::endl;
							}
							//read hit info
							if (dataStruct.triIndex[j] >= 0)
							{
								image[dataStruct.rayId[j] * 4 + 0] = 255 * (currentId / (float)endId);
								image[dataStruct.rayId[j] * 4 + 1] = 255 * (tId / (float)maxThreadCount);
								image[dataStruct.rayId[j] * 4 + 2] = 255;
								image[dataStruct.rayId[j] * 4 + 3] = 255;
								dataStruct.triIndex[j] = -1;
							}
							else
							{
								image[dataStruct.rayId[j] * 4 + 0] = 0;
								image[dataStruct.rayId[j] * 4 + 1] = 0;
								image[dataStruct.rayId[j] * 4 + 2] = 0;
								image[dataStruct.rayId[j] * 4 + 3] = 0;
								dataStruct.triIndex[j] = -1;
							}

							if (!lastRun)
							{
								//reset ray:
								int rayId = fillCount + half * (workGroupSquare / 2);
								glm::vec3 targetPos = getRayTargetPosition(
									(-tmp0 - (rayId / workGroupSize)),
									(tmp1 + (rayId % workGroupSize)), cameraId);
								dataStruct.rays[j] = FastRay(positions[cameraId], targetPos);
								dataStruct.rayId[j] = currentId * workGroupSquare + rayId;
								dataStruct.stack[0][j] = 0;
								dataStruct.stackIndex[j] = 1;
								(*dataStruct.currentWork)[dataStruct.nodeRays++] = j;

								fillCount++;
								if (fillCount == workGroupSquare / 2)
								{
									break;
								}
							}
						}
					}
					if (lastRun)
					{
						break;
					}

					//TODO: this recomputes the axis for all.
					dataStruct.fillRayMajorAxis();

					if (fillCount != workGroupSquare / 2)
					{
						std::cerr << "was not able to refill correct ammount of rays" << std::endl;
					}

					currentId += half;
					half = (half + 1) % 2;

				}
				if (dataStruct.nodeRays != 0)
				{
					std::cerr << "didnt finish all rays" << std::endl;
				}

				//secondary rays:
				//while (true)
				//{
				//
				//}
			}
		}
		nanoSec totalTime = getTimeSpan(timeBeginRaytracer);
		nanoSec timeRaySum = std::accumulate(timesRay.begin(), timesRay.end(), nanoSec(0));
		nanoSec timeTrianglesSum = std::accumulate(timesTriangles.begin(), timesTriangles.end(), nanoSec(0));

		//for detailed timing analysis, save times of each ray.
		if (saveRayTimes)
		{
			if (wideRender)
			{
				std::string timingFileName = path + "/" + name + "RayPerformanceWideV" + std::to_string(wideAlternative) + "_c" + std::to_string(cameraId) + ".txt";
				std::ofstream file(timingFileName);
				if (file.is_open())
				{
					file << "totalTime, nodeTime, leafTime" << std::endl;
					for (int i = 0; i < (height / nonTemplateWorkGroupSize) * (width / nonTemplateWorkGroupSize); i++)
					{
						file << timesRay[i].count() / (float)workGroupSquare << ", ";
						file << (timesRay[i] - timesTriangles[i]).count() / (float)workGroupSquare << ", ";
						file << timesTriangles[i].count() / (float)workGroupSquare << std::endl;
					}
				}
				else
				{
					std::cerr << "unable to open ray performance output file" << std::endl;
				}
			}
			else
			{
				std::string timingFileName = path + "/" + name + "RayPerformance_c" + std::to_string(cameraId) + ".txt";
				std::ofstream file(timingFileName);
				if (file.is_open())
				{
					file << "totalTime, nodeTime, leafTime" << std::endl;
					for (int i = 0; i < width * height; i++)
					{
						file << timesRay[i].count() << ", ";
						file << (timesRay[i] - timesTriangles[i]).count() << ", ";
						file << timesTriangles[i].count() << std::endl;
					}
				}
				else
				{
					std::cerr << "unable to open ray performance output file" << std::endl;
				}
			}
		}


		if (saveImage)
		{
			//reorder image before we save it.
			std::vector<uint8_t> imageCorrect;
			imageCorrect.resize(height * width * 4);

			for (int h = 0; h < height; h++)
			{
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
			}
			encodeTwoSteps(path + "/" + name + "_c" + std::to_string(cameraId) + "_Perf.png", imageCorrect, width, height);
		}
		return std::make_tuple(getTimeFloat(totalTime), getTimeFloat(timeRaySum), getTimeFloat(timeTrianglesSum));
}
};