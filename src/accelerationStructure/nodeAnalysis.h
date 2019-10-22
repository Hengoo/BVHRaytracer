#pragma once
#include <iostream>
#include <algorithm>
#include <vector>

#include "aabb.h"
#include "node.h"
#include "../color.h"
#include "../primitives/triangle.h"
#include "../glmUtil.h"

// this is to count bits in integers
#include <intrin.h>

//class is only there to analyse bvhs
//it saves additional information about nodes that are not needed for raytracing, like parent
//can be discared when bvh analysis is finshed
class  NodeAnalysis
{
	//some constants for line clipping
	const int INSIDE = 0; // 000000
	const int LEFT = 1;   // 000001
	const int RIGHT = 2;  // 000010
	const int BOTTOM = 4; // 000100
	const int TOP = 8;    // 001000
	const int FRONT = 16; // 010000
	const int BACK = 32;  // 100000

public:
	unsigned int depth;
	Node* node;
	NodeAnalysis* parent;
	std::vector<std::unique_ptr<NodeAnalysis>> children;
	unsigned int primitiveCount;

	unsigned int branchingFactor;
	unsigned int targetPrimitiveCount;
	float volume;
	float surfaceArea;
	float sah;
	//x -> -1 for non leafs, for leafs its the id
	int x;

	//node id
	int id;

	uint32_t allPrimitiveBeginId;
	uint32_t allPrimitiveEndId;

	NodeAnalysis(Node* node, int branchingFactor, int targetPrimitiveCount,
		primPointVector* primitives, float triangleCostFactor, float nodeCostFactor)
		:node(node), branchingFactor(branchingFactor), targetPrimitiveCount(targetPrimitiveCount)
	{
		parent = nullptr;
		depth = node->depth;
		primitiveCount = node->getPrimCount();
		volume = node->getVolume();
		surfaceArea = node->getSurfaceArea();
		float invSurfaceAreaRoot = 1 / surfaceArea;
		sah = 0;
		allPrimitiveBeginId = std::distance(primitives->begin(), node->allPrimitiveBegin);
		allPrimitiveEndId = std::distance(primitives->begin(), node->allPrimitiveEnd);
		for (auto& n : node->children)
		{
			children.push_back(std::make_unique<NodeAnalysis>(n.get(), this, branchingFactor, targetPrimitiveCount,
				primitives, invSurfaceAreaRoot, triangleCostFactor, nodeCostFactor));
		}
		x = -1;
	}
	NodeAnalysis(Node* node, NodeAnalysis* parent, int branchingFactor, int targetPrimitiveCount,
		primPointVector* primitives, float invSurfaceAreaRoot, float triangleCostFactor, float nodeCostFactor)
		:node(node), parent(parent), branchingFactor(branchingFactor), targetPrimitiveCount(targetPrimitiveCount)
	{
		depth = node->depth;
		primitiveCount = node->getPrimCount();
		volume = node->getVolume();
		surfaceArea = node->getSurfaceArea();

		//calc sah according to https://users.aalto.fi/~laines9/publications/aila2013hpg_paper.pdf
		//so its  surface area / surface area of root * cost of node
		//cost of node:
		//		for triangle: triCount * costFactorTri
		//		for node: costFactorNode
		if (primitiveCount == 0)
		{
			sah = surfaceArea * invSurfaceAreaRoot * nodeCostFactor;
		}
		else
		{
			sah = primitiveCount * surfaceArea * invSurfaceAreaRoot * triangleCostFactor;
		}
		allPrimitiveBeginId = std::distance(primitives->begin(), node->allPrimitiveBegin);
		allPrimitiveEndId = std::distance(primitives->begin(), node->allPrimitiveEnd);
		for (auto& n : node->children)
		{
			children.push_back(std::make_unique<NodeAnalysis>(n.get(), this, branchingFactor, targetPrimitiveCount,
				primitives, invSurfaceAreaRoot, triangleCostFactor, nodeCostFactor));
		}
		x = -1;
	}

	//traverses tree from left to right
	void analysis(std::vector<NodeAnalysis*>& leafNodes, std::vector<uint32_t>& treeDepth, std::vector<uint32_t>& childCount,
		std::vector<uint32_t>& primCount, float& allNodeSah)
	{
		size_t cc = children.size();
		size_t pc = primitiveCount;
		if (pc != 0 && cc == 0)
		{
			if (treeDepth.size() < depth + 1)
			{
				treeDepth.resize(depth + 1);
			}
			treeDepth[depth]++;
			leafNodes.push_back(this);
		}
		if (childCount.size() < cc + 1)
		{
			childCount.resize(cc + 1);
		}
		childCount[cc]++;
		if (primCount.size() < pc + 1)
		{
			primCount.resize(pc + 1);
		}
		primCount[pc]++;
		allNodeSah += sah;

		for (size_t i = 0; i < children.size(); i++)
		{
			children[i]->analysis(leafNodes, treeDepth, childCount, primCount, allNodeSah);
		}
	}

