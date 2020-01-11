#pragma once
#include "camera.h"
#include "glmInclude.h"
#include "timing.h"

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
		glm::vec3 upward = glm::vec3(0, 1, 0), float focalLength = 0.866f, size_t height = 1088, size_t width = 1920);

	CameraFast(std::string path, std::string name, std::string problem, std::string problemPrefix, int nonTemplateWorkGroupSize,
		bool saveDistance, bool wideRender, std::vector<glm::mat4>& transforms,
		float focalLength = 0.866f, size_t height = 1088, size_t width = 1920);

	//renders 6 images, We take median of the last 5 renders
	template <unsigned gangSize, unsigned nodeMemory, unsigned workGroupSize>
	void renderImages(const bool saveImage, const FastNodeManager<gangSize, nodeMemory, workGroupSize>& nodeManager, const unsigned ambientSampleCount,
		const float ambientDistance, const bool mute, const bool wideAlternative);

	template <unsigned gangSize, unsigned nodeMemory, unsigned workGroupSize>
	std::tuple<float, float, float> renderImage(const bool saveImage, const FastNodeManager<gangSize, nodeMemory, workGroupSize>& nodeManager,
		const unsigned ambientSampleCount, const float ambientDistance, int cameraId, bool wideAlternative);
};