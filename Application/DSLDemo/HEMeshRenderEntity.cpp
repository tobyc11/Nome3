#include "HEMeshRenderEntity.h"

#include <glad/glad.h>

#include <vector>

namespace Nome
{

void CHEMeshRenderEntity::Draw() const
{
    GLuint vertBuffer, normalBuffer, vao;
    glGenBuffers(1, &vertBuffer);
	glGenBuffers(1, &normalBuffer);
    glGenVertexArrays(1, &vao);

    std::vector<Vector3> positions_tris;
	std::vector<Vector3> normals_tris;

    for (auto iter = Mesh->FacesBegin(); iter != Mesh->FacesEnd(); ++iter)
    {
        auto faceVerts = Mesh->FVQuery(*iter);
        //Triangulate
        for (size_t i = 1; i < faceVerts.size() - 1; i++)
        {
            positions_tris.push_back(faceVerts[0]->Position);
            positions_tris.push_back(faceVerts[i]->Position);
            positions_tris.push_back(faceVerts[i + 1]->Position);

			auto d1 = faceVerts[i]->Position - faceVerts[0]->Position;
			auto d2 = faceVerts[i + 1]->Position - faceVerts[0]->Position;
			auto normal = d2.CrossProduct(d1).Normalized();
			normals_tris.push_back(normal);
			normals_tris.push_back(normal);
			normals_tris.push_back(normal);
        }
    }

    glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vertBuffer);
	glBufferData(GL_ARRAY_BUFFER, positions_tris.size() * sizeof(Vector3), positions_tris.data(), GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glBufferData(GL_ARRAY_BUFFER, normals_tris.size() * sizeof(Vector3), normals_tris.data(), GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glDrawArrays(GL_TRIANGLES, 0, (GLsizei)positions_tris.size());
    glBindVertexArray(0);

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vertBuffer);
	glDeleteBuffers(1, &normalBuffer);
}

}
