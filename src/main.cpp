#pragma once
#include <iostream>

#include "accelerationStructure/aabb.h"
#include "accelerationStructure/node.h"
#include "accelerationStructure/bvh.h"
#include "camera.h"

#include "glmInclude.h"

/*
	general performance things / todo:
		check inline (there is an optimization option to inline when possible?)
		check constexpr and where it can be usefull
*/

class RayTracer
{
public:
	void run()
	{
		
	}

	RayTracer()
	{
		//IMPORTANT!!!! Bvh b = Bvh() calls the copy constructor
		//Camera c(std::make_unique<Bvh>(), glm::vec3(0,-5,0), glm::vec3(1,-1,0));
		Camera c(std::make_unique<Bvh>(), glm::vec3(0, 5, 5), glm::vec3(1, -0.5, -0.5));
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