	void traverseAnalysisBvh(float& epoNode, float& epoLeaf, Triangle* tri, const glm::vec3& triMin, const glm::vec3& triMax,
		const float& triSurfaceArea, uint16_t primId, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2)
	{
		float area = 0;
		//look if tri is part of this subtree:

		if (allPrimitiveBeginId <= primId && allPrimitiveEndId > primId)
		{
			for (auto& c : children)
			{
				c->traverseAnalysisBvh(epoNode, epoLeaf, tri, triMin, triMax, triSurfaceArea, primId, v0, v1, v2);
			}
		}
		else
		{
			//cheap aabb aabb intersection first
			auto aabb = dynamic_cast<Aabb*>(node);
			bool triMinInsideAabb = aabbPointIntersection(aabb->boundMin, aabb->boundMax, triMin);
			bool triMaxInsideAabb = aabbPointIntersection(aabb->boundMin, aabb->boundMax, triMax);
			bool aabbMinInsideTri = aabbPointIntersection(triMin, triMax, aabb->boundMin);
			bool aabbMaxInsideTri = aabbPointIntersection(triMin, triMax, aabb->boundMax);


			if (triMinInsideAabb || triMaxInsideAabb || aabbMinInsideTri || aabbMaxInsideTri)
			{
				//no "collision" guaranteed yet...

				//check if all vertices are inside aabb -> trivial surface area
				if (triMinInsideAabb && triMaxInsideAabb)
				{
					//should be a rather rare case
					area += triSurfaceArea;
				}
				else
				{
					//yeay triangle clipping -> this can be really complex 
					//approach: take each line of the polygon and clip it against one of the boundaries
					//after each boundary recover the polygon by going trough the lines and filling the "hole"
					//since we have convex polygons there can always only be one missing line per boundary intersection

					std::vector <glm::vec3> points;
					std::vector<std::pair<int, int>> lines;
					//TODO: performance check if its faster when i keep sizes in average area
					//worst possible case 9 edges in the polygon
					lines.reserve(9);
					//since we dont delete points: worst case i observed was 15
					points.reserve(16);
					points.push_back(v0);
					points.push_back(v1);
					points.push_back(v2);
					lines.push_back({ 0 ,1 });
					lines.push_back({ 1 ,2 });
					lines.push_back({ 2 ,0 });
					bool change = false;

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
								axisPosition = aabb->boundMin;
							}
							else
							{
								axisPosition = aabb->boundMax;
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
							for (int lineId = 0; lineId < lines.size(); lineId++)
							{
								if (lines[lineId].second != lines[(lineId + 1) % lines.size()].first)
								{
									lines.insert(lines.begin() + lineId + 1, { lines[lineId].second , lines[(lineId + 1) % lines.size()].first });
									break;
								}
							}
						}
						change = false;
					}
					if (lines.size() == 3)
					{
						//simple triangle area:
						glm::vec3 a = points[lines[0].second] - points[lines[0].first];
						glm::vec3 b = points[lines[2].first] - points[lines[2].second];
						area += 0.5 * glm::length(glm::cross(a, b));
					}
					else if (lines.size() > 3)
					{
						//polygon surface:
						glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
						float tmp = calcSurfaceAreaPolygon(points, lines, normal);
						area += tmp;
					}
				}
			}
			if (primitiveCount == 0)
			{
				epoNode += area;
			}
			else
			{
				epoLeaf += area;
			}
			//only continue when it contributes area
			if (area >= 0)
			{
				for (auto& c : children)
				{
					c->traverseAnalysisBvh(epoNode, epoLeaf, tri, triMin, triMax, triSurfaceArea, primId, v0, v1, v2);
				}
			}
		}
	}

	float calcSurfaceAreaPolygon(const std::vector<glm::vec3>& points, const std::vector<std::pair<int, int>>& lines, const glm::vec3& normal)
	{
		//modified from http://geomalgorithms.com/a01-_area.html
		//major changes: usage of glm and its features, and different approach to point storage

		// Copyright 2000 softSurfer, 2012 Dan Sunday
		// This code may be freely used and modified for any purpose
		// providing that this copyright notice is included with it.
		// iSurfer.org makes no warranty for this code, and cannot be held
		// liable for any real or imagined damage resulting from its use.
		// Users of this code must verify correctness for their application.
		int dimensionToSkip = maxAbsDimension(normal);
		int dim0 = 0, dim1 = 0;

		if (dimensionToSkip == 0)
		{
			dim0 = 1;
			dim1 = 2;
		}
		else if (dimensionToSkip == 1)
		{
			dim0 = 2;
			dim1 = 0;
		}
		else if ((dimensionToSkip == 2))
		{
			dim0 = 0;
			dim1 = 1;
		}
		//compute area of the 2d projection

		float area = 0;
		for (int i = 1; i < lines.size(); i++)
			//area += (V[i].y * (V[j].z - V[k].z));
		{
			area += points[lines[i].first][dim0] * (points[lines[i].second][dim1] - points[lines[i - 1].first][dim1]);
		}
		//wrap arround term
		int s = lines.size();
		area += points[lines[0].first][dim0] * (points[lines[0].second][dim1] - points[lines[s - 1].first][dim1]);

		//scale area
		area *= 1 / (2 * normal[dimensionToSkip]);
		return area;
	}

	//clips line to a single axis: results are saved in p0 and p1. Smaller side: true if "smaller" side of is inside aabb
	bool clipLine(glm::vec3& p0, glm::vec3& p1, int& changedP, const glm::vec3& axisPosition, const int axis, const bool smallerSide)
	{
		//changedP: -1 = unchanged 0 = p0, 1 = p1
		bool p0Inside = false, p1Inside = false;
		bool p0Boundary = false, p1Boundary = false;
		changedP = -1;
		if (smallerSide)
		{
			p0Inside = p0[axis] <= axisPosition[axis];
			p1Inside = p1[axis] <= axisPosition[axis];
		}
		else
		{
			//greater or greater equal doesnt matter since that is an extra test case.
			p0Inside = p0[axis] > axisPosition[axis];
			p1Inside = p1[axis] > axisPosition[axis];
		}
		//This is done to avoid cuts with factor 0 or 1
		p0Boundary = p0[axis] == axisPosition[axis];
		p1Boundary = p1[axis] == axisPosition[axis];

		if ((p0Inside && p1Inside) || (p0Inside && p1Boundary) || (p0Boundary && p1Inside) || (p0Boundary && p1Boundary))
		{
			return true;
		}
		else if ((!p0Inside && !p1Inside) || (!p0Inside && p1Boundary) || (p0Boundary && !p1Inside))
		{
			return false;
		}
		else
		{
			if (p0Inside)
			{
				//cut p1 so its at boundary
				float factor = (axisPosition[axis] - p0[axis]) / (p1[axis] - p0[axis]);
				p1 = p0 + (p1 - p0) * factor;
				changedP = 1;
			}
			else
			{
				//cut p0 so its at the boundary
				float factor = (axisPosition[axis] - p0[axis]) / (p1[axis] - p0[axis]);
				p0 = p0 + (p1 - p0) * factor;
				changedP = 0;
			}
		}
		return true;
	}

	//clips line to aabb. result points are stored in p0 and p1
	bool clipLine(glm::vec3& p0, glm::vec3& p1, const glm::vec3& boundMin, const glm::vec3& boundMax)
	{
		//modified version of this algorithm https://en.wikipedia.org/wiki/Cohen%E2%80%93Sutherland_algorithm
		//major modification: works in 3d

		// compute outcodes for P0, P1, and whatever point lies outside the clip rectangle
		int outcode0 = computeOutCode(p0, boundMin, boundMax);
		int outcode1 = computeOutCode(p1, boundMin, boundMax);
		bool accept = false;

		while (true) {
			if (!(outcode0 | outcode1)) {
				// bitwise OR is 0: both points inside window; trivially accept and exit loop
				accept = true;
				break;
			}
			else if (outcode0 & outcode1) {
				// bitwise AND is not 0: both points share an outside zone (LEFT, RIGHT, TOP,
				// or BOTTOM), so both must be outside window; exit loop (accept is false)
				break;
			}
			else {
				// failed both tests, so calculate the line segment to clip
				// from an outside point to an intersection with clip edge
				float x, y, z;

				// At least one endpoint is outside the clip rectangle; pick it.
				int outcodeOut = outcode0 ? outcode0 : outcode1;

				// Now find the intersection point;
				// use formulas:
				//   slope = (y1 - y0) / (x1 - x0)
				//   x = x0 + (1 / slope) * (ym - y0), where ym is ymin or ymax
				//   y = y0 + slope * (xm - x0), where xm is xmin or xmax
				// No need to worry about divide-by-zero because, in each case, the
				// outcode bit being tested guarantees the denominator is non-zero
				if (outcodeOut & TOP) {           // point is above the clip window
					float factor = (boundMax[1] - p0[1]) / (p1[1] - p0[1]);
					x = p0[0] + (p1[0] - p0[0]) * factor;
					y = boundMax[1];
					z = p0[2] + (p1[2] - p0[2]) * factor;
				}
				else if (outcodeOut & BOTTOM) { // point is below the clip window
					float factor = (boundMin[1] - p0[1]) / (p1[1] - p0[1]);
					x = p0[0] + (p1[0] - p0[0]) * factor;
					y = boundMin[1];
					z = p0[2] + (p1[2] - p0[2]) * factor;
				}
				else if (outcodeOut & RIGHT) {  // point is to the right of clip window
					float factor = (boundMax[0] - p0[0]) / (p1[0] - p0[0]);
					x = boundMax[0];
					y = p0[1] + (p1[1] - p0[1]) * factor;
					z = p0[2] + (p1[2] - p0[2]) * factor;
				}
				else if (outcodeOut & LEFT) {   // point is to the left of clip window
					float factor = (boundMin[0] - p0[0]) / (p1[0] - p0[0]);
					x = boundMin[0];
					y = p0[1] + (p1[1] - p0[1]) * factor;
					z = p0[2] + (p1[2] - p0[2]) * factor;
				}
				else if (outcodeOut & BACK) {  // point is to the right of clip window
					float factor = (boundMax[2] - p0[2]) / (p1[2] - p0[2]);
					x = p0[0] + (p1[0] - p0[0]) * factor;
					y = p0[1] + (p1[1] - p0[1]) * factor;
					z = boundMax[2];
				}
				else if (outcodeOut & FRONT) {   // point is to the left of clip window
					float factor = (boundMin[2] - p0[2]) / (p1[2] - p0[2]);
					x = p0[0] + (p1[0] - p0[0]) * factor;
					y = p0[1] + (p1[1] - p0[1]) * factor;
					z = boundMin[2];
				}

				// Now we move outside point to intersection point to clip
				// and get ready for next pass.
				if (outcodeOut == outcode0) {
					p0[0] = x;
					p0[1] = y;
					p0[2] = z;
					outcode0 = computeOutCode(p0, boundMin, boundMax);
				}
				else {
					p1[0] = x;
					p1[1] = y;
					p1[2] = z;
					outcode1 = computeOutCode(p1, boundMin, boundMax);
				}
			}
		}
		return accept;
	}


	int computeOutCode(const glm::vec3& p, const glm::vec3& boundMin, const glm::vec3& boundMax)
	{
		//modified version of this algorithm https://en.wikipedia.org/wiki/Cohen%E2%80%93Sutherland_algorithm

		int code;

		code = INSIDE;	// initialised as being inside of [[clip window]]

		if (p[0] < boundMin[0])			// to the left of clip window
			code |= LEFT;
		else if (p[0] > boundMax[0])	// to the right of clip window
			code |= RIGHT;
		if (p[1] < boundMin[1])			// below the clip window
			code |= BOTTOM;
		else if (p[1] > boundMax[1])	// above the clip window
			code |= TOP;
		if (p[2] < boundMin[2])			// below the clip window
			code |= FRONT;
		else if (p[2] > boundMax[2])	// above the clip window
			code |= BACK;

		return code;
	}

	//used to print bvh image
	void printLeaf(float& factor, std::vector<NodeAnalysis*>& parents, int& x, int& y)
	{
		parents.push_back(parent);
		y = depth;
		this->x = x;
		factor = 1 - primitiveCount / (float)targetPrimitiveCount;
	}

	//used to print bvh image
	bool printNode(float& factor, std::vector<NodeAnalysis*>& parents, int& x, int& y)
	{
		if (depth != 0)
		{
			parents.push_back(parent);
		}
		y = depth;
		x = 0;
		for (auto& c : children)
		{
			if (c->x == -1)
			{
				return false;
			}
			x += c->x;
		}
		x = x / children.size();
		this->x = x;
		factor = children.size() / (float)branchingFactor;
		return true;
	}
};