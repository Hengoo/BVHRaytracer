#pragma once
#include <iostream>

#include "accelerationStructure/aabb.h"
#include "accelerationStructure/node.h"
#include "accelerationStructure/bvh.h"
#include "camera.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> //https://glm.g-truc.net/0.9.4/api/a00158.html  for make quat from a pointer to a array
#include <glm/gtx/hash.hpp>
#include <glm/gtx/matrix_decompose.hpp>


class RayTracer
{
public:
	void run()
	{
		
	}

	RayTracer()
	{
		//IMPORTANT!!!! Bvh b = Bvh() calls the copy constructor
		Camera c(std::make_unique<Bvh>(), glm::vec3(0,0,0), glm::vec3(1,0,0));
		c.renderImage();
	}

private:

};

int main()
{
	//HelloTriangleApplication app;
	RayTracer rayTracer;

	//need to check performance impact of this:
	try
	{
		rayTracer.run();
		std::cout << "end of program" << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}