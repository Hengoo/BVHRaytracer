#pragma once
#include "glmInclude.h"
#include "util.h"

#include <vector>

struct RenderInfo
{
	float w;
	float h;
	size_t index;

	RenderInfo(float w, float h, size_t index) : w(w), h(h), index(index)
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
	glm::vec3 position;
	glm::mat4 transform;
	std::vector<uint8_t> image;

	//contains all info needed to spawn the ray for the specific pixel. Only needed because i dont know how to get the loop index into the unsequenced for_each
	std::vector<RenderInfo> renderInfos;

	size_t height;
	size_t width;

	int workGroupSize;

	//1.37f for 40 degree, 0.866f for 60 degree (horizontal fov)
	//This focallength has most likely nothing to do with real life focal length
	float focalLength;

	Camera(std::string path, std::string name, std::string problem, int workGroupSize, glm::vec3 position, glm::vec3 lookCenter
		, glm::vec3 upward = glm::vec3(0, 1, 0), float focalLength = 0.866f, size_t height = 1088, size_t width = 1920)
		: path(path), name(name), problem(problem), workGroupSize(workGroupSize), position(position), focalLength(focalLength), height(height), width(width)
	{
		transform = glm::inverse(glm::lookAt(position, lookCenter, upward));
		//renderInfos.resize(height * width);
	}

	Camera(std::string path, std::string name, std::string problem, int workGroupSize, glm::mat4 transform,
		float focalLength = 0.866f, size_t height = 1088, size_t width = 1920)
		: path(path), name(name), problem(problem), workGroupSize(workGroupSize), transform(transform), focalLength(focalLength), height(height), width(width)
	{
		position = transform * glm::vec4(0, 0, 0, 1);
		//renderInfos.resize(height * width);
	}

protected:

	void fillRenderInfo();


	//deterministic random direction for ambient occlusion. i is the number of the current ambient ray.
	glm::vec3 getAmbientDirection(const RenderInfo& info, const glm::vec3& surfaceNormal, const int i);

	glm::vec3 sampleHemisphere(const glm::vec3& normal, const float u, const float v);

	glm::vec3 getRayTargetPosition(const RenderInfo& info);
};