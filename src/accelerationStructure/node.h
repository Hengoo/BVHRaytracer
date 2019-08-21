#pragma once
#include <iostream>
#include <algorithm>
#include <vector>

//forward declarations:
class Primitive;
class Ray;

//node of the acceleration structure. Ray intersections are defineed in derived classes
class Node
{
public:
	virtual void addNode(std::shared_ptr<Node> n);

	virtual void addPrimitive(std::shared_ptr<Primitive> p);

	virtual bool intersect(std::shared_ptr<Ray> ray);

	virtual void constructBvh();

	virtual int getChildCount();
	virtual int getPrimCount();

protected:
	//if i change this to array i save space IF it is mostly filled. Vector has 24 byte overhead so its not that bad
	std::vector<std::shared_ptr<Node>> children;

	//when all nodes can have primitives: nodes can have larger primitives higher in tree -> less duplicates (if a triangle is in all children nodes, have it directly here). ALso most likely less tests for shadow ray?
	std::vector<std::shared_ptr<Primitive>> primitives;
};
