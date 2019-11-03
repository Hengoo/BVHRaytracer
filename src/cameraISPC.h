#pragma once
#include "lights/light.h"
#include "glmInclude.h"
#include "util.h"

#include "camera.h"

#include <iostream>
// writing on a text file
#include <fstream>

#include <algorithm>
#include <vector>

//for the parallel for
#include <execution>



//Fast version of camera that does no intersection counters but is only for performance analysis
class CameraISPC : public Camera
{
public:

	//only black and white image for now ->  image is only height * width and not height * width * 4

	CameraISPC(std::string path, std::string name, std::string problem, glm::vec3 position, glm::vec3 lookCenter
		, glm::vec3 upward = glm::vec3(0, 1, 0), float focalLength = 0.866f, size_t height = 1080, size_t width = 1920)
		:Camera(path, name, problem, position, lookCenter, upward, focalLength, height, width)
	{
		image.resize(height * width);;
	}

	CameraISPC(std::string path, std::string name, std::string problem, glm::mat4 transform,
		float focalLength = 0.866f, size_t height = 1080, size_t width = 1920)
		:Camera(path, name, problem, transform, focalLength, height, width)
	{
		image.resize(height * width);
	}


};