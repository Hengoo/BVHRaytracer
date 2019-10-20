#include "ray.h"


Ray::Ray(glm::vec3 pos, glm::vec3 direction, Bvh bvh, bool shadowRay)
	: pos(pos), direction(direction), shadowRay(shadowRay)
{
	nodeIntersectionCount.resize(bvh.bvhDepth);
	leafIntersectionCount.resize(bvh.bvhDepth);
	childFullness.resize(bvh.branchingFactor + 1);
	primitiveFullness.resize(bvh.leafSize + 1);

	this->direction = glm::normalize(direction);
	//the possible inv here should be no problem
	
	invDirection = 1.0f / (this->direction);
	if (direction.x == 0)
	{
		invDirection.x = std::numeric_limits<float>::max();
	}
	if (direction.y == 0)
	{
		invDirection.y = std::numeric_limits<float>::max();
	}
	if (direction.z == 0)
	{
		invDirection.z = std::numeric_limits<float>::max();
	}
	tMax = 222222.f;
	surfaceColor = Color(0);
	surfaceNormal = glm::vec3(0, 0, 0);
	surfacePosition = glm::vec3(0, 0, 0);
	primitiveIntersectionCount = 0;
	successfulPrimitiveIntersectionCount = 0;
	successfulAabbIntersectionCount = 0;
	aabbIntersectionCount = 0;
}