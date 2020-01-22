#include "cameraFast.h"

CameraFast::CameraFast(std::string path, std::string name, std::string problem, std::string problemPrefix, int nonTemplateWorkGroupSize,
	bool saveDistance, bool wideRender, std::vector<glm::vec3>& positions, std::vector<glm::vec3>& lookCenters, size_t width, size_t height, glm::vec3 upward, float focalLength)
	:Camera(path, name, problem, nonTemplateWorkGroupSize, positions, lookCenters, width, height, upward, focalLength), problemPrefix(problemPrefix), saveDistance(saveDistance), wideRender(wideRender)
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

CameraFast::CameraFast(std::string path, std::string name, std::string problem, std::string problemPrefix, int nonTemplateWorkGroupSize,
	bool saveDistance, bool wideRender, std::vector<glm::mat4>& transforms, size_t width, size_t height, float focalLength)
	:Camera(path, name, problem, nonTemplateWorkGroupSize, transforms, width, height, focalLength), problemPrefix(problemPrefix), saveDistance(saveDistance), wideRender(wideRender)
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