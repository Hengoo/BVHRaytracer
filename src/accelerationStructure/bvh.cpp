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
#include "../glmUtil.h"
#include "../lodepng/lodepng.h"

#include"../mesh.h"
#include"../meshBin.h"



//includes for the timer
#include <ctime>
#include <ratio>
#include <chrono>

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

void Bvh::traverseAnalysisBvh(float& epoNode, float& epoLeaf, Triangle* tri, const float& triSurfaceArea, const uint32_t primId,
	const glm::vec3& triMin, const glm::vec3& triMax, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2)
{
	std::vector<NodeAnalysis*> queue;
	queue.reserve(50);
	queue.push_back(analysisRoot.get());

	std::vector <glm::vec3> points;
	std::vector<std::pair<int, int>> lines;
	//should check if its better to not allocate worst case but average?
	//worst possible case 9 edges in the polygon
	lines.reserve(9);
	//since we dont delete points: worst case i observed was 15
	points.reserve(16);

	float area = 0;
	while (!queue.empty())
	{
		NodeAnalysis* n = queue.back();
		queue.pop_back();
		//check if triangle is part of this subtree. if yes -> continue inside
		if (n->allPrimitiveBeginId <= primId && n->allPrimitiveEndId > primId)
		{
			for (auto& c : n->children)
			{
				queue.push_back(c.get());
			}
		}
		else
		{
			//cheap aabb aabb intersection first

			if (aabbAabbIntersection(triMin, triMax, n->boundMin, n->boundMax))
			{
				//no "collision" guaranteed yet...
				
				//check if all vertices are inside aabb -> trivial surface area
				if (aabbPointIntersection(n->boundMin, n->boundMax, triMin)
					&& aabbPointIntersection(n->boundMin, n->boundMax, triMax))
				{
					//should be a rather rare case
					area = triSurfaceArea;
				}
				else
				{
					//yeay triangle clipping -> this can be really complex 
					//approach: take each line of the polygon and clip it against one of the boundaries
					//after each boundary recover the polygon by going trough the lines and filling the "hole"
					//since we have convex polygons there can always only be one missing line per boundary intersection

					points.clear();
					lines.clear();
					points.insert(points.end(), { v0,v1,v2 });
					lines.insert(lines.end(), { { 0 ,1 } ,{ 1 ,2 } ,{ 2 ,0 } });

					bool change = false;

					//clipping triangle to polygon that is inside aabb:
					for (int b = 0; b < 6; b++)
					{
						for (int i = 0; i < lines.size(); i++)
						{
							glm::vec3 point0 = points[lines[i].first];
							glm::vec3 point1 = points[lines[i].second];
							int axis = b % 3;
							int changedP = -1;
							glm::vec3 axisPosition;
							if (b < 3)
							{
								axisPosition = n->boundMin;
							}
							else
							{
								axisPosition = n->boundMax;
							}

							if (clipLine(point0, point1, changedP, axisPosition, axis, b >= 3))
							{

								if (changedP == 0)
								{
									lines[i].first = points.size();

									points.push_back(point0);
									change = true;
								}
								else if (changedP == 1)
								{
									lines[i].second = points.size();

									points.push_back(point1);
									change = true;
								}
								else
								{
									//both points inside -> do nothing
								}
							}
							else
							{
								//both points outside -> erase
								lines.erase(lines.begin() + i);
								i--;
								change = true;
							}
						}
						if (lines.size() == 1)
						{
							//rare case of triangle where 1 edge lies on boundary and both other edges are outside
							break;
						}
						if (change)
						{
							//fix the hole in the lines
							for (int lineId = 0; lineId < lines.size(); lineId++)
							{
								if (lines[lineId].second != lines[(lineId + 1) % lines.size()].first)
								{
									lines.insert(lines.begin() + lineId + 1, { lines[lineId].second , lines[(lineId + 1) % lines.size()].first });
									//per cut there can only be one hole
									break;
								}
							}
						}
						change = false;
					}
					if (lines.size() == 3)
					{
						//simple triangle area:
						area = calcTriangleSurfaceArea(points[lines[0].first], points[lines[1].first], points[lines[2].first]);
					}
					else if (lines.size() > 3)
					{
						//polygon surface:
						glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
						area = calcSurfaceAreaPolygon(points, lines, normal);
					}
					else
					{
						area = 0;
					}
				}
				//only continue when it contributes area
				if (area >= 0)
				{
					if (n->primitiveCount == 0)
					{
						epoNode += area;
					}
					else
					{
						epoLeaf += area;
					}
					for (auto& c : n->children)
					{
						queue.push_back(c.get());
					}
				}
			}
		}
	}
}

