#pragma once

#include <iostream>
// writing on a text file
#include <fstream>

#include <vector>
#include <array>

#include "node.h"
#include "nodeAnalysis.h"
#include "aabb.h"
#include "../ray.h"
#include "../primitives/sphere.h"
#include "../gameobject.h"
#include "../primitives/triangle.h"

#include "../util.h"
#include "../typedef.h"
#include "../glmInclude.h"
#include "../lodepng/lodepng.h"

#include"../mesh.h"
#include"../meshBin.h"

class Bvh
{
public:

	int branchingFactor;
	int leafCount;
	std::shared_ptr<primPointVector> primitives;

	Bvh()
	{
		//root = std::make_shared<Aabb>(0);

		//randomFillBvh();


		//naive idea: make a node with "everything" then iteratively split it up in the middle

		//Put everything inside the primitive vector, and split it with each iteration

		//DONT forget to call shrink_to_fit or to clear the primitive vector afterwards
	}

	Bvh(GameObject& gameObject)
	{
		//iterate trough gameobject root and add all triangles to the aabb
		primitives = std::make_shared<primPointVector>();
		iterateGo(gameObject, primitives);
		root = std::make_shared<Aabb>(0, primitives->begin(), primitives->end());

	}

	~Bvh()
	{
	}

	Node* getRoot()
	{
		return &*root;
	}

	NodeAnalysis* getAnalysisRoot()
	{
		return &*analysisRoot;
	}

	//calls the recusive node/ray intersect
	bool intersect(Ray& ray)
	{
		float dist = 0;
		ray.aabbIntersectionCount++;
		if (root->intersectNode(ray, dist))
		{
			ray.successfulAabbIntersectionCount++;
			return root->intersect(ray);
		}
		return false;
	}

	// copy constructor -> called when an already existing object is overwritten by an other
	//Bvh(const Bvh& other) = delete;
	// copy assignment -> called when an already existing object is used to create a new object
	//Bvh& operator=(const Bvh& other) = delete;

	void recursiveOctree(const unsigned int branchingFactor, const unsigned int leafCount, int bucketCount)
	{
		this->branchingFactor = branchingFactor;
		this->leafCount = leafCount;
		//root->recursiveOctree(leafCount);
		root->recursiveBvh(branchingFactor, leafCount, bucketCount);

		//for better performance: could go trough all nodes and recreate primitives in the order they are in the tree (also with minimal needed data)
	}

	//collapses the next collapeCount child hierarchies to this node
	void collapseChilds(int collapeCount)
	{
		std::cout << "TODO: before using collapse : fix ray intersection order so it uses the sorting" << std::endl;
		if (collapeCount > 0)
		{
			collapseChilds(root, collapeCount);
		}
	}

