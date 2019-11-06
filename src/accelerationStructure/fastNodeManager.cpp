#include "fastNodeManager.h"
#include "nodeAnalysis.h"
#include "..\primitives\triangle.h"
#include "..\util.h"
#include <algorithm>

#include "..\timing.h"

FastNodeManager::FastNodeManager(Bvh& bvh)
{
	//very similar to compactNodeManager

	//general appraoch: save all analysisNodes in a vector -> then write the id (the position in the vector)
	//Then we now the ids of all nodes and the ids of those children -> write them into compactNodesVector

	//tmp node vector
	std::vector<NodeAnalysis*> nodeVector;
	nodeVector.push_back(bvh.getAnalysisRoot());
	customTreeOrder(bvh.getAnalysisRoot(), nodeVector);

	branchingFactor = bvh.branchingFactor;
	leafSize = bvh.leafSize;

	int padTo = 8;
	bool padInside = false;
	bool padAfter = true;
	//set ids:
	for (size_t i = 0; i < nodeVector.size(); i++)
	{
		nodeVector[i]->id = i;
	}

	//SoA order triangle list
	trianglePoints.reserve(bvh.primitives->size() * 3 * 3);

	for (auto& n : nodeVector)
	{
		//begin and end the same -> empty
		uint32_t cBegin = 0;
		uint32_t cCount = 0;
		uint32_t pBegin = 0;
		uint32_t pCount = 0;
		if (n->node->getChildCount() > 0)
		{
			cBegin = (*n->children.begin())->id;
			cCount = n->node->getChildCount();
		}
		if (n->node->getPrimCount() > 0)
		{
			pBegin = std::distance(bvh.primitives->begin(), n->node->primitiveBegin);
			pCount = n->node->getPrimCount();
			pBegin = trianglePoints.size();
			//idea is to restucture the floats inside the primitives
			//i want to have all x of p0, then all y of p0, then all z of p0 -> all x of p1 ...
			for (int i = 0; i < 3; i++)
			{
				std::for_each(std::execution::seq, n->node->primitiveBegin, n->node->primitiveEnd,
					[&](auto& p)
					{
						Triangle* tri = static_cast<Triangle*>(p.get());
						glm::vec3 p0, p1, p2;
						tri->getVertexPositions(p0, p1, p2);
						trianglePoints.push_back(p0[i]);
					});
				if (padInside)
					pad(padTo, trianglePoints);
			}
			for (int i = 0; i < 3; i++)
			{
				std::for_each(std::execution::seq, n->node->primitiveBegin, n->node->primitiveEnd,
					[&](auto& p)
					{
						Triangle* tri = static_cast<Triangle*>(p.get());
						glm::vec3 p0, p1, p2;
						tri->getVertexPositions(p0, p1, p2);
						trianglePoints.push_back(p1[i]);
					});
				if (padInside)
					pad(padTo, trianglePoints);
			}
			for (int i = 0; i < 3; i++)
			{
				std::for_each(std::execution::seq, n->node->primitiveBegin, n->node->primitiveEnd,
					[&](auto& p)
					{
						Triangle* tri = static_cast<Triangle*>(p.get());
						glm::vec3 p0, p1, p2;
						tri->getVertexPositions(p0, p1, p2);
						trianglePoints.push_back(p2[i]);
					});
				if (padInside)
					pad(padTo, trianglePoints);
			}
			if (padAfter)
				pad(padTo, trianglePoints);
		}
		Aabb* aabb = static_cast<Aabb*>(n->node);


		//aabbs of children:
		/*
		std::vector<glm::vec3> bounds;
		bounds.reserve(n->node->getChildCount() * 2);

		for (auto& c : n->children)
		{
			bounds.push_back(c->boundMin);
			bounds.push_back(c->boundMax);
		}*/

		//soa order aabb
		uint32_t aabbId = boundsSoA.size();
		for (int i = 0; i < 3; i++)
		{
			for (auto& c : n->children)
			{
				boundsSoA.push_back(c->boundMin[i]);
			}
			if (padInside)
				pad(padTo, boundsSoA);
		}
		for (int i = 0; i < 3; i++)
		{
			for (auto& c : n->children)
			{
				boundsSoA.push_back(c->boundMax[i]);
			}
			if (padInside)
				pad(padTo, boundsSoA);
		}
		if (padAfter)
			pad(padTo, boundsSoA);

		compactNodes.push_back(FastNode(cBegin, cCount, pBegin, pCount, aabbId, n->node->traverseOrderEachAxis));
	}

	//fill triangle array:
	//triangles.reserve(bvh.primitives->size());
	/*
	for (auto& p : *bvh.primitives)
	{
		Triangle* tri = static_cast<Triangle*>(p.get());
		glm::vec3 p0, p1, p2;
		tri->getVertexPositions(p0, p1, p2);
		triangles.push_back(FastTriangle(p0, p1, p2));
	}*/
}

