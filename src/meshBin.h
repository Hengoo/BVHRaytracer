#pragma once
#include <iostream>
#include <vector>
#include <array>

//#include "mesh.h"

class Mesh;

//Just a bin of multiple meshes.(according to gltf a mesh can contain multiple "primitives" with different textures)
class MeshBin
{
public:
	std::string name;
	std::vector<std::shared_ptr<Mesh>> meshes;

	inline void addMesh(std::shared_ptr<Mesh> mesh)
	{
		meshes.push_back(mesh);
	}

	MeshBin(std::string name)
		:name(name)
	{
	}
};