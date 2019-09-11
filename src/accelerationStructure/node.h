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

	//intersect calls the intersectNodes for all children and the similar method for all primitives
	virtual bool intersect(Ray& ray);
	//intersectNode is the ray collision algorithm of the specific node
	virtual bool intersectNode(Ray& ray, float& distance) = 0;

	//might want to add tree depth here?
	virtual void recursiveOctree(const unsigned int leafCount);

	//unsigned int depth, const unsigned int branchingFactor, const unsigned int leafCount
	virtual  void recursiveBvh(const unsigned int branchingFactor, const unsigned int leafCount);

	virtual unsigned int getChildCount();
	virtual unsigned int getPrimCount();
	virtual void increasePrimitives() = 0;
	virtual void decreasePrimitives() = 0;
	virtual float getSurfaceArea() = 0;

	//using sah approach from pbrt http://www.pbr-book.org/3ed-2018/Primitives_and_Intersection_Acceleration/Bounding_Volume_Hierarchies.html
	//TODO : currently also use their cost of 1/8
	inline float sah(Node& n1, Node& n2)
	{
		return 0.125 * ((n1.getPrimCount() * n1.getSurfaceArea() + n2.getPrimCount() * n2.getSurfaceArea()) / getSurfaceArea());
	}

	//this could be a unique pointer
	std::vector<std::shared_ptr<Node>> children;

protected:
	
	//all nodes share the same vector
	std::shared_ptr<primPointVector> primitives;
	//begin and end iterator represent the children this node contains  (what when none???)
	primPointVector::iterator primitiveBegin;
	primPointVector::iterator primitiveEnd;

private:

};