bool FastNodeManager::intersect(FastRay& ray, double& timeTriangleTest)
{
	//ids of ndodes that we still need to test:
	//TODO: test perfroamnce if pair is faster !!
	std::vector<uint32_t> queue;
	queue.reserve(40);
	queue.push_back(0);
	std::vector<float> distances;
	distances.reserve(40);
	distances.push_back(0);
	bool result = false;


	//prepare some memory for ispc triangle method output: 
	std::vector<glm::vec3> surfaceNormals(leafSize);
	for (auto& i : surfaceNormals)
	{
		i.x = 2;
	}
	std::vector<glm::vec3> surfacePositions(leafSize);

	//prepare some data for ispc aabb method output:
	std::vector<float> aabbDistances(branchingFactor);

	while (queue.size() != 0)
	{
		//get current id (most recently added because we do depth first)
		uint32_t id = queue.back();
		queue.pop_back();
		float distance = distances.back();
		distances.pop_back();
		if (distance > ray.tMax)
		{
			continue;
		}

		FastNode* node = &compactNodes[id];

		if (!node->hasChildren)
		{
			auto timeBeforeTriangleTest = getTime();

			//primitive test: ispc instruction
			//if (triIntersect((ispc::Triangle*)triangles.data(), node->primIdBegin,
			//	(ispc::Ray*) & ray, (float*)surfaceNormals.data(), (float*)surfacePositions.data(), node->primIdEndOffset))

			if (triIntersect(trianglePoints.data(), node->primIdBegin,
				(ispc::Ray*) & ray, (float*)surfaceNormals.data(), (float*)surfacePositions.data(), node->primCount))
			{
				//find  the elements in surfacenormals and surfacepositions because ... yeay
				for (int i = 0; i < surfaceNormals.size(); i++)
				{
					//since i currently need to find the right element, surfaceNormal x = 2 is wrong.
					//The rest of the data can be random, the ispc programm overwrites the random data to what we want

					if (surfaceNormals[i].x != 2)
					{
						ray.surfaceNormal = surfaceNormals[i];
						surfaceNormals[i].x = 2;
						ray.surfacePosition = surfacePositions[i];
					}
				}
				if (ray.shadowRay)
				{
					return true;
				}
				result = true;
			}

			/*
			//serial version here:
			for (uint32_t pId = node->primIdBegin; pId < node->primIdBegin + node->primIdEndOffset; pId++)
			{
				//if shadowray :  return true if hit
				if (triangleCheck(ray, pId))
				{
					if (ray.shadowRay)
					{
						return true;
					}
					result = true;
				}
			}*/
			timeTriangleTest += getTimeSpan(timeBeforeTriangleTest);
		}
		else
		{
			//child tests: 
			//ispc version:

			//call ispc method that returns an array of min distances. if minDist = 0 -> no hit otherwise hit.
			//go trough traverseOrder and add those childs with distance != 0 to queue (in right order)

			if (aabbIntersect((float*)boundsSoA.data(), (float*)aabbDistances.data(), (ispc::Ray*) & ray, node->boundsId, node->childCount))
			{
				if (node->boundsId != 0)
				{
					auto deb = 0;
				}
				int8_t code = 0;
				code = code | (ray.direction[0] <= 0);
				code = code | ((ray.direction[1] <= 0) << 1);
				bool reverse = ray.direction[2] <= 0;
				if (reverse)
				{
					code = code ^ 3;
					std::for_each(std::execution::seq, node->traverseOrderEachAxis[code].begin(), node->traverseOrderEachAxis[code].begin() + node->childCount,
						[&](auto& cId)
						{
							if (aabbDistances[cId] != -100000)
							{
								queue.push_back(node->childIdBegin + cId);
								distances.push_back(aabbDistances[cId]);
								aabbDistances[cId] = -100000;
							}
						});
				}
				else
				{
					std::for_each(std::execution::seq, node->traverseOrderEachAxis[code].rbegin() + (16 - node->childCount), node->traverseOrderEachAxis[code].rend(),
						[&](auto& cId)
						{
							if (aabbDistances[cId] != -100000)
							{
								queue.push_back(node->childIdBegin + cId);
								distances.push_back(aabbDistances[cId]);
								aabbDistances[cId] = -100000;
							}
						});
				}
			}


			//serial version here:
			/*
			int8_t code = 0;
			code = code | (ray.direction[0] <= 0);
			code = code | ((ray.direction[1] <= 0) << 1);
			bool reverse = ray.direction[2] <= 0;
			if (reverse)
			{
				code = code ^ 3;
				std::for_each(std::execution::seq, node->traverseOrderEachAxis[code].begin(), node->traverseOrderEachAxis[code].begin() + node->childIdEndOffset + 1,
					[&](auto& cId)
					{

						if (aabbCheck(ray, node->bounds[cId * 2], node->bounds[cId * 2 + 1], distance))
						{
							queue.push_back(node->childIdBegin + cId);
							distances.push_back(distance);
						}
					});
			}
			else
			{
				std::for_each(std::execution::seq, node->traverseOrderEachAxis[code].rbegin() + (15 - node->childIdEndOffset), node->traverseOrderEachAxis[code].rend(),
					[&](auto& cId)
					{
						if (aabbCheck(ray, node->bounds[cId * 2], node->bounds[cId * 2 + 1], distance))
						{
							queue.push_back(node->childIdBegin + cId);
							distances.push_back(distance);
						}
					});
			}
			*/
		}
	}
	return result;
}

