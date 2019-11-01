#include "ASTContext.h"

namespace Nome
{

CBlockAllocator::CBlockAllocator(size_t size)
    : BytesLeft(size)
{
    Mem = p = malloc(size);
}

CBlockAllocator::~CBlockAllocator()
{
    if (Mem)
        free(Mem);
}

CBlockAllocator::CBlockAllocator(CBlockAllocator&& o)
    : Mem(std::exchange(o.Mem, nullptr))
    , p(o.p)
    , BytesLeft(o.BytesLeft)
{
}

CBlockAllocator& CBlockAllocator::operator=(CBlockAllocator&& o)
{
    if (Mem)
        free(Mem);
    Mem = std::exchange(o.Mem, nullptr);
    p = o.p;
    BytesLeft = o.BytesLeft;
    return *this;
}

void* CBlockAllocator::aligned_alloc(std::size_t a, std::size_t size)
{
    if (std::align(a, size, p, BytesLeft))
    {
        void* result = p;
        p = (char*)p + size;
        BytesLeft -= size;
        return result;
    }
    return nullptr;
}

CASTContext::~CASTContext() {}

void* CASTContext::Alloc(size_t align, size_t size)
{
    if (Blocks.empty())
    {
        Blocks.emplace_back(NextBlockSize);
        NextBlockSize *= 2;
    }
    void* result = Blocks[Blocks.size() - 1].aligned_alloc(align, size);
    if (!result)
    {
        Blocks.emplace_back(NextBlockSize);
        NextBlockSize *= 2;
        result = Blocks[Blocks.size() - 1].aligned_alloc(align, size);
    }
    return result;
}

AExpr* CASTContext::GetExpr() const { return Expr; }

void CASTContext::SetExpr(AExpr* value) { Expr = value; }

}
