#include "triangle.h"

#include "../accelerationStructure/bvh.h"
#include "../lights/light.h"
#include "../global.h"

#include "../glmUtil.h"
#include "../util.h"

#include "../color.h"
#include "../gameobject.h"
#include "../mesh.h"
#include "../accelerationStructure/aabb.h"

void Triangle::update()
{
	//why:
	//auto t1 = mesh->vertices->operator[](index);
	//auto t2 = (*mesh->vertices)[index];
	//auto t3 = mesh->vertices->at(index);

	glm::vec4 pos0((*mesh->vertices)[(*mesh->indices)[(size_t)index + 0]].pos, 1);
	glm::vec4 pos1((*mesh->vertices)[(*mesh->indices)[(size_t)index + 1]].pos, 1);
	glm::vec4 pos2((*mesh->vertices)[(*mesh->indices)[(size_t)index + 2]].pos, 1);

	glm::vec3 pos00 = gameObject->globalTransform * pos0;
	glm::vec3 pos11 = gameObject->globalTransform * pos1;
	glm::vec3 pos22 = gameObject->globalTransform * pos2;

	boundMin = glm::min(pos00, pos11);
	boundMax = glm::max(pos00, pos11);
	boundMin = glm::min(boundMin, pos22);
	boundMax = glm::max(boundMax, pos22);

}

bool Triangle::intersect(Ray& ray)
{
	//mostly from here: http://www.pbr-book.org/3ed-2018/Shapes/Triangle_Meshes.html
	//code here https://github.com/mmp/pbrt-v3/blob/master/src/shapes/triangle.cpp
	//it seems very long but every watertight algorithm i found is similarly complex long so i will give this one a try

	//load vertices:
	std::array<Vertex, 3> vertices = {};
	vertices[0] = (*mesh->vertices)[(*mesh->indices)[(size_t)index + 0]];
	vertices[1] = (*mesh->vertices)[(*mesh->indices)[(size_t)index + 1]];
	vertices[2] = (*mesh->vertices)[(*mesh->indices)[(size_t)index + 2]];

	//vec4 for transform:
	glm::vec4 pos0(vertices[0].pos, 1);
	glm::vec4 pos1(vertices[1].pos, 1);
	glm::vec4 pos2(vertices[2].pos, 1);

	//world space vertex position:
	std::array<glm::vec3, 3> points = {};
	points[0] = gameObject->globalTransform * pos0;
	points[1] = gameObject->globalTransform * pos1;
	points[2] = gameObject->globalTransform * pos2;

	//transform vertex positions to ray coordinate space:
	glm::vec3 p0t = points[0] - ray.pos;
	glm::vec3 p1t = points[1] - ray.pos;
	glm::vec3 p2t = points[2] - ray.pos;

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
	else if (det > 0 && (tScaled <= 0 || tScaled > ray.tMax * det))
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
	float deltaE =
		2 * (gamma(2) * maxXt * maxYt + deltaY * maxXt + deltaX * maxYt);

	// Compute $\delta_t$ term for triangle $t$ error bounds and check _t_
	tmp = glm::abs(glm::vec3(e0, e1, e2));
	float maxE = maxComponent(tmp);
	float deltaT = 3 *
		(gamma(3) * maxE * maxZt + deltaE * maxZt + deltaZ * maxE) *
		std::abs(invDet);
	if (t <= deltaT) return false;

	//ray triangle intersection complete

	//end if shadowRay:
	if (ray.shadowRay)
	{
		//small number to prevent self shadowing due to floating point errors
		return t >= 0.001f;
	}


	// Interpolate $(u,v)$ parametric coordinates and hit point
	glm::vec3 pHit = b0 * points[0] + b1 * points[0] + b2 * points[0];
	glm::vec2 uvHit = b0 * vertices[0].texCoord + b1 * vertices[1].texCoord + b2 * vertices[2].texCoord;

	Color color(1.f, 1.f, 1.f, 1.f);
	if (mesh->texture)
	{
		color.scale(mesh->texture->getColor(uvHit));
	}
	color.scale(mesh->color);

	//TODO: to this similar to derefered rendering (gather all information and solve rest later)
	//light shading / shadowrays:
	for (auto& l : lights)
	{
		float lightDistance;
		glm::vec3 lightVector;

		//todo use light color
		auto lightColor = l->getLightDirection(pHit, lightVector, lightDistance);
		//those calculated triangle normals seem to be inversed right now?
		//auto normal = -computeNormal(points[0], points[1], points[2]);
		auto normal = glm::normalize(b0 * vertices[0].normal + b1 * vertices[1].normal + b2 * vertices[2].normal);
		//TODO this is wrong with non uniform sclaing
		normal = glm::normalize(gameObject->globalTransform * glm::vec4(normal, 0));
		float f = glm::dot(normal, lightVector);
		Ray ray(pHit, lightVector, true);
		ray.tMax = lightDistance;

		//only shoot ray when surface points in light direction
		if (f > 0)
		{
			if (bvh.intersect(ray))
			{
				f = 0;
			}
		}
		f = std::max(0.2f, f);
		color.scale(f);
	}

	ray.tMax = t;
	ray.result = color;
	return true;
}

bool Triangle::intersect(Node* node)
{
	//i really dislike how this turned out but i need to determine what primitive to intersect with what node.
	Aabb* aabb = dynamic_cast<Aabb*>(node);
	if (aabb)
	{
		//aabb triangle intersection

		//for now: just aabb with aabb of triangle intersection. (could be more accurate)
		return (boundMin.x <= aabb->boundMax.x && boundMax.x >= aabb->boundMin.x) &&
			(boundMin.y <= aabb->boundMax.y && boundMax.y >= aabb->boundMin.y) &&
			(boundMin.z <= aabb->boundMax.z && boundMax.z >= aabb->boundMin.z);
		
		//TODO: more accurate aabb triangle test in case the above one detects intersection
	}

	//no know type-> should not happen
	return false;
}