#pragma once
#include <iostream>
#include <algorithm>
#include <vector>

#include "../typedef.h"
#include "../glmInclude.h"
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

	//std::vector<std::array<int8_t, 3>>& sortAxisEachSplit;

	//3 faces (could sort by x axis and therefore reduce to 2 for compact representation.
	std::array<std::vector<int8_t>,3> traverseOrderEachAxis;

	virtual void addNode(std::shared_ptr<Node> n);

	//primitives are given at creation and can only be removed(when they are "placed" in child nodes)
	//virtual void addPrimitive(std::shared_ptr <Primitive> p);

	//intersect calls the intersectNodes for all children and the similar method for all primitives
	virtual bool intersect(Ray& ray);
	//intersectNode is the ray collision algorithm of the specific node
	virtual bool intersectNode(Ray& ray, float& distance) = 0;

	//might want to add tree depth here?
	virtual void recursiveOctree(const unsigned leafSize);

	//unsigned int depth, const unsigned int branchingFactor, const unsigned int leafCount
	virtual void recursiveBvh(const unsigned branchingFactor, const unsigned leafSize, bool sortEachSplit, const int leafSplitOption);

	virtual size_t getChildCount();
	virtual size_t getPrimCount();

	inline virtual void sweepRight() = 0;
	inline virtual void sweepLeft() = 0;
	inline virtual void sweepRight(Node* n) = 0;
	inline virtual void sweepLeft(Node* n) = 0;
	inline virtual float getSurfaceArea() = 0;
	inline virtual float getVolume() = 0;
	inline virtual glm::vec3 getCenter() = 0;

	//using sah approach from pbrt http://www.pbr-book.org/3ed-2018/Primitives_and_Intersection_Acceleration/Bounding_Volume_Hierarchies.html
	inline float sah(Node& n1, Node& n2)
	{
		return (n1.getPrimCount() * n1.getSurfaceArea() + n2.getPrimCount() * n2.getSurfaceArea()) / getSurfaceArea();
	}

	//using sah approach from pbrt http://www.pbr-book.org/3ed-2018/Primitives_and_Intersection_Acceleration/Bounding_Volume_Hierarchies.html
	//TODO: what is this cost factor sometimes used? either + const or * constant or both
	//		since we search for the min value it doesnt change anything????
	inline float sah(const float invArea, const int leafTarget)
	{
		//inv area is 1/area of root node

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

		unsigned primCount = getPrimCount();
		if (primCount != 0)
		{
			//leafnode sah:

			//a: linear scaling: -> problem: produces half empty leaf nodes
			//return (getPrimCount() * getSurfaceArea()) * invArea;

			//b: half step, half linear
			//return ((((getPrimCount() - 1) / leafTarget) + 1) * leafTarget / 2.f + getPrimCount() / 2.f) * getSurfaceArea() * invArea;

			//c: step function: Mostly produces full leafs 
			return (((getPrimCount() - 1) / leafTarget) + 1) * getSurfaceArea() * invArea * leafTarget;
		}
		else
		{
			//currently not used:

			//node sah: 
			float factor = 1;
			return getSurfaceArea() * invArea * factor;
		}
	}

	//this could be a unique pointer
	std::vector<std::shared_ptr<Node>> children;

	//begin and end iterator represent the children this node contains  (what when none???)
	primPointVector::iterator primitiveBegin;
	primPointVector::iterator primitiveEnd;

	//both all variables give the begin and endpoint for all primitivies in child and childes child nodes
	primPointVector::iterator allPrimitiveBegin;
	primPointVector::iterator allPrimitiveEnd;
protected:
	void calculateTraverseOrderEachAxis(unsigned int branchingFactor);
private:
};
