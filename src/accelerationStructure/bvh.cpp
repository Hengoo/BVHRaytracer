#include "bvh.h"

#include <iostream>
// writing on a text file
#include <fstream>
#include <set>

#include "node.h"
#include "nodeAnalysis.h"

#include "aabb.h"
#include "../ray.h"
#include "../primitives/sphere.h"
#include "../gameobject.h"
#include "../primitives/triangle.h"

#include "../util.h"

#include "../glmInclude.h"
#include "../lodepng/lodepng.h"

#include"../mesh.h"
#include"../meshBin.h"

Bvh::Bvh(primPointVector primitives, const unsigned int branchingFactor, const unsigned int leafSize, bool sortEachSplit)
	:branchingFactor(branchingFactor), leafSize(leafSize), sortEachSplit(sortEachSplit)
{
	//copy primitives (needed so we dont need to recompute for each branching factor and leafsize)
	this->primitives = std::make_shared<primPointVector>(primitives);
	root = std::make_shared<Aabb>(0, this->primitives->begin(), this->primitives->end());
}

void Bvh::recursiveOctree(int bucketCount)
{
	//root->recursiveOctree(leafCount);
	root->recursiveBvh(branchingFactor, leafSize, bucketCount, sortEachSplit);
}

void Bvh::collapseChilds(int collapeCount)
{
	std::cerr << "TODO: before using collapse : fix ray intersection order so it uses the sorting" << std::endl;
	if (collapeCount > 0)
	{
		collapseChilds(root, collapeCount);
	}
}

void Bvh::collapseChilds(std::shared_ptr<Node> node, int collapseCount)
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

bool Bvh::intersect(Ray& ray)
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

float Bvh::calcEndPointOverlap()
{
	//calculate epo:
	//i think best approach is to go torugh bvh like a ray but with the triangles (aabb)
	//then add the surface area of each triangle to each node -> need to do something smart about concurrency?

	//its <primitiveId, epo of this triangle, triSurface of primitive>
	std::vector<std::tuple<uint32_t, float, float>> workPairs(primitives->size());

	for (int i = 0; i < primitives->size(); i++)
	{
		std::get<0>(workPairs[i]) = i;
		std::get<1>(workPairs[i]) = 0;
		std::get<2>(workPairs[i]) = 0;
	}

	//for every primitive we go trough the bvh:
	std::for_each(std::execution::par_unseq, workPairs.begin(), workPairs.end(),
		[&](auto& info)
		{
			//get bounds:
			uint32_t primId = std::get<0>(info);
			auto tri = dynamic_cast<Triangle*>((*primitives)[primId].get());
			glm::vec3 triMin, triMax;
			tri->getBounds(triMin, triMax);

			//need length of all sides:
			float a = 0, b = 0, c = 0, triSurfaceArea = 0;

			glm::vec3 v0, v1, v2;
			tri->getVertexPositions(v0, v1, v2);
			a = glm::distance(v0, v1);
			b = glm::distance(v1, v2);
			c = glm::distance(v2, v0);

			float s = (a + b + c) / 2;
			triSurfaceArea += sqrt(s * (s - a) * (s - b) * (s - c));

			//traverse analysisBvh
			std::get<1>(info) = analysisRoot->traverseAnalysisBvh(tri, triMin, triMax, triSurfaceArea, primId);
			std::get<2>(info) = triSurfaceArea;
		});
	float overlapSum = 0;
	float totalSum = 0;
	for (auto& i : workPairs)
	{
		overlapSum += std::get<1>(i);
		totalSum += std::get<2>(i);
	}
	//TODO: Cn (cost factor !!!!!!!!!!!!!!!!)
	return overlapSum / totalSum;
}

