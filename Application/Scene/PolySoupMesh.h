#pragma once
#include <Vector3.h>
#include <vector>

namespace Nome::Scene
{

class CPolySoupMesh
{
public:
    typedef size_t VertexHandle;
    typedef size_t FaceHandle;

    class VertexIter
    {
    public:
        VertexIter() = default;
        VertexIter(VertexHandle vh) : Handle(vh) {}
        // Default copy, assign, and destructor

        VertexIter& operator++() { ++Handle; return *this; }
        const VertexHandle& operator*() { return Handle; }

        friend bool operator==(const VertexIter& lhs, const VertexIter& rhs) { return lhs.Handle == rhs.Handle; }
        friend bool operator!=(const VertexIter& lhs, const VertexIter& rhs) { return lhs.Handle != rhs.Handle; }

    private:
        VertexHandle Handle;
    };

    class FaceIter
    {
    public:
        FaceIter() = default;
        FaceIter(FaceHandle fh) : Handle(fh) {}
        // Default copy, assign, and destructor

        FaceIter& operator++() { ++Handle; return *this; }
        const FaceHandle& operator*() { return Handle; }

        friend bool operator==(const FaceIter& lhs, const FaceIter& rhs) { return lhs.Handle == rhs.Handle; }
        friend bool operator!=(const FaceIter& lhs, const FaceIter& rhs) { return lhs.Handle != rhs.Handle; }

    private:
        FaceHandle Handle;
    };

    class FaceVertexIter
    {
    public:
        FaceVertexIter() = default;
        FaceVertexIter(const CPolySoupMesh* mesh, FaceHandle fh) : Mesh(mesh), Fh(fh) {}
        FaceVertexIter(const CPolySoupMesh& mesh, FaceHandle fh) : Mesh(&mesh), Fh(fh) {}
        FaceVertexIter(const CPolySoupMesh* mesh, FaceHandle fh, size_t index) : Mesh(mesh), Fh(fh), Index(index) {}
        // Default copy, assign, and destructor

        FaceVertexIter& operator++() { ++Index; return *this; }
        VertexHandle operator*()
        {
            return Mesh->Faces[Fh].FaceVerts[Index];
        }

        friend bool operator==(const FaceVertexIter& lhs, const FaceVertexIter& rhs) { return lhs.Index == rhs.Index; }
        friend bool operator!=(const FaceVertexIter& lhs, const FaceVertexIter& rhs) { return lhs.Index != rhs.Index; }

        bool is_valid() const
        {
            return Index < Mesh->Faces[Fh].FaceVerts.size();
        }

    private:
        const CPolySoupMesh* Mesh = nullptr;
        FaceHandle Fh;
        size_t Index = 0;
    };

    class VertexRange
    {
    public:
        VertexRange(const CPolySoupMesh& mesh) : Mesh(mesh) {}

        VertexIter begin() { return Mesh.vertices_begin(); }
        VertexIter end() { return Mesh.vertices_end(); }

    private:
        const CPolySoupMesh& Mesh;
    };

    class FVRange
    {
    public:
        FVRange(const CPolySoupMesh& mesh, FaceHandle fh) : Mesh(mesh), Fh(fh) {}

        FaceVertexIter begin() { return Mesh.fv_begin(Fh); }
        FaceVertexIter end() { return Mesh.fv_end(Fh); }

    private:
        const CPolySoupMesh& Mesh;
        FaceHandle Fh;
    };

private:
    struct CVertex
    {
        CVertex(tc::Vector3 pos) : Position(pos) {}
        tc::Vector3 Position;
    };

    struct CFace
    {
        CFace(std::vector<VertexHandle> verts, tc::Vector3 normal, bool del) : FaceVerts(verts), Normal(normal), bIsDeleted(del) {}
        std::vector<VertexHandle> FaceVerts;
        tc::Vector3 Normal;
        bool bIsDeleted;
    };

public:
    CPolySoupMesh();

    VertexHandle add_vertex(const tc::Vector3& pos);
    FaceHandle add_face(const std::vector<VertexHandle>& facePoints);
    void delete_face(FaceHandle fh, bool placeHolder);
    void clear();

    VertexIter vertices_begin() const { return VertexIter(0); }
    VertexIter vertices_end() const { return VertexIter(Vertices.size()); }
    VertexRange vertices() const { return VertexRange(*this); }
    FaceIter faces_begin() const { return FaceIter(0); }
    FaceIter faces_end() const { return FaceIter(Faces.size()); }
    FaceVertexIter fv_begin(FaceHandle fh) const { return FaceVertexIter(this, fh); }
    FaceVertexIter fv_end(FaceHandle fh) const { return FaceVertexIter(this, fh, Faces[fh].FaceVerts.size()); }
    FVRange fv_range(FaceHandle fh) const { return FVRange(*this, fh); }

    std::vector<float> point(VertexHandle vh) const;
    std::vector<float> normal(FaceHandle fh) const;

private:
    std::vector<CVertex> Vertices;
    std::vector<CFace> Faces;
};

}
