#pragma once
#include <iostream>
#include <vector>

#include "glmInclude.h"
#include "gameobject.h"
#include "vertex.h"
#include "mesh.h"
#include "texture.h"
#include "util.h"
#include "color.h"


//include tinygltf
// Define these only in *one* .cc file.
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.
#include "tinyGltf/tiny_gltf.h"

void loadGltfModel(std::string modelPath, std::vector<std::shared_ptr<GameObject>>& gameObjects, std::vector<std::shared_ptr<Mesh>>& meshes)
{
	//TODO: update tinygltf form time to time (or wait for next release?) https://github.com/syoyo/tinygltf
	tinygltf::Model gltfModel;
	tinygltf::TinyGLTF loader;
	std::string error, warning;

	bool ret = false;
	//detect type(glb or gltf
	if (modelPath.substr(modelPath.find_last_of(".") + 1) == "gltf")
	{
		ret = loader.LoadASCIIFromFile(&gltfModel, &error, &warning, modelPath);
	}
	else if (modelPath.substr(modelPath.find_last_of(".") + 1) == "glb")
	{
		ret = loader.LoadBinaryFromFile(&gltfModel, &error, &warning, modelPath);
	}
	else
	{
		std::cerr << "Path was not a gltf or glb file" << std::endl;
	}

	if (!warning.empty()) {
		printf("Warning: %s\n", warning.c_str());
	}
	if (!error.empty()) {
		printf("Error: %s\n", error.c_str());
	}
	if (!ret) {
		std::cerr << "Could not load gltf file: " << error << std::endl;
	}

	//helpfull: https://github.com/syoyo/tinygltf/wiki/Accessing-vertex-data

	//vector of sharded pointers to images. The sharedptr are later in the meshes
	std::vector<std::shared_ptr<Texture>> textures;

	//offset to the modelUID of gltf
	size_t modelIndexOffset = meshes.size();
	//offset to the gameobject uid of gltf (needed to reference the right children)
	size_t gameObjectOffset = gameObjects.size();

	//import and load all images into gpu memory:
	//increase texture vector sizes:
	textures.reserve(gltfModel.images.size());

	//go trough all images and import them.
	for (int i = 0; i < gltfModel.images.size(); i++)
	{
		auto& gltfImage = gltfModel.images[i];

		if (gltfImage.component != 4)
		{
			std::cout << "image doesnt have 4 components... -> must convert to rgba (if required)" << std::endl;
			//look at this https://github.com/SaschaWillems/Vulkan/blob/master/base/VulkanglTFModel.hpp for help
		}
		//think about bits
		if (gltfImage.bits != 8)
		{
			std::cout << "image doesnt have 8 bits per component?" << std::endl;
		}

		//need to check what exactly happens to the memory here? i kinda want it to copy??
		//auto sharPtr = std::make_shared<std::vector<unsigned char>>(std::move(gltfImage));
		textures.push_back(std::make_shared<Texture>(gltfImage.width, gltfImage.height, gltfImage.image));
	}

	struct VertexId {
		int positionId = -1;
		int normalId = -1;
		int uvId = -1;

		VertexId(int positionId, int normalId, int uvId)
			: positionId(positionId), normalId(normalId), uvId(uvId)
		{
		}

		bool operator==(const VertexId& other) const
		{
			return positionId == other.positionId && normalId == other.normalId && uvId == other.uvId;
		}

	};

	//both vectors are there to determine if a vertex / index array is already loaded
	std::vector<VertexId> vertexIds;
	std::vector<int> indexIds;

	//vector of sharded pointers to the vertexArrays. The sharedptr are later in the meshes
	std::vector<std::shared_ptr<std::vector<Vertex>>> vertices;

	//vector of sharded pointers to the indexArrays. The sharedptr are later in the meshes
	std::vector<std::shared_ptr<std::vector<uint32_t>>> indices;

	//go through all meshes and import them.

	for (size_t i = 0; i < gltfModel.meshes.size(); i++)
	{
		auto& m = gltfModel.meshes[i];
		size_t primitiveId = 0;

		auto& primitive = m.primitives[primitiveId];
		if (m.primitives.size() != 1)
		{
			//TODO do something else here ;(
			std::cout << "primtive count is not 1 but " << m.primitives.size() << std::endl;
		}

		size_t meshVertexId = 0;
		size_t meshIndexId = 0;

		//i use the id that is used to get the accessor for testing if its unique

		auto positionAccId = primitive.attributes["POSITION"];
		auto normalAccId = primitive.attributes["NORMAL"];
		auto uvAccId = primitive.attributes["TEXCOORD_0"];

		//check if we already loaded this combination of position,normal,and uv
		VertexId vId = VertexId(positionAccId, normalAccId, uvAccId);
		auto vertexResult = std::find(vertexIds.begin(), vertexIds.end(), vId);
		meshVertexId = vertexResult - vertexIds.begin();
		if (vertexResult == vertexIds.end())
		{
			//add the next combination:
			vertexIds.push_back(vId);

			//load into memory

			//position
			auto& accessor = gltfModel.accessors[positionAccId];
			tinygltf::BufferView& bufferView = gltfModel.bufferViews[accessor.bufferView];
			tinygltf::Buffer& buffer = gltfModel.buffers[bufferView.buffer];
			glm::vec3* positionArray = reinterpret_cast<glm::vec3*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);

			//normal
			auto& normalAccessor = gltfModel.accessors[normalAccId];
			auto& normalBufferView = gltfModel.bufferViews[normalAccessor.bufferView];
			auto& normalBuffer = gltfModel.buffers[normalBufferView.buffer];
			glm::vec3* normalArray = reinterpret_cast<glm::vec3*>(&normalBuffer.data[normalBufferView.byteOffset + normalAccessor.byteOffset]);

			//texture coord
			auto& uvAccessor = gltfModel.accessors[uvAccId];
			auto& uvBufferView = gltfModel.bufferViews[uvAccessor.bufferView];
			auto& uvBuffer = gltfModel.buffers[uvBufferView.buffer];
			glm::vec2* uvArray = reinterpret_cast<glm::vec2*>(&uvBuffer.data[uvBufferView.byteOffset + uvAccessor.byteOffset]);

			//im confused about the uvBufferView.bytelength   its with stride 8 (vec2).   the normal and position have stride 12 and triple the bytelength than the uv list????

			//store into vertex vector
			//initialize sharedptr
			vertices.push_back(std::make_shared<std::vector<Vertex>>());
			vertices[meshVertexId]->reserve(vertices.size() + accessor.count);
			for (size_t j = 0; j < accessor.count; ++j) {
				//go trough each vertex variable we have. (pos, normal, uvcoordinate)
				vertices[meshVertexId]->push_back(Vertex(positionArray[j], normalArray[j], uvArray[j]));
			}
		}
		else
		{
			auto deb = 0;
			//this combination is already loeaded into memory.
		}

		//indexBuffer
		auto& indexAccessor = gltfModel.accessors[primitive.indices];

		//check if we already loaded this indexBuffer (neccessary since we iterate trough meshes which could share index and vertex buffer but just have a differnt texture)
		int iId = primitive.indices;
		auto indexResult = std::find(indexIds.begin(), indexIds.end(), iId);
		meshIndexId = indexResult - indexIds.begin();
		if (indexResult == indexIds.end())
		{
			indexIds.push_back(iId);

			auto& indexBufferView = gltfModel.bufferViews[indexAccessor.bufferView];
			auto& indexBuffer = gltfModel.buffers[indexBufferView.buffer];

			//depending on component type: (vertex.insert converts corretly)
			if (indexAccessor.componentType == 5125)
			{
				//unsigned int
				uint32_t* indexArray = reinterpret_cast<uint32_t*>(&indexBuffer.data[indexBufferView.byteOffset + indexAccessor.byteOffset]);
				//directly copy to vector:
				indices.push_back(std::make_shared<std::vector<uint32_t>>(indexArray, indexArray + indexAccessor.count));
			}
			else if (indexAccessor.componentType == 5123)
			{
				//unsigned short
				uint16_t* indexArray = reinterpret_cast<uint16_t*>(&indexBuffer.data[indexBufferView.byteOffset + indexAccessor.byteOffset]);
				//directly copy to vector:
				indices.push_back(std::make_shared<std::vector<uint32_t>>(indexArray, indexArray + indexAccessor.count));
			}
			else if (indexAccessor.componentType == 5121)
			{
				//unsigned byte
				uint8_t* indexArray = reinterpret_cast<uint8_t*>(&indexBuffer.data[indexBufferView.byteOffset + indexAccessor.byteOffset]);
				//directly copy to vector:
				indices.push_back(std::make_shared<std::vector<uint32_t>>(indexArray, indexArray + indexAccessor.count));
			}
			else
			{
				std::cout << "index component type not known?" << std::endl;
			}
		}
		else
		{
			auto debug = 0;
			//index already loaded
		}

		int matId = m.primitives[primitiveId].material;

		//TODO: load all textures correctly:

		int texId = -1;
		if (gltfModel.materials[matId].pbrMetallicRoughness.baseColorTexture.index != -1)
		{
			texId = gltfModel.materials[matId].pbrMetallicRoughness.baseColorTexture.index;
		}
		else if (gltfModel.materials[matId].emissiveTexture.index != -1)
		{
			texId = gltfModel.materials[matId].emissiveTexture.index;
		}
		auto color = gltfModel.materials[matId].pbrMetallicRoughness.baseColorFactor;
		meshes.push_back(std::make_shared<Mesh>(vertices[meshVertexId], indices[meshIndexId], Color(color), m.name));

		//add the different textures if they are specified
		if (texId != -1)
		{
			meshes[i + modelIndexOffset]->texture = textures[texId];
		}
	}

	size_t vertexCount = 0;
	for (auto& v : vertices)
	{
		vertexCount += v->size();
	}
	std::cout << "loaded " << vertices.size() << " GameObjects with " << vertexCount << " Vertices" << std::endl;

	//reserve enought gameobjects so we can point on empty slots (TODO: need to check if this is correct????????)
	gameObjects.resize(gameObjects.size() + gltfModel.nodes.size());
	int id = 0;

	//go trough nodes. -> make the gameobjects and link all the models and textures to it
	//i think i sould remove empty (mesh == -1) nodes THAT DONT change the transform?
	for (auto& node : gltfModel.nodes)
	{
		glm::quat rot = glm::mat4(1.0f);
		glm::vec3 tran = glm::vec4(0.0f);
		glm::vec3 scale = glm::vec3(1.0f);
		glm::mat4 mat = glm::mat4(1.0f);
		if (node.rotation.size() == 4)
		{
			rot = glm::make_quat(node.rotation.data());
		}
		if (node.translation.size() == 3)
		{
			tran = glm::make_vec3(node.translation.data());
		}
		if (node.scale.size() == 3)
		{
			scale = glm::make_vec3(node.scale.data());
		}
		//matrix is decomposed into components that are then saved into the go
		if (node.matrix.size() == 16)
		{
			mat = glm::make_mat4x4(node.matrix.data());

			glm::vec3 decScale;
			glm::quat decOrientation;
			glm::vec3 decTranslation;
			glm::vec3 decSkew;
			glm::vec4 decPerspective;
			glm::decompose(mat, decScale, decOrientation, decTranslation, decSkew, decPerspective);
			scale *= decScale;
			tran += decTranslation;
			//not sure if its correct but it works when rot is not set by node
			rot = rot * decOrientation;
			//when there is skew then something fucked up .... (ignored)
		}

		std::vector<int> childIds;
		//creating children pointer:
		for (auto& c : node.children)
		{
			//i really hope gltf does not support circular dependencies
			childIds.push_back(c + (int)gameObjectOffset);
		}

		//TODO node.mesh is not correct when i only save unique models
		//auto go = GameObject(std::make_shared<Mesh>(meshes[node.mesh + modelIndexOffset]), tran, rot, scale, node.name);
		//gameobjects.push_back(go);

		int meshId = node.mesh;
		if (node.mesh != -1)
		{
			gameObjects[id + gameObjectOffset] = std::make_shared<GameObject>(meshes[node.mesh + modelIndexOffset], childIds, tran, rot, scale, node.name);
		}
		else
		{
			gameObjects[id + gameObjectOffset] = std::make_shared<GameObject>(nullptr, childIds, tran, rot, scale, node.name);
		}

		id++;
	}
}