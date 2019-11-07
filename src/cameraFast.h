#pragma once
#include "camera.h"
#include "glmInclude.h"

template <size_t gangSize, size_t nodeMemory>
class FastNodeManager;

//Fast version of camera that does no intersection counters but is only for performance analysis
class CameraFast : public Camera
{
private:
	std::vector<double> times;
	std::vector<double> times2;
public:
	//only black and white image for now ->  image is only height * width and not height * width * 4

	CameraFast(std::string path, std::string name, std::string problem, glm::vec3 position, glm::vec3 lookCenter
		, glm::vec3 upward = glm::vec3(0, 1, 0), float focalLength = 0.866f, size_t height = 1080, size_t width = 1920);


	CameraFast(std::string path, std::string name, std::string problem, glm::mat4 transform,
		float focalLength = 0.866f, size_t height = 1080, size_t width = 1920);

	template <size_t gangSize, size_t nodeMemory>
	void renderImage(bool saveImage, FastNodeManager<gangSize, nodeMemory> nodeManager, unsigned ambientSampleCount,
		float ambientDistance, bool mute);
};