#pragma once

#include <vector>
#include <iostream>
#include "../typedef.h"

class GameObject;
class Node;
class NodeAnalysis;
class Ray;

class Bvh
{
public:

	int branchingFactor;
	int leafCount;
	int bvhDepth;
	std::shared_ptr<primPointVector> primitives;

	Bvh()
	{
		//root = std::make_shared<Aabb>(0);

		//randomFillBvh();


		//naive idea: make a node with "everything" then iteratively split it up in the middle

		//Put everything inside the primitive vector, and split it with each iteration

		//DONT forget to call shrink_to_fit or to clear the primitive vector afterwards
	}

	Bvh(GameObject& gameObject);

	~Bvh()
	{
	}

	inline Node* getRoot()
	{
		return &*root;
	}

	inline NodeAnalysis* getAnalysisRoot()
	{
		return &*analysisRoot;
	}

	//calls the recusive node/ray intersect
	bool intersect(Ray& ray);

	// copy constructor -> called when an already existing object is overwritten by an other
	//Bvh(const Bvh& other) = delete;
	// copy assignment -> called when an already existing object is used to create a new object
	//Bvh& operator=(const Bvh& other) = delete;

	void recursiveOctree(const unsigned int branchingFactor, const unsigned int leafCount, int bucketCount);

	//collapses the next collapeCount child hierarchies to this node
	void collapseChilds(int collapeCount);

	void bvhAnalysis(std::string path, bool saveAndPrintResult, std::string name, std::string problem);

protected:
	std::shared_ptr<Node> root;
	std::shared_ptr<NodeAnalysis> analysisRoot;

private:

	//collapses the next collapeCount child hierarchies to this node
	void collapseChilds(std::shared_ptr<Node> node, int collapseCount);

	void iterateGo(const GameObject& go, std::shared_ptr<primPointVector>& primitives);
};