	void bvhAnalysis(std::string path, bool saveAndPrintResult, std::string name, std::string problem)
	{
		//analysis includes
		//metric analysis: sah, the overlapp heuristic
		//average childcount vs target
		//average primitive count vs target

		//image with 1 pixel for each leafNode -> showing depth of tree
		//image with 1 pixel for each leafnode -> fullness of tree   (might be possible to combine both)

		//create image -> go through bvh and set one pixel per node

		//position is needed to know where to place center in the image i want to draw
		int minPos = 0;
		int maxPos = 0;
		//counts: childCount[i] = number of nodes that have i children
		std::vector<unsigned int> childCount;
		std::vector<unsigned int> primCount;
		//depth of the leaf nodes
		std::vector<unsigned int> treeDepth;
		std::vector<NodeAnalysis*> leafNodes;
		analysisRoot = std::make_shared<NodeAnalysis>(&*root, branchingFactor, leafCount);
		analysisRoot->analysis(leafNodes, treeDepth, childCount, primCount);

		if (saveAndPrintResult)
		{
			int nodes = std::accumulate(childCount.begin(), childCount.end(), 0);
			int leafs = leafNodes.size();
			float averageTreeDepth = 0;
			for (size_t i = 0; i < treeDepth.size(); i++)
			{
				averageTreeDepth += treeDepth[i] * i;
			}
			averageTreeDepth /= (float)leafs;

			std::cout << "BVH Analysis:" << std::endl;
			std::cout << "Tree depth: " << treeDepth.size() - 1 << std::endl;
			std::cout << "average leaf depth: " << averageTreeDepth << std::endl;
			std::cout << "number of nodes: " << nodes - leafs << std::endl;
			std::cout << "nodes with x childen:" << std::endl;
			float sum = 0;
			float sum2 = 0;
			for (size_t i = 1; i < childCount.size(); i++)
			{
				sum += childCount[i] * i;
				sum2 += childCount[i];
			}
			//sum /= std::accumulate(childCount.begin(), childCount.end(), 0);
			sum /= sum2;
			std::cout << "average: " << std::to_string(sum) << std::endl;
			for (size_t i = 2; i < childCount.size(); i++)
			{
				std::cout << i << " : " << childCount[i] << std::endl;
			}
			std::cout << std::endl;
			std::cout << "number of leafnodes: " << leafs << std::endl;
			std::cout << "leafnodes with x primitives:" << std::endl;
			sum = 0;
			sum2 = 0;
			for (size_t i = 1; i < primCount.size(); i++)
			{
				sum += primCount[i] * i;
				sum2 += primCount[i];
			}
			//sum /= std::accumulate(primCount.begin(), primCount.end(), 0);
			sum /= sum2;
			std::cout << "average: : " << std::to_string(sum) << std::endl;
			for (size_t i = 1; i < primCount.size(); i++)
			{
				std::cout << i << " : " << primCount[i] << std::endl;
			}
			std::cout << std::endl;

			//different metrics analysis:
			float leafSah = 0, leafVolume = 0, leafSurfaceArea = 0;
			for (auto& l : leafNodes)
			{
				leafSah += l->sah;
				leafVolume += l->volume;
				leafSurfaceArea += l->surfaceArea;
			}
			leafVolume = leafVolume / analysisRoot->volume;
			leafSurfaceArea = leafSurfaceArea / analysisRoot->surfaceArea;
			//think volume and surface area need to be normalised by roof values
			std::cout << "Sah of leafs: " << std::to_string(leafSah) << " average: : " << std::to_string(leafSah / (double)leafs) << std::endl;
			std::cout << "Volume of leafs: " << std::to_string(leafVolume) << " average: : " << customToString(leafVolume / (double)leafs, 10) << std::endl;
			std::cout << "Surface area of leafs: " << std::to_string(leafSurfaceArea) << " average: : " << customToString(leafSurfaceArea / (double)leafs, 10) << std::endl;
			std::cout << std::endl;

			//write to file
			std::ofstream myfile(path + "/" + name + problem + "_BVHInfo.txt");
			if (myfile.is_open())
			{
				myfile << "scenario " << name << " with branching factor of " << std::to_string(branchingFactor) << " and leafsize of " << leafCount << std::endl;

				myfile << "BVH Analysis:" << std::endl;
				myfile << "Tree depth: " << treeDepth.size() - 1 << std::endl;
				myfile << "number of nodes: " << nodes - leafs << std::endl;
				myfile << "nodes with x childen:" << std::endl;
				float sum = 0;
				float sum2 = 0;
				for (size_t i = 1; i < childCount.size(); i++)
				{
					sum += childCount[i] * i;
					sum2 += childCount[i];
				}
				//sum /= std::accumulate(childCount.begin(), childCount.end(), 0);
				sum /= sum2;
				myfile << "average: " << std::to_string(sum) << std::endl;
				for (size_t i = 2; i < childCount.size(); i++)
				{
					myfile << i << " : " << childCount[i] << std::endl;
				}

				myfile << std::endl << "number of leafnodes: " << leafs << std::endl;
				myfile << "leafnodes with x primitives:" << std::endl;
				sum = 0;
				sum2 = 0;
				for (size_t i = 1; i < primCount.size(); i++)
				{
					sum += primCount[i] * i;
					sum2 += primCount[i];
				}
				//sum /= std::accumulate(primCount.begin(), primCount.end(), 0);
				sum /= sum2;
				myfile << "average: : " << std::to_string(sum) << std::endl;
				for (size_t i = 1; i < primCount.size(); i++)
				{
					myfile << i << " : " << primCount[i] << std::endl;
				}
				myfile << std::endl;
				myfile << "Sah of leafs: " << std::to_string(leafSah) << " average: : " << std::to_string(leafSah / (double)leafs) << std::endl;
				myfile << "Volume of leafs: " << std::to_string(leafVolume) << " average: : " << customToString(leafVolume / (double)leafs, 10) << std::endl;
				myfile << "Surface area of leafs: " << std::to_string(leafSurfaceArea) << " average: : " << customToString(leafSurfaceArea / (double)leafs, 10) << std::endl;
				myfile << std::endl;
			}
			else std::cout << "Unable to open file" << std::endl;



			//create image
			int height = treeDepth.size();
			int width = leafs;
			std::vector<unsigned char> image(height * width * 4, 255);

			int x, y;
			int id;
			float factor = 0;
			std::vector<NodeAnalysis*> parents;
			for (size_t i = 0; i < leafNodes.size(); i++)
			{
				x = i;
				leafNodes[i]->printLeaf(factor, parents, x, y);
				id = x + y * width;
				image[id * 4 + 0] = (unsigned char)(factor * 255);
				image[id * 4 + 1] = (unsigned char)(factor * 255);
				image[id * 4 + 2] = (unsigned char)(factor * 255);
				image[id * 4 + 3] = (unsigned char)255;

			}
			bool finished = false;
			std::vector<NodeAnalysis*> newParents;
			while (!finished)
			{
				finished = true;
				for (auto& p : parents)
				{
					if (p->printNode(factor, newParents, x, y))
					{
						id = x + y * width;
						image[id * 4 + 0] = (unsigned char)(factor * 255);
						image[id * 4 + 1] = (unsigned char)0;
						image[id * 4 + 2] = (unsigned char)0;
						image[id * 4 + 3] = (unsigned char)255;
					}
					else
					{
						finished = false;
					}
				}
				//TODO remove duplicats
				parents = newParents;
				newParents.clear();
			}

			//encodeTwoSteps(path + "/" + name + ".png", image, width, height);
			encodeTwoSteps(path + "/" + name + problem + "_BvhAnalysis.png", image, width, height);
		}
	}

protected:
	std::shared_ptr<Node> root;
	std::shared_ptr<NodeAnalysis> analysisRoot;

private:

