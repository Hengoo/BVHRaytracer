#pragma once
#include <iostream>
#include <direct.h>
#include <vector>
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

	bool renderAnalysisImage;
	bool saveImage;
	bool saveDepthDetailedImage;
	bool bvhAnalysis;
	bool saveBvhImage;
	bool mute;

	bool doPerformanceTest;
	bool doMemoryTests;

	//true -> take new axis and sort for each split. False -> only do it once in the beginning
	bool sortEachSplit;



	//0 = bvh tree traversal, 1 = compact node, 2 = compact node immediate
	unsigned renderType;
	std::vector<unsigned> scenarios;
	unsigned bucketCount;

	//0 = custom order, 1 = level, 2 = depth first,
	unsigned compactNodeOrder;

	unsigned ambientSampleCount;

	bool castShadows;

	//both factors work, but for now i leave them 1 so we can adjust the sah cost in python 
	//(i save the leafsah and node sah seperate)
	float triangleCostFactor = 1;
	float nodeCostFactor = 1;

	int perfLoopCount = 1;


public:
	void run();
	
	//is called in the loop that iterates trough branchingfactor and leafsize
	void renderImage(unsigned branchingFactor, unsigned leafSize, unsigned gangSize, primPointVector& primitives,
		glm::vec3& cameraPos, glm::vec3& cameraTarget, std::vector<std::unique_ptr<Light>>& lights,
		std::string& name, std::string& path);
	
	void preparePrimitives(primPointVector& primitives, GameObject& root);

	void readConfig();
};