void Bvh::bvhAnalysis(std::string path, bool saveAndPrintResult, bool saveBvhImage, std::string name,
	std::string problem, float triangleCostFactor, float nodeCostFactor, bool mute)
{
	//analysis includes
	//metric analysis: sah, the overlapp heuristic
	//average childcount vs target
	//average primitive count vs target

	//image with 1 pixel for each leafNode -> showing depth of tree
	//image with 1 pixel for each leafnode -> fullness of tree   (might be possible to combine both)

	//create image -> go through bvh and set one pixel per node

	//counts: childCount[i] = number of nodes that have i children
	std::vector<uint32_t> childCount;
	std::vector<uint32_t> primCount;
	//depth of the leaf nodes
	std::vector<uint32_t> treeDepth;
	std::vector<NodeAnalysis*> leafNodes;
	float allNodesSah = 0;

	analysisRoot = std::make_shared<NodeAnalysis>(root.get(), branchingFactor, leafSize,
		primitives.get(), triangleCostFactor, nodeCostFactor);

	analysisRoot->analysis(leafNodes, treeDepth, childCount, primCount, allNodesSah);
	bvhDepth = treeDepth.size();
	//allNodesSah /= analysisRoot->surfaceArea;

	uint32_t nodes = std::accumulate(childCount.begin(), childCount.end(), 0);
	uint32_t leafs = leafNodes.size();

	float epo = 0;
	if (saveAndPrintResult)
	{
		epo = calcEndPointOverlap();
	}

	if (saveAndPrintResult)
	{
		uint32_t vertexCount = 0;
		uint32_t uniqueVertexCount = 0;
		//analyse vertexcount vs unique vertex count -> triangleFan ec?

		std::set<uint32_t> trianglePrimitiveIds;
		//different metrics analysis:
		float leafSah = 0, leafVolume = 0, leafSurfaceArea = 0, averageTreeDepth = 0;
		for (auto& l : leafNodes)
		{
			leafSah += l->sah;
			leafVolume += l->volume;
			leafSurfaceArea += l->surfaceArea;
			averageTreeDepth += l->depth;

			vertexCount += l->primitiveCount * 3;

			//Triangle* tri = static_cast<Triangle*>((*l->node->primitiveBegin).get());
			//primitiveIds.push_back(tri->index)

			std::for_each(l->node->primitiveBegin, l->node->primitiveEnd,
				[&](auto& prim)
				{
					Triangle* tri = static_cast<Triangle*>(prim.get());
					uint32_t vertex0, vertex1, vertex2;
					tri->getVertexIds(vertex0, vertex1, vertex2);
					auto result = trianglePrimitiveIds.insert(vertex0);
					if (result.second)
					{
						uniqueVertexCount += 1;
					}
					result = trianglePrimitiveIds.insert(vertex1);
					if (result.second)
					{
						uniqueVertexCount += 1;
					}
					result = trianglePrimitiveIds.insert(vertex2);
					if (result.second)
					{
						uniqueVertexCount += 1;
					}
				});
			trianglePrimitiveIds.clear();
		}
		//could "normalise" those by the root node
		//leafVolume = leafVolume / analysisRoot->volume;
		//leafSurfaceArea = leafSurfaceArea / analysisRoot->surfaceArea;
		averageTreeDepth /= (float)leafs;

		//make checksum / hash of vector to compare leafnodes of different bvhs
		size_t seed = leafNodes.size();
		for (auto& i : leafNodes)
		{

			auto p = (*i->node->primitiveBegin).get();
			auto tri = dynamic_cast<Triangle*>(p);
			uint32_t a, b, c;
			tri->getVertexIds(a, b, c);
			if (std::distance(primitives->begin(), i->node->primitiveBegin) < 100)
			{
				//std::cerr << std::distance(primitives->begin(), i->node->primitiveBegin) << "  " << a << std::endl;
			}
			seed ^= a + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= b + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= c + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
		//std::cerr << "branch" << branchingFactor << ": " << seed << std::endl;

		if (!mute)
		{
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
			std::cout << "leafnode checksum: " << seed << std::endl;
			std::cout << "vertexCount : " << vertexCount << std::endl;
			std::cout << "uniqueVertexCount : " << uniqueVertexCount << std::endl;
			std::cout << "factor : " << uniqueVertexCount / (float)vertexCount << std::endl;
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

			//think volume and surface area need to be normalised by roof values
			std::cout << "Sah everything: " << std::to_string(allNodesSah) << " average: " << std::to_string(allNodesSah / (double)nodes) << std::endl;
			std::cout << "Sah of Nodes: " << std::to_string(allNodesSah - leafSah) << " average: " << std::to_string((allNodesSah - leafSah) / (nodes - (double)leafs)) << std::endl;
			std::cout << "Sah of leafs: " << std::to_string(leafSah) << " average: " << std::to_string(leafSah / (double)leafs) << std::endl;
			std::cout << "End-Point Overlap: " << std::to_string(epo) << " average: " << std::to_string(epo / (double)nodes) << std::endl;
			std::cout << "Volume of leafs: " << std::to_string(leafVolume) << " average: " << customToString(leafVolume / (double)leafs, 10) << std::endl;
			std::cout << "Surface area of leafs: " << std::to_string(leafSurfaceArea) << " average: " << customToString(leafSurfaceArea / (double)leafs, 10) << std::endl;
			std::cout << std::endl;

		}
		//write to file
		std::ofstream myfile(path + "/" + name + problem + "_BVHInfo.txt");
		if (myfile.is_open())
		{
			myfile << "scenario " << name << " with branching factor of " << std::to_string(branchingFactor) << " and leafsize of " << leafSize << std::endl;

			myfile << "BVH Analysis:" << std::endl;
			myfile << "Tree depth: " << treeDepth.size() - 1 << std::endl;
			myfile << "average leaf depth: " << averageTreeDepth << std::endl;
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
			myfile << "leafnode checksum: " << seed << std::endl;
			myfile << "vertexCount : " << vertexCount << std::endl;
			myfile << "uniqueVertexCount : " << uniqueVertexCount << std::endl;
			myfile << "factor : " << uniqueVertexCount / (float)vertexCount << std::endl;
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
			myfile << "Sah everything: " << std::to_string(allNodesSah) << " average: " << std::to_string(allNodesSah / (double)nodes) << std::endl;
			myfile << "Sah of Nodes: " << std::to_string(allNodesSah - leafSah) << " average: " << std::to_string((allNodesSah - leafSah) / (nodes - (double)leafs)) << std::endl;
			myfile << "Sah of leafs: " << std::to_string(leafSah) << " average: " << std::to_string(leafSah / (double)leafs) << std::endl;
			myfile << "End-Point Overlap: " << std::to_string(epo) << " average: " << std::to_string(epo / (double)nodes) << std::endl;
			myfile << "Volume of leafs: " << std::to_string(leafVolume) << " average: " << customToString(leafVolume / (double)leafs, 10) << std::endl;
			myfile << "Surface area of leafs: " << std::to_string(leafSurfaceArea) << " average: " << customToString(leafSurfaceArea / (double)leafs, 10) << std::endl;
			myfile << std::endl;
		}
		else std::cerr << "Unable to open file" << std::endl;
	}

	//create image
	if (saveBvhImage && leafs)
	{
		int height = treeDepth.size();
		int width = leafs;
		std::vector<uint8_t> image(height * width * 4, 255);

		int x, y;
		int id;
		float factor = 0;
		std::vector<NodeAnalysis*> parents;
		for (size_t i = 0; i < leafNodes.size(); i++)
		{
			x = i;
			leafNodes[i]->printLeaf(factor, parents, x, y);
			id = x + y * width;
			image[id * 4 + 0] = (uint8_t)(factor * 255);
			image[id * 4 + 1] = (uint8_t)(factor * 255);
			image[id * 4 + 2] = (uint8_t)(factor * 255);
			image[id * 4 + 3] = (uint8_t)255;

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
					image[id * 4 + 0] = (uint8_t)(factor * 255);
					image[id * 4 + 1] = (uint8_t)0;
					image[id * 4 + 2] = (uint8_t)0;
					image[id * 4 + 3] = (uint8_t)255;
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