	//collapses the next collapeCount child hierarchies to this node
	void collapseChilds(std::shared_ptr<Node> node, int collapseCount)
	{
		//TODO: this method assumes there are only primitives in the leaf nodes
		//(we only collapse nodes that have no primitives because otherwise it could lead to larger primcount than planned)

		//not collapse leaf nodes
		if (node->getPrimCount() != 0)
		{
			return;
		}
		std::vector<std::shared_ptr<Node>> newChildren;
		for (size_t i = 0; i < collapseCount; i++)
		{
			for (auto& child : node->children)
			{
				if (child->getPrimCount() == 0)
				{
					newChildren.insert(newChildren.end(), child->children.begin(), child->children.end());
				}
				else
				{
					newChildren.push_back(child);
				}
			}
			node->children = newChildren;
			newChildren.clear();
		}

		//this could be parrallel (i dont think its needed)
		for (auto& child : node->children)
		{
			child->depth = node->depth + 1;
			collapseChilds(child, collapseCount);
		}
	}

	void iterateGo(const GameObject& go, std::shared_ptr<primPointVector>& primitives)
	{
		//for (auto& p : (*go.mesh->vertices))
		if (go.meshBin)
		{
			for (auto& m : go.meshBin->meshes)
			{
				for (int i = 0; i < m->indices->size(); i += 3)
				{
					//triangle version:
					primitives->push_back(std::make_shared<Triangle>(&go, &*m, i));

					//sphere version (one sphere for each triangle?
					//std::array<unsigned char, 4> color = { ruchar(0, 255), ruchar(0, 255), ruchar(0, 255), 255 };
					//auto p = std::make_shared<Sphere>(go.pos, 0.5f, color);
					//root->addPrimitive(p);
				}
			}
		}
		for (auto& g : go.children)
		{
			iterateGo(*g, primitives);
		}
	}

	//debug method, just fill bvh with some stuff:
	void randomFillBvh()
	{
		//auto tmp = std::make_shared<Aabb>(glm::vec3(9, -1, -1), glm::vec3(2, 2, 2));
		//auto tmp = std::make_shared<Aabb>(glm::vec3(12, -1, -1), glm::vec3(2, 2, 2));

		//auto p = std::make_shared<Sphere>(glm::vec3(5, 0, 0), 1);
		//root->addPrimitive(p);


		srand(static_cast <unsigned> (42));

		for (int i = 0; i < 100; i++)
		{
			addRandomSphere();
		}
	}



	std::shared_ptr<Primitive> addRandomSphere()
	{
		float dist = 20;
		std::array<unsigned char, 4> color = { ruchar(0, 255), ruchar(0, 255), ruchar(0, 255), 255 };
		return std::make_shared<Sphere>(glm::vec3(rfloat(-dist, dist), rfloat(-dist, dist), rfloat(-dist, dist)), rfloat(0.2f, 3.0f), color);
	}
};
