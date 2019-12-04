#include "fastRay.h"


FastRay::FastRay(const glm::vec3& pos, const glm::vec3& direction,const bool shadowRay)
	: pos(pos), direction(direction), shadowRay(shadowRay)
{	this->direction = glm::normalize(direction);

	//fix infinite because 0 / 0 is NaN
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
	surfaceNormal = glm::vec3(0, 0, 0);
	surfacePosition = glm::vec3(0, 0, 0);

}