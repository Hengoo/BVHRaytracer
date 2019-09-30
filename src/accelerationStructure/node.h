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
	Node(uint8_t depth, primPointVector::iterator primitiveBegin, primPointVector::iterator primitiveEnd)
		:depth(depth), primitiveBegin(primitiveBegin), primitiveEnd(primitiveEnd)
	{
		//update bounds in derived classes
	}
	uint8_t sortAxis;
	uint8_t depth;
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
	virtual  void recursiveBvh(const unsigned int branchingFactor, const unsigned int leafCount, int bucketCount);

	virtual size_t getChildCount();
	virtual size_t getPrimCount();

	inline virtual void sweepRight() = 0;
	inline virtual void sweepLeft() = 0;
	inline virtual void sweepRight(Node* n) = 0;
	inline virtual void sweepLeft(Node* n) = 0;
	inline virtual float getSurfaceArea() = 0;
	inline virtual float getVolume() = 0;

	//using sah approach from pbrt http://www.pbr-book.org/3ed-2018/Primitives_and_Intersection_Acceleration/Bounding_Volume_Hierarchies.html
	inline float sah(Node& n1, Node& n2)
	{
		return (n1.getPrimCount() * n1.getSurfaceArea() + n2.getPrimCount() * n2.getSurfaceArea()) / getSurfaceArea();
	}

	//using sah approach from pbrt http://www.pbr-book.org/3ed-2018/Primitives_and_Intersection_Acceleration/Bounding_Volume_Hierarchies.html
	//TODO: what is this cost factor sometimes used? either + const or * constant or both
	//		since we search for the min value it doesnt change anything????
	inline float sah(float invArea, int leafSize)
	{
		/*
		*overall its x * getSurfaceArea * invArea
		*the x is related to primitive count
		*c produces most compact tree.
		*intersection counts seem to depend on the scene and branching factor.
		*for sponza:	branch 4 is overall better with c
		*				branch 2 normal rays are better with a and shadowray heavily prefer c
		*					(but the counts only change for leafnodes not for overall node itnersections for b=2)
		*for shift happens:
		*				a is best for leaf intersections (and therefor even more for aabb intersections)
		*/

		//TODO: test different scenes

		//a: linear scaling: -> problem: produces half empty leaf nodes
		//return (getPrimCount() * getSurfaceArea()) * invArea;

		//b: half step, half linear
		//return ((((getPrimCount() - 1) / leafSize) + 1) * leafSize / 2.f + getPrimCount() / 2.f) * getSurfaceArea() * invArea;

		//c: step function: Mostly produces full leafs 
		return (((getPrimCount() - 1) / leafSize) + 1) * getSurfaceArea() * invArea;


	}

	//this could be a unique pointer
	std::vector<std::shared_ptr<Node>> children;

	//begin and end iterator represent the children this node contains  (what when none???)
	primPointVector::iterator primitiveBegin;
	primPointVector::iterator primitiveEnd;
protected:
private:
};