//first add all children of node, then recusion for each child
void FastNodeManager::customTreeOrder(NodeAnalysis* n, std::vector<NodeAnalysis*>& nodeVector)
{
	//same as in nodeManager
	for (auto& c : n->children)
	{
		nodeVector.push_back(c.get());
	}
	for (auto& c : n->children)
	{
		customTreeOrder(c.get(), nodeVector);
	}
}

inline bool FastNodeManager::triangleCheck(FastRay& ray, uint32_t& id)
{
	/*
	//mostly from here: http://www.pbr-book.org/3ed-2018/Shapes/Triangle_Meshes.html
	//code here https://github.com/mmp/pbrt-v3/blob/master/src/shapes/triangle.cpp

	//similar code also used in triangle.cpp

	//transform vertex positions to ray coordinate space:
	glm::vec3 p0t = triangles[id].points[0] - ray.pos;
	glm::vec3 p1t = triangles[id].points[1] - ray.pos;
	glm::vec3 p2t = triangles[id].points[2] - ray.pos;

	//permute components  of triangle vertices and ray direction
	glm::vec3 tmp = glm::abs(ray.direction);
	int kz = maxDimension(tmp);
	int kx = kz + 1; if (kx == 3) kx = 0;
	int ky = kx + 1; if (ky == 3) ky = 0;
	glm::vec3 d = permute(ray.direction, kx, ky, kz);
	p0t = permute(p0t, kx, ky, kz);
	p1t = permute(p1t, kx, ky, kz);
	p2t = permute(p2t, kx, ky, kz);

	//apply shear transformation to translated vertex positions
	float Sx = -d.x / d.z;
	float Sy = -d.y / d.z;
	float Sz = 1.f / d.z;
	p0t.x += Sx * p0t.z;
	p0t.y += Sy * p0t.z;
	p1t.x += Sx * p1t.z;
	p1t.y += Sy * p1t.z;
	p2t.x += Sx * p2t.z;
	p2t.y += Sy * p2t.z;

	// Compute edge function coefficients _e0_, _e1_, and _e2_
	float e0 = p1t.x * p2t.y - p1t.y * p2t.x;
	float e1 = p2t.x * p0t.y - p2t.y * p0t.x;
	float e2 = p0t.x * p1t.y - p0t.y * p1t.x;

	// Fall back to double precision test at triangle edges
	if (e0 == 0.0f || e1 == 0.0f || e2 == 0.0f) {
		double p2txp1ty = (double)p2t.x * (double)p1t.y;
		double p2typ1tx = (double)p2t.y * (double)p1t.x;
		e0 = (float)(p2typ1tx - p2txp1ty);
		double p0txp2ty = (double)p0t.x * (double)p2t.y;
		double p0typ2tx = (double)p0t.y * (double)p2t.x;
		e1 = (float)(p0typ2tx - p0txp2ty);
		double p1txp0ty = (double)p1t.x * (double)p0t.y;
		double p1typ0tx = (double)p1t.y * (double)p0t.x;
		e2 = (float)(p1typ0tx - p1txp0ty);
	}

	// Perform triangle edge and determinant tests
	if ((e0 < 0 || e1 < 0 || e2 < 0) && (e0 > 0 || e1 > 0 || e2 > 0))
		return false;
	float det = e0 + e1 + e2;
	if (det == 0) return false;

	// Compute scaled hit distance to triangle and test against ray $t$ range
	p0t.z *= Sz;
	p1t.z *= Sz;
	p2t.z *= Sz;
	float tScaled = e0 * p0t.z + e1 * p1t.z + e2 * p2t.z;
	if (det < 0 && (tScaled >= 0 || tScaled < ray.tMax * det))
		return false;
	else if (det > 0 && (tScaled <= 0 || tScaled > ray.tMax* det))
		return false;

	// Compute barycentric coordinates and $t$ value for triangle intersection
	float invDet = 1 / det;
	float b0 = e0 * invDet;
	float b1 = e1 * invDet;
	float b2 = e2 * invDet;
	float t = tScaled * invDet;

	// Ensure that computed triangle $t$ is conservatively greater than zero

	// Compute $\delta_z$ term for triangle $t$ error bounds
	tmp = glm::abs(glm::vec3(p0t.z, p1t.z, p2t.z));
	float maxZt = maxComponent(tmp);
	float deltaZ = gamma(3) * maxZt;

	// Compute $\delta_x$ and $\delta_y$ terms for triangle $t$ error bounds
	tmp = glm::abs(glm::vec3(p0t.x, p1t.x, p2t.x));
	float maxXt = maxComponent(tmp);
	tmp = glm::abs(glm::vec3(p0t.y, p1t.y, p2t.y));
	float maxYt = maxComponent(tmp);
	float deltaX = gamma(5) * (maxXt + maxZt);
	float deltaY = gamma(5) * (maxYt + maxZt);

	// Compute $\delta_e$ term for triangle $t$ error bounds
	float deltaE = 2 * (gamma(2) * maxXt * maxYt + deltaY * maxXt + deltaX * maxYt);

	// Compute $\delta_t$ term for triangle $t$ error bounds and check _t_
	tmp = glm::abs(glm::vec3(e0, e1, e2));
	float maxE = maxComponent(tmp);
	float deltaT = 3 *
		(gamma(3) * maxE * maxZt + deltaE * maxZt + deltaZ * maxE) *
		std::abs(invDet);
	if (t <= deltaT) return false;

	//finished for shadowRay:
	if (ray.shadowRay)
	{
		//small number to prevent self shadowing due to floating point errors
		//return t >= 0.001f;

		//instead of above we spawns shadowrays "above" the surface
		return true;
	}


	//ray triangle intersection complete
	// Interpolate $(u,v)$ parametric coordinates and hit point
	glm::vec3 pHit = b0 * triangles[id].points[0] + b1 * triangles[id].points[1] + b2 * triangles[id].points[2];

	ray.tMax = t;
	//manual surface point calculation:
	ray.surfaceNormal = -computeNormal(triangles[id].points[0], triangles[id].points[1], triangles[id].points[2]);
	ray.surfacePosition = pHit;
	return true;
	*/

	//faster version -> from ISPC ratracer exmaple https://github.com/ispc/ispc/blob/master/examples/rt/rt_serial.cpp

	//glm::vec3 e1 = triangles[id].points[1] - triangles[id].points[0];
	//glm::vec3 e2 = triangles[id].points[2] - triangles[id].points[0];

	glm::vec3 p0 = triangles[id].points[0];
	glm::vec3 p1 = triangles[id].points[1];
	glm::vec3 p2 = triangles[id].points[2];

	//glm::vec3 p0(triangles[id].p[0][0], triangles[id].p[0][1], triangles[id].p[0][2]);
	//glm::vec3 p1(triangles[id].p[1][0], triangles[id].p[1][1], triangles[id].p[1][2]);
	//glm::vec3 p2(triangles[id].p[2][0], triangles[id].p[2][1], triangles[id].p[2][2]);

	//glm::vec3 p0 = glm::make_vec3(triangles[id].p[0]);
	//glm::vec3 p1 = glm::make_vec3(triangles[id].p[1]);
	//glm::vec3 p2 = glm::make_vec3(triangles[id].p[2]);


	glm::vec3 e1 = p1 - p0;
	glm::vec3 e2 = p2 - p0;
	glm::vec3 s1 = glm::cross(ray.direction, e2);
	float divisor = glm::dot(s1, e1);

	if (divisor == 0.)
		return false;
	float invDivisor = 1.f / divisor;

	// Compute first barycentric coordinate
	glm::vec3 d = ray.pos - p0;
	float b1 = glm::dot(d, s1) * invDivisor;
	if (b1 < 0. || b1 > 1.)
		return false;

	// Compute second barycentric coordinate
	glm::vec3 s2 = glm::cross(d, e1);
	float b2 = glm::dot(ray.direction, s2) * invDivisor;
	if (b2 < 0. || b1 + b2 > 1.)
		return false;

	// Compute _t_ to intersection point
	float t = glm::dot(e2, s2) * invDivisor;
	if (t < 0 || t > ray.tMax)
		return false;

	ray.surfacePosition = p0 + e1 * b1 + e2 * b2;
	ray.tMax = t;
	ray.surfaceNormal = computeNormal(p0, p1, p2);
	return true;
}

