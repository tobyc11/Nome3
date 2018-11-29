#include "DSLDemoCoreService.h"
#include "HEMeshRenderEntity.h"

#include "Nome/App.h"
#include "Nome/RenderService.h"

#include "imgui.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <unordered_map>
#include <vector>

namespace Nome
{

template <class key_t, class value_t>
bool nested_key_exists(std::unordered_map<key_t, std::unordered_map<key_t, value_t>> const& data, key_t const a, key_t const b)
{
    auto itInner = data.find(a);
    if (itInner != data.end())
    {
        return itInner->second.find(b) != itInner->second.end();
    }
    return false;
}

CHEMesh* HalfEdgeMeshFromObj(const std::string& fileName)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string warn;
    std::string err;
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, fileName.c_str());

    if (!err.empty())
    {
        std::cerr << err << std::endl;
    }

    if (!ret)
    {
        return nullptr;
    }

    auto* mesh = new CHEMesh();

    //Preallocate all vertices
    std::unordered_map<size_t, CHEMesh::Vertex*> idxToVert;
    int index = 0;
    for (size_t i = 0; i < attrib.vertices.size(); i += 3)
    {
        auto* vert = mesh->MakeVertex({attrib.vertices[i], attrib.vertices[i + 1], attrib.vertices[i + 2]});
        idxToVert[index] = vert;
        index++;
    }

    // Loop over shapes
    for (auto& shape : shapes)
    {
        std::unordered_map<CHEMesh::Vertex*, std::unordered_map<CHEMesh::Vertex*, CHEMesh::HalfEdge*>> adjList;

        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++)
        {
            int fv = shape.mesh.num_face_vertices[f];

            if (fv < 3)
            {
                index_offset += fv;
                continue;
            }

            CHEMesh::Face* face = mesh->AllocateFace();
            face->bIsBoundary = false;

            // Loop over vertices in the face.
            std::vector<CHEMesh::Vertex*> faceVerts;
            for (size_t v = 0; v < fv; v++)
            {
                faceVerts.push_back(idxToVert[shape.mesh.indices[index_offset + v].vertex_index]);
            }

            std::vector<CHEMesh::HalfEdge*> faceHE;
            for (size_t i = 0; i < faceVerts.size(); i++)
            {
                auto* fromV = faceVerts[i];
                auto* toV = faceVerts[(i + 1) % faceVerts.size()];

                auto* he = mesh->AllocateHalfEdge();
                face->OneHE = he;
                he->Face = face;
                he->Vert = fromV;
                fromV->OneHE = he;
                faceHE.push_back(he);

                if (nested_key_exists(adjList, toV, fromV))
                {
                    auto* twin = adjList[toV][fromV];
                    twin->Twin = he;
                    he->Twin = twin;
                    he->Edge = twin->Edge;
                }
                else
                {
                    auto* edge = mesh->AllocateEdge();
                    he->Edge = edge;
                    edge->OneHE = he;
                }
                adjList[fromV][toV] = he;
            }

            for (size_t i = 0; i < faceHE.size(); i++)
            {
                auto* he = faceHE[i];
                auto* nextHe = faceHE[(i + 1) % faceHE.size()];
                he->Next = nextHe;
            }

            index_offset += fv;

            // per-face material
            shape.mesh.material_ids[f];
        }

        //Handle open mesh
    }

    return mesh;
}

int CDSLDemoCoreService::Setup()
{
    DemoScene = new Scene::CScene();
    DemoScene->CreateDefaultCamera();
    GApp->GetService<CRenderService>()->SetScene(DemoScene);
    return 0;
}

int CDSLDemoCoreService::FrameUpdate()
{
    {
        ImGui::Begin("DSL");
        if (ImGui::Button("Load Demo Patch"))
        {
            auto* node = DemoScene->GetRootNode()->CreateChildNode("patch");
            auto* mesh = HalfEdgeMeshFromObj("../../Application/Resources/patch.obj");
            node->SetEntity(new CHEMeshRenderEntity(mesh));
        }
        ImGui::End();
    }
    return 0;
}

int CDSLDemoCoreService::Cleanup()
{
    GApp->GetService<CRenderService>()->SetScene(nullptr);
    return 0;
}

}
