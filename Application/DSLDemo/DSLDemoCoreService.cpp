#include "DSLDemoCoreService.h"
#include "HEMeshRenderEntity.h"
#include "DSLDemoRenderer.h"
#include "ObjLoader.h"

//Testing out IR
#include "CppIRBuilder.h"
#include "EffiCompiler.h"

#include "Nome/App.h"
#include "Nome/SDLService.h"
#include "Nome/RenderService.h"

#include "imgui.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <unordered_map>
#include <vector>

namespace Nome
{

static CEffiMesh* GlobalMesh = nullptr;

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
	GApp->GetService<CDSLDemoRenderer>()->SetCamera(DemoScene->GetMainCamera());
	//RenderService is broken rn
    //GApp->GetService<CRenderService>()->SetScene(DemoScene);

	EffiContext = new CEffiContext(GApp->GetService<CSDLService>()->RenderContext->GetGraphicsDevice());
	IRProgram* program = nullptr;
	//{
	//	using namespace CppIRBuilder;
	//	ScopedBuilderContext ctx;

	//	//Bend around origin
	//	Attr("pos") = InputAttr<Vector3>("pos");
	//	Attr("factor") = Const(0.5f) * Sqrt(Dot(Attr("pos"), Attr("pos")));
	//	auto& co = Cos(Attr("factor"));
	//	auto& si = Sin(Attr("factor"));
	//	auto& rotation = Mat3(co, -si, Const(0.0f),
	//		                  si, co, Const(0.0f),
	//		                  Const(0.0f), Const(0.0f), Const(1.0f));
	//	Attr("pos") = rotation * Attr("pos");
	//	MaterializeAttr("pos");

	//	program = ctx.GetProgram();
	//}
	//{
	//	using namespace CppIRBuilder;
	//	ScopedBuilderContext ctx;

	//	//Collapse into a ball
	//	Attr("pos") = InputAttr<Vector3>("pos");
	//	Attr("dist_from_center") = Sqrt(Dot(Attr("pos"), Attr("pos")));
	//	Attr("pos") = Attr("pos") / Attr("dist_from_center");
	//	MaterializeAttr("pos");

	//	program = ctx.GetProgram();
	//}
	//{
	//	using namespace CppIRBuilder;
	//	ScopedBuilderContext ctx;
	//	Attr("pos") = InputAttr<Vector3>("pos");
	//	Attr("normal") = Matrix3::IDENTITY * Attr("pos") + Vector3(-0.1f, 0.1f, 0.05f);
	//	MaterializeAttr("normal");
	//	Attr("dir") = Matrix3{ 0, 1, 0, 1, 0, 0, 0, 0, 1 } *Attr("pos");
	//	Offset("dir");
	//	SubdivideCatmullClark();

	//	program = ctx.GetProgram();
	//}
	{
		using namespace CppIRBuilder;
		ScopedBuilderContext ctx;

		//Bend around origin
		Attr("pos") = InputAttr<Vector3>("pos");
		Attr("pos") = Const(Matrix3::IDENTITY * 2.0f) * Attr("pos");
		MaterializeAttr("pos");

		program = ctx.GetProgram();
	}

	CEffiCompiler compiler{ EffiContext };
	CompiledPipeline = compiler.Compile(program);

    return 0;
}

int CDSLDemoCoreService::FrameUpdate()
{
    {
        ImGui::Begin("DSL");
		if (ImGui::Button("Load Demo Patch"))
		{
			if (GlobalMesh)
			{
				GApp->GetService<CDSLDemoRenderer>()->SetRenderMesh(nullptr);
				delete GlobalMesh;
				GlobalMesh = nullptr;
			}

			CObjLoader loader{ "Resources/patch.obj" };
			GlobalMesh = loader.LoadEffiMesh(GApp->GetService<CSDLService>()->RenderContext->GetGraphicsDevice());
			GApp->GetService<CDSLDemoRenderer>()->SetRenderMesh(GlobalMesh);
		}
        if (ImGui::Button("Load Demo Mesh"))
        {
			if (GlobalMesh)
			{
				GApp->GetService<CDSLDemoRenderer>()->SetRenderMesh(nullptr);
				delete GlobalMesh;
				GlobalMesh = nullptr;
			}

			CObjLoader loader{ "Resources/monkey.obj" };
			GlobalMesh = loader.LoadEffiMesh(GApp->GetService<CSDLService>()->RenderContext->GetGraphicsDevice());
			GApp->GetService<CDSLDemoRenderer>()->SetRenderMesh(GlobalMesh);
        }
		if (ImGui::Button("Apply Operator"))
		{
			if (GlobalMesh)
				CompiledPipeline->operator()(*GlobalMesh);
		}
		DemoScene->GetMainCamera()->ShowDebugImGui();
        ImGui::End();
    }
    return 0;
}

int CDSLDemoCoreService::Cleanup()
{
    //GApp->GetService<CRenderService>()->SetScene(nullptr);
    return 0;
}

}