inline bool FastNodeManager::aabbCheck(FastRay& ray, glm::vec3& boundMin, glm::vec3& boundMax, float& distance)
{

	//aabb intersection (same as in Aabb)
	//code modified from here : https://gamedev.stackexchange.com/questions/18436/most-efficient-aabb-vs-ray-collision-algorithms

	/*
	glm::fvec3 t1 = (boundMin - ray.pos) * ray.invDirection;
	glm::fvec3 t2 = (boundMax - ray.pos) * ray.invDirection;
	float tmin = glm::compMax(glm::min(t1, t2));
	float tmax = glm::compMin(glm::max(t1, t2));

	// if tmax < 0, ray (line) is intersecting AABB, but the whole AABB is behind us
	if (tmax < 0)
	{
		return false;
	}
	// if tmin > tmax, ray doesn't intersect AABB
	if (tmin > tmax)
	{
		return false;
	}
	//stop when current ray distance is closer than minimum possible distance of the aabb
	if (ray.tMax < tmin)
	{
		return false;
	}
	distance = tmin;
	return true;
	*/

	//faster version -> from ISPC ratracer exmaple https://github.com/ispc/ispc/blob/master/examples/rt/rt_serial.cpp
	float t0 = 0, t1 = ray.tMax;
	glm::fvec3 tNear = (boundMin - ray.pos) * ray.invDirection;
	glm::fvec3 tFar = (boundMax - ray.pos) * ray.invDirection;
	if (tNear.x > tFar.x)
	{
		std::swap(tNear.x, tFar.x);
	}
	t0 = std::max(tNear.x, t0);
	t1 = std::min(tFar.x, t1);

	if (tNear.y > tFar.y)
	{
		std::swap(tNear.y, tFar.y);
	}
	t0 = std::max(tNear.y, t0);
	t1 = std::min(tFar.y, t1);

	if (tNear.z > tFar.z)
	{
		std::swap(tNear.z, tFar.z);
	}
	t0 = std::max(tNear.z, t0);
	t1 = std::min(tFar.z, t1);

	distance = t0;
	return (t0 <= t1);
}