void Bvh::calcEndPointOverlap(float& nodeEpo, float& leafEpo)
{
	std::chrono::high_resolution_clock::time_point timeBegin = std::chrono::high_resolution_clock::now();
	//calculate epo:
	//i think best approach is to go torugh bvh like a ray but with the triangles (aabb)
	//then add the surface area of each triangle to each node -> need to do something smart about concurrency?

	//its <primitiveId, epoNode of this triangle, epoLeaf of this triangle, triSurface of primitive>
	std::vector<std::tuple<uint32_t, float, float, float>> workPairs(primitives->size());

	for (int i = 0; i < primitives->size(); i++)
	{
		std::get<0>(workPairs[i]) = i;
		std::get<1>(workPairs[i]) = 0;
		std::get<2>(workPairs[i]) = 0;
		std::get<3>(workPairs[i]) = 0;
	}

	//for every primitive we go trough the bvh:
	std::for_each(std::execution::par_unseq, workPairs.begin(), workPairs.end(),
		[&](auto& info)
		{
			//get bounds:
			uint32_t primId = std::get<0>(info);
			auto tri = static_cast<Triangle*>((*primitives)[primId].get());
			glm::vec3 triMin, triMax;
			tri->getBounds(triMin, triMax);

			//need length of all sides:
			float triSurfaceArea = 0;

			//extract vertex
			glm::vec3 v0, v1, v2;
			tri->getVertexPositions(v0, v1, v2);
			triSurfaceArea = calcTriangleSurfaceArea(v0, v1, v2);

			//check against tri without surface
			if (triSurfaceArea != 0)
			{
				//traverse analysisBvh
				float nodeEpoPerTri = 0, leafEpoPerTri = 0;
				/*
				std::vector <glm::vec3> points;
				std::vector<std::pair<int, int>> lines;
				//should check if its better to not allocate worst case but average?
				//worst possible case 9 edges in the polygon
				lines.reserve(9);
				//since we dont delete points: worst case i observed was 15
				points.reserve(16);*/

				traverseAnalysisBvh(nodeEpoPerTri, leafEpoPerTri, tri, triSurfaceArea,
					primId, triMin, triMax, v0, v1, v2);
				//analysisRoot->traverseAnalysisBvhRekursive(nodeEpoPerTri, leafEpoPerTri, tri, triSurfaceArea,
				//	primId, triMin, triMax, v0, v1, v2, points, lines, count1, count2, count3);
				std::get<1>(info) = nodeEpoPerTri;
				std::get<2>(info) = leafEpoPerTri;
				std::get<3>(info) = triSurfaceArea;
			}
		});
	float nodeEpoSum = 0;
	float leafEpoSum = 0;
	float totalSum = 0;
	for (auto& i : workPairs)
	{
		nodeEpoSum += std::get<1>(i);
		leafEpoSum += std::get<2>(i);
		totalSum += std::get<3>(i);

	}
	nodeEpo = nodeEpoSum / (float)totalSum;
	leafEpo = leafEpoSum / (float)totalSum;

	std::chrono::high_resolution_clock::time_point timeEnd = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> time_spanAll = std::chrono::duration_cast<std::chrono::duration<double>>(timeEnd - timeBegin);
	std::cout << "Epo took " << time_spanAll.count() << " seconds." << std::endl;
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

	float nodeEpo = 0;
	float leafEpo = 0;
	if (saveAndPrintResult)
	{
		calcEndPointOverlap(nodeEpo, leafEpo);
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
			auto tri = static_cast<Triangle*>(p);
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
			std::cout << "vertexc ount : " << vertexCount << std::endl;
			std::cout << "unique vertex count : " << uniqueVertexCount << std::endl;
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
			std::cout << "sah everything: " << std::to_string(allNodesSah) << " average: " << std::to_string(allNodesSah / (double)nodes) << std::endl;
			std::cout << "sah of nodes: " << std::to_string(allNodesSah - leafSah) << " average: " << std::to_string((allNodesSah - leafSah) / (double)(nodes - leafs)) << std::endl;
			std::cout << "sah of leafs: " << std::to_string(leafSah) << " average: " << std::to_string(leafSah / (double)leafs) << std::endl;
			std::cout << "end point overlap everything: " << std::to_string(nodeEpo + leafEpo) << " average: " << std::to_string((nodeEpo + leafEpo) / (double)(nodes)) << std::endl;
			std::cout << "end point overlap of nodes: " << std::to_string(nodeEpo) << " average: " << std::to_string(nodeEpo / (double)(nodes - leafs)) << std::endl;
			std::cout << "end point overlap of leafs: " << std::to_string(leafEpo) << " average: " << std::to_string(leafEpo / (double)leafs) << std::endl;
			std::cout << "volume of leafs: " << std::to_string(leafVolume) << " average: " << customToString(leafVolume / (double)leafs, 10) << std::endl;
			std::cout << "surface area of leafs: " << std::to_string(leafSurfaceArea) << " average: " << customToString(leafSurfaceArea / (double)leafs, 10) << std::endl;
			std::cout << std::endl;

		}
		//write to file
		std::ofstream myfile(path + "/" + name + problem + "_BVHInfo.txt");
		if (myfile.is_open())
		{
			myfile << "scenario " << name << " with branching factor of " << std::to_string(branchingFactor) << " and leafsize of " << leafSize << std::endl;

			myfile << "BVH Analysis:" << std::endl;
			myfile << "tree depth: " << treeDepth.size() - 1 << std::endl;
			myfile << "average leaf depth: " << averageTreeDepth << std::endl;
			myfile << "number of nodes: " << nodes - leafs << std::endl;
			myfile << "nodes with x childen:" << std::endl;
			myfile << std::endl;
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
			myfile << "vertex count : " << vertexCount << std::endl;
			myfile << "unique vertex count : " << uniqueVertexCount << std::endl;
			myfile << "factor : " << uniqueVertexCount / (float)vertexCount << std::endl;
			myfile << std::endl;
			myfile << "leafs with x primitives:" << std::endl;
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
			myfile << "sah everything: " << std::to_string(allNodesSah) << " average: " << std::to_string(allNodesSah / (double)nodes) << std::endl;
			myfile << "sah of node: " << std::to_string(allNodesSah - leafSah) << " average: " << std::to_string((allNodesSah - leafSah) / (nodes - (double)leafs)) << std::endl;
			myfile << "sah of leaf: " << std::to_string(leafSah) << " average: " << std::to_string(leafSah / (double)leafs) << std::endl;
			myfile << "end point overlap everything: " << std::to_string(nodeEpo + leafEpo) << " average: " << std::to_string((nodeEpo + leafEpo) / (double)(nodes)) << std::endl;
			myfile << "end point overlap of node: " << std::to_string(nodeEpo) << " average: " << std::to_string(nodeEpo / (double)(nodes - leafs)) << std::endl;
			myfile << "end point overlap of leaf: " << std::to_string(leafEpo) << " average: " << std::to_string(leafEpo / (double)leafs) << std::endl;
			myfile << "volume of leafs: " << std::to_string(leafVolume) << " average: " << customToString(leafVolume / (double)leafs, 10) << std::endl;
			myfile << "surface area of leafs: " << std::to_string(leafSurfaceArea) << " average: " << customToString(leafSurfaceArea / (double)leafs, 10) << std::endl;
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