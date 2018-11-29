#pragma once

#include <Vector3.h>

#include <AutoPtr.h>

#include <vector>
#include <unordered_set>

namespace Nome
{

using tc::Vector3;

template <typename T>
class TArrayBucketAllocator
{
public:
    explicit TArrayBucketAllocator(uint32_t size) : Size(size)
    {
        ArrayBucket = (T*)malloc(sizeof(T) * Size);

        static_assert(sizeof(T) >= sizeof(uint32_t), "Block size is too small to store a linked list");

        //Unused array blocks form a singly linked list of free blocks
        //The linked list head is stored in FirstFreeIndex
        FirstFreeIndex = 0;
        for (uint32_t i = 0; i < Size; i++)
        {
            *(uint32_t*)(ArrayBucket + i) = i + 1;
        }
    }

    ~TArrayBucketAllocator()
    {
        free(ArrayBucket);
    }

    T* Allocate()
    {
        if (IsFull())
            return nullptr;
        T* result = ArrayBucket + FirstFreeIndex;
        FirstFreeIndex = *(uint32_t*)(ArrayBucket + FirstFreeIndex);
        return result;
    }

    void Free(T* p)
    {
        ptrdiff_t index = p - ArrayBucket;
        *(uint32_t*)(p) = FirstFreeIndex;
        FirstFreeIndex = (uint32_t)index;
    }

    bool TryFree(T* p)
    {
        ptrdiff_t index = p - ArrayBucket;
        if (index >= 0 && index < Size)
        {
            Free(p);
            return true;
        }
        return false;
    }

    bool IsFull() const
    {
        return FirstFreeIndex == Size;
    }

    size_t SizeOfDynamicData() const
    {
        return Size * sizeof(T);
    }

private:
    T* ArrayBucket;
    uint32_t FirstFreeIndex;
    uint32_t Size;
};

template <typename T>
class TGraduallyGrowingMemoryPool
{
public:
    T* Allocate()
    {
        //Go through all the buckets to find a free spot
        for (auto& bucket : Buckets)
        {
            if (!bucket.IsFull())
                return bucket.Allocate();
        }

        //Need new bucket
        Buckets.emplace_back(NextBucketSize);
        NextBucketSize *= 2;

        return Buckets.back().Allocate();
    }

    void Free(T* p)
    {
        for (auto& bucket : Buckets)
        {
            if (bucket.TryFree(p))
                break;
        }
    }

    size_t SizeOfDynamicData() const
    {
        size_t result = 0;
        for (auto& bucket : Buckets)
        {
            result += bucket.SizeOfDynamicData();
        }
        return result;
    }

private:
    std::vector<TArrayBucketAllocator<T>> Buckets;
    uint32_t NextBucketSize = 256;
};

//A good reference: http://kaba.hilvi.org/homepage/blog/halfedge/halfedge.htm

class CHEMesh : public tc::FRefCounted
{
public:
    struct HalfEdge;

    struct Vertex
    {
        Vector3 Position;
        HalfEdge* OneHE;
    };

    struct Edge
    {
        HalfEdge* OneHE;
    };

    struct Face
    {
        HalfEdge* OneHE;
        bool bIsBoundary;
    };

    struct HalfEdge
    {
        Vertex* Vert;
        Edge* Edge;
        Face* Face;
        HalfEdge* Twin;
        HalfEdge* Next;

        bool IsFree() const
        {
            return !Face || Face->bIsBoundary;
        }
    };

    CHEMesh();

    //Low level memory allocation methods
    Vertex* AllocateVertex();
    Edge* AllocateEdge();
    Face* AllocateFace();
    HalfEdge* AllocateHalfEdge();
    void FreeVertex(Vertex* p);
    void FreeEdge(Edge* p);
    void FreeFace(Face* p);
    void FreeHalfEdge(HalfEdge* p);

    size_t SizeOfDynamicData() const
    {
        return VertexPool.SizeOfDynamicData() + EdgePool.SizeOfDynamicData() +
            FacePool.SizeOfDynamicData() + HalfEdgePool.SizeOfDynamicData() +
            sizeof(void*) * (Vertices.size() + Edges.size() + Faces.size() + HalfEdges.size() + 1);
    }

    /*
     * Operations
    */
    ///Find an outgoing half-edge that doesn't have a face attached
    HalfEdge* FindFreeIncident(Vertex* v);

    ///Creates an isolated vertex
    Vertex* MakeVertex(const Vector3& position);

    ///Make edge and vertex from a given vertex
    Edge* MakeEdgeVertex(Vertex* v0, const Vector3& position, Face* left, Face* right);
    Edge* MakeEdgeFace(Face* face, Vertex* v0, Vertex* v1);

    void CreateHalfEdgePair(HalfEdge*& outPtr, HalfEdge*& outPtr2);

public:
    /*
     * Query
    */
    std::vector<Vertex*> VVQuery(Vertex* v);
    std::vector<Edge*> VEQuery(Vertex* v);
    std::vector<Face*> VFQuery(Vertex* v);
    std::vector<Vertex*> EVQuery(Edge* e);
    std::vector<Edge*> EEQuery(Edge* e);
    std::vector<Face*> EFQuery(Edge* e);
    std::vector<Vertex*> FVQuery(Face* f);
    std::vector<Edge*> FEQuery(Face* f);
    std::vector<Face*> FFQuery(Face* f);
    HalfEdge* NextAroundVertex(HalfEdge* he);
    HalfEdge* PrevAroundVertex(HalfEdge* he);
    bool AreAdjacent(Vertex* v1, Vertex* v2);

    std::unordered_set<HalfEdge*>::iterator HalfEdgesBegin() { return HalfEdges.begin(); }
    std::unordered_set<HalfEdge*>::iterator HalfEdgesEnd() { return HalfEdges.end(); }

    std::unordered_set<Face*>::iterator FacesBegin() { return Faces.begin(); }
    std::unordered_set<Face*>::iterator FacesEnd() { return Faces.end(); }

private:
    TGraduallyGrowingMemoryPool<Vertex> VertexPool;
    TGraduallyGrowingMemoryPool<Edge> EdgePool;
    TGraduallyGrowingMemoryPool<Face> FacePool;
    TGraduallyGrowingMemoryPool<HalfEdge> HalfEdgePool;

    std::unordered_set<Vertex*> Vertices;
    std::unordered_set<Edge*> Edges;
    std::unordered_set<Face*> Faces;
    std::unordered_set<HalfEdge*> HalfEdges;

    std::unordered_set<Face*> BoundaryFaces;
};

}
