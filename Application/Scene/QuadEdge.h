#pragma once
#include <Vector3.h>
#include <cstdint>
#include <vector>

namespace Nome::Scene
{

class CQEMesh
{
    // Private internal types
    class CEdge
    {
    public:
        static void Splice(CEdge* a, CEdge* b);

        CEdge *GetNext() const { return Next; }
        void SetNext(CEdge *value) { Next = value; }

        uint32_t GetId() const { return IdAndIndex >> 2; }
        void SetId(uint32_t id) { IdAndIndex = (id << 2) & GetIndex(); }

        uint8_t GetIndex() const { return IdAndIndex & 0b11; }
        void SetIndex(uint8_t index) { IdAndIndex &= ~0b11u; IdAndIndex += index & 0b11u; }

        uint32_t GetVFIndex() const { return VFIndex; }
        void SetVFIndex(uint32_t idx) { VFIndex = idx; }

        // Edge algebra
        CEdge* Rot() { return GetIndex() < 3 ? this + 1 : this - 3; }
        CEdge* InvRot() { return GetIndex() > 0 ? this - 1 : this + 3; }
        CEdge* Sym() { return GetIndex() < 2 ? this + 2 : this - 2; }
        CEdge* Onext() { return Next; }
        CEdge* Oprev() { return Rot()->Onext()->Rot(); }
        CEdge* Dnext() { return Sym()->Onext()->Sym(); }
        CEdge* Dprev() { return InvRot()->Onext()->InvRot(); }
        CEdge* Lnext() { return InvRot()->Onext()->Rot(); }
        CEdge* Lprev() { return Onext()->Sym();}
        CEdge* Rnext() { return Rot()->Onext()->InvRot(); }
        CEdge* Rprev() { return Sym()->Onext(); }

        uint32_t GetOrg() { return VFIndex; }
        uint32_t GetDest() { return Sym()->GetOrg(); }
        uint32_t GetLeft() { return InvRot()->GetOrg(); }
        uint32_t GetRight() { return Rot()->GetOrg(); }
        void SetOrg(uint32_t idx) { VFIndex = idx; }
        void SetDest(uint32_t idx) { Sym()->SetOrg(idx); }
        void SetLeft(uint32_t idx) { InvRot()->SetOrg(idx); }
        void SetRight(uint32_t idx) { Rot()->SetOrg(idx); }

    private:
        CEdge* Next;
        uint32_t IdAndIndex; // 30 bit id for the whole quad edge. The lowest 2 bits are the index.
        uint32_t VFIndex = -1u; // An index for the origin vertex or face depending on primal or dual.
    };

    class CQuadEdge
    {
    public:
        CQuadEdge(uint32_t id);

        CEdge Comp[4];
    };

    class CVertex
    {
    public:
        CEdge* Edge = nullptr;
        tc::Vector3 Position;
    };

    class CFace
    {
    public:
        CEdge* Edge = nullptr;
        bool bIsBorder;
    };

public:
    CQEMesh() = default;
    ~CQEMesh() = default;

    void MakeEdge2V1F();
    void MakeEdge1V2F();

private:
    uint32_t NextQuadEdgeId = 0;
    std::vector<CVertex> Vertices;
    std::vector<CFace> Faces;
};

}
