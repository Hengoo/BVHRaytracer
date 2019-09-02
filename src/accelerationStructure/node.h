#pragma once
#include <iostream>
#include <algorithm>
#include <vector>

#include "../typedef.h"

//forward declarations:
class Primitive;
class Ray;

//node of the acceleration structure. Ray intersections are defineed in derived classes
class Node
{
public:
	Node(unsigned int depth, std::shared_ptr<primPointVector> primitives, primPointVector::iterator primitiveBegin, primPointVector::iterator primitiveEnd)
		:depth(depth), primitives(primitives), primitiveBegin(primitiveBegin), primitiveEnd(primitiveEnd)
	{
		//update bounds in derived classes
	}

	unsigned int depth;
	virtual void addNode(std::shared_ptr<Node> n);

	//primitives are given at creation and can only be removed(when they are "placed" in child nodes)
	//virtual void addPrimitive(std::shared_ptr <Primitive> p);

	virtual bool intersect(Ray& ray);
	virtual bool intersectNode(Ray& ray, float& distance) = 0;

	//might want to add tree depth here?
	virtual void recursiveOctree(const unsigned int leafCount);

	//unsigned int depth, const unsigned int branchingFactor, const unsigned int leafCount
	virtual  void recursiveBvh(const unsigned int branchingFactor, const unsigned int leafCount);

	virtual unsigned int getChildCount();
	virtual unsigned int getPrimCount();

protected:
	//this could be a unique pointer
	std::vector<std::shared_ptr<Node>> children;

	//all nodes share the same vector
	std::shared_ptr<primPointVector> primitives;
	//begin and end iterator represent the children this node contains  (what when none???)
	primPointVector::iterator primitiveBegin;
	primPointVector::iterator primitiveEnd;

private:

};
