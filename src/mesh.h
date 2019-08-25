#pragma once

#include <iostream>
#include <vector>
#include <array>


#include "glmInclude.h"
#include "vertex.h"
#include "texture.h"

//triangle mesh. If i ever want a mesh of other primitives i can make some interface structure
class Mesh
{
public:
	//mesh and gameobjects are independent from bvh, but are referenced from primitives(triangles) that are stored in bvh

	//variables i need for gameobject: -> gameobject structure so i can resue mesh data when possible
	//parent /  child
	//local transform
	//World transform -> calc from parent transform

	std::shared_ptr<std::vector<Vertex>> vertices;
	std::shared_ptr<std::vector<uint32_t>> indices;
	//all textures can be empty
	std::shared_ptr<Texture> texture;
	//TODO: add the other possible textures (at least normalmap?)

	std::array<unsigned char, 4> color;
	std::string name;

	Mesh(std::shared_ptr<std::vector<Vertex>> vertices, std::shared_ptr<std::vector<uint32_t>> indices, std::array<unsigned char, 4> color, std::string name)
		:vertices(vertices), indices(indices), color(color), name(name)
	{
	}
};