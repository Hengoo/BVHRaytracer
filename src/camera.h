#pragma once
#include "glmInclude.h"
#include "util.h"

#include <vector>

struct RenderInfo
{
	int w;
	int h;
	size_t index;

	RenderInfo(int w, int h, size_t index) : w(w), h(h), index(index)
	{
	}
	RenderInfo() : w(0), h(0), index(0)
	{
	}
};

//Fast version of camera that does no intersection counters but is only for performance analysis
class Camera
{
public:
	//path to write the analysis results
	std::string path;
	//model / scenario name
	std::string name;
	//description for info.txt
	std::string problem;
	std::vector<glm::vec3> positions;
	std::vector<glm::mat4> transforms;
	std::vector<uint8_t> image;

	//contains all info needed to spawn the ray for the specific pixel. Only needed because i dont know how to get the loop index into the unsequenced for_each
	std::vector<RenderInfo> renderInfos;

	int height;
	int width;

	//not optimal, but for most parts i dont need template workGroup size.
	int nonTemplateWorkGroupSize;

	//1.37f for 40 degree, 0.866f for 60 degree (horizontal fov)
	//This focallength has most likely nothing to do with real life focal length
	float focalLength;

	Camera(std::string path, std::string name, std::string problem, int nonTemplateWorkGroupSize, std::vector<glm::vec3>& positions,
		std::vector<glm::vec3>& lookCenters, glm::vec3 upward = glm::vec3(0, 1, 0), float focalLength = 0.866f, size_t height = 1088, size_t width = 1920)
		: path(path), name(name), problem(problem), nonTemplateWorkGroupSize(nonTemplateWorkGroupSize), positions(positions), focalLength(focalLength), height(height), width(width)
	{
		transforms.reserve(positions.size());
		for (int i = 0; i < positions.size(); i++)
		{
			transforms.push_back(glm::inverse(glm::lookAt(positions[i], lookCenters[i], upward)));
		}
		//renderInfos.resize(height * width);
	}

	Camera(std::string path, std::string name, std::string problem, int nonTemplateWorkGroupSize,
		std::vector<glm::mat4>& transforms, float focalLength = 0.866f, size_t height = 1088, size_t width = 1920)
		: path(path), name(name), problem(problem), nonTemplateWorkGroupSize(nonTemplateWorkGroupSize), transforms(transforms), focalLength(focalLength), height(height), width(width)
	{
		positions.reserve(transforms.size());
		for (int i = 0; i < transforms.size(); i++)
		{
			positions.push_back(transforms[i] * glm::vec4(0, 0, 0, 1));
		}
		//renderInfos.resize(height * width);
	}

protected:

	void fillRenderInfo();


	//deterministic random direction for ambient occlusion. i is the number of the current ambient ray.
	glm::vec3 getAmbientDirection(const int index, const glm::vec3& surfaceNormal, const int i);

	glm::vec3 sampleHemisphere(const glm::vec3& normal, const float u, const float v);

	inline glm::vec3 getRayTargetPosition(const RenderInfo& info, int id)
	{
		glm::vec4 centerOffset = (glm::vec4(0, 1, 0, 0) * (float)info.h + glm::vec4(1, 0, 0, 0) * (float)info.w) * (1.0f / width) + glm::vec4(0, 0, -focalLength, 0);
		//next line to get perfect forward ray
		//centerOffset = glm::vec4(0, 0, -1, 0);
		glm::vec3 pos = positions[id] + glm::vec3(transforms[id] * centerOffset);
		return pos;
	}

	inline glm::vec3 getRayTargetPosition(const int h, const int w, int id)
	{
		glm::vec4 centerOffset = (glm::vec4(0, 1, 0, 0) * (float)h + glm::vec4(1, 0, 0, 0) * (float)w) * (1.0f / width) + glm::vec4(0, 0, -focalLength, 0);
		//next line to get perfect forward ray
		//centerOffset = glm::vec4(0, 0, -1, 0);
		glm::vec3 pos = positions[id] + glm::vec3(transforms[id] * centerOffset);
		return pos;
	}
};