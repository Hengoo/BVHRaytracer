#pragma once
#include <iostream>
#include <direct.h>
#include <vector>
#include <string>
#include <optional>

#include "glmInclude.h"
#include "typedef.h"


class Light;
class GameObject;

class RayTracer
{

	unsigned minLeafSize;
	unsigned maxLeafSize;
	unsigned leafStep;
	unsigned minBranch;
	unsigned maxBranch;
	unsigned branchStep;
	unsigned xRes;
	unsigned yRes;

	bool renderAnalysisImage;
	bool saveImage;
	bool saveDepthDetailedImage;
	bool bvhAnalysis;
	bool saveBvhImage;
	bool mute;
	bool doWorkGroupAnalysis;

	bool doPerformanceTest;
	bool doNodeMemoryTest;
	bool doLeafMemoryTest;
	bool saveRayTimes;
	bool doCacheAnalysis;

	//true -> take new axis and sort for each split. False -> only do it once in the beginning
	bool sortEachSplit;
	int leafSplitOption;

	//this is for the performance renderer
	bool saveDistance;
	bool wideRender;
	bool wideAlternative;
	bool renderAllOptions;

	//0 = bvh tree traversal, 1 = compact node, 2 = compact node immediate
	unsigned renderType;
	std::vector<unsigned> scenarios;

	//0 = custom order, 1 = level, 2 = depth first,
	unsigned compactNodeOrder;

	unsigned ambientSampleCount;

	bool castShadows;

	//both factors work, but for now i leave them 1 so we can adjust the sah cost in python 
	//(i save the leafsah and node sah seperate)
	float triangleCostFactor = 1;
	float nodeCostFactor = 1;

	int subdivisionStart;
	int subdivisionEnd;
	int subdivisionStep;

	int workGroupSize;

	int cacheSize;

public:
	void run();

	//is called in the loop that iterates trough branchingfactor and leafsize
	void renderImage(unsigned branchingFactor, unsigned leafSize, unsigned gangSize, primPointVector& primitives,
		std::vector<glm::vec3>& cameraPositions, std::vector<glm::vec3>& cameraTargets, std::vector<std::unique_ptr<Light>>& lights,
		std::string& name, std::string& path, std::string& pathPerf, float sahFactor);

	void preparePrimitives(primPointVector& primitives, GameObject& root, int subdivision);

	void readConfig();

	float loadSahFactor(int leafSize, int nodeSize, int stepSize);
};