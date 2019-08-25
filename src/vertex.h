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

	Vertex()
	{
		pos = glm::vec3(0);
		normal = glm::vec3(0,1,0);
		texCoord = glm::vec2(0);
	}
};