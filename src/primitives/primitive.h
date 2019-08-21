#pragma once
#include <iostream>
#include <vector>

//forward declaration
class Ray;
class Node;
#include "../glmInclude.h"

class Primitive
{
public:

	//check ray - primitive intersection
	virtual bool intersect(std::shared_ptr<Ray> ray) = 0;

	
	/*
	-there can be different node types (i want to experiment with aabb, rotated boxes, but also with spheres
	-there can be different primitive types: triangles, spheres, boxes, ?quads?, ez
	--> every primitive needs the intersection code to work for every node type
	*/

	//check node - primitive intersection
	virtual bool intersect(std::shared_ptr<Node> node) = 0;

	//returns the min and max bounds of the primitive
	virtual void getBounds(glm::vec3& min, glm::vec3& max) = 0;
};