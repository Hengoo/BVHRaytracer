#include "ray.h"
#include "global.h"

Ray::Ray(glm::vec3 pos, glm::vec3 direction, bool shadowRay)
	: pos(pos), direction(direction), shadowRay(shadowRay)
{
	nodeIntersectionCount.resize(bvh.bvhDepth);
	leafIntersectionCount.resize(bvh.bvhDepth);
	childFullness.resize(bvh.branchingFactor + 1);
	primitiveFullness.resize(bvh.leafCount + 1);

	this->direction = glm::normalize(direction);
	invDirection = 1.0f / this->direction;
	tMax = 222222.f;
	surfaceColor = Color(0);
	surfaceNormal = glm::vec3(0, 0, 0);
	surfacePosition = glm::vec3(0, 0, 0);
	primitiveIntersectionCount = 0;
	successfulPrimitiveIntersectionCount = 0;
	successfulAabbIntersectionCount = 0;
	aabbIntersectionCount = 0;
}