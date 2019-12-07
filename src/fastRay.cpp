#include "fastRay.h"


FastRay::FastRay(const glm::vec3& pos, const glm::vec3& direction, float maxDistance)
	: pos(pos), direction(direction), tMax(maxDistance)
{
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
}