#include "fastRay.h"


FastRay::FastRay(const glm::vec3& pos, const glm::vec3& direction)
	: pos(pos), direction(direction)
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
	tMax = 222222.f;
	leafIndex = 0;
	triIndex = 0;
}