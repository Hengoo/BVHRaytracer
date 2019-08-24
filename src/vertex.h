#pragma once
#include "glmInclude.h"
#include "primitives/primitive.h"

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 texCoord;
	//color? gltf would support vertex color but lets see if the models need it

	Vertex(glm::vec3 pos, glm::vec3 normal, glm::vec2 texCoord)
		:pos(pos), normal(normal), texCoord(texCoord)
	{
	}
};