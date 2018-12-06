#include "ObjLoader.h"

#include "tiny_obj_loader.h"

#include <iostream>

namespace Nome
{

CObjLoader::CObjLoader(const std::string& fileName) : FileName(fileName)
{
}

CEffiMesh* CObjLoader::LoadEffiMesh(CGraphicsDevice* gd)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string warn;
	std::string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, FileName.c_str());

	if (!err.empty())
	{
		std::cerr << err << std::endl;
	}

	if (!ret)
	{
		return nullptr;
	}

	auto* mesh = new CEffiMesh(gd);

	// Create position buffer
	CEffiMesh::AttributeBuffer posBuffer;
	posBuffer.DataType = EDataType::Float3;
	posBuffer.AttrArr.resize(attrib.vertices.size() * sizeof(float));
	memcpy(posBuffer.AttrArr.data(), attrib.vertices.data(), posBuffer.AttrArr.size());
	mesh->VertexAttrs.insert({ "pos", posBuffer });

	// Loop over shapes
	for (size_t s = 0; s < shapes.size(); s++) {
		// Loop over faces(polygon)
		size_t index_offset = 0;
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			int fv = shapes[s].mesh.num_face_vertices[f];

			CEffiMesh::FaceData faceData;

			// Loop over vertices in the face.
			for (size_t v = 0; v < fv; v++) {
				// access to vertex
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				faceData.VertexIndexList.push_back(idx.vertex_index);
			}
			index_offset += fv;

			mesh->Faces.push_back(faceData);
		}
	}

	return mesh;
}

} /* namespace Nome */
