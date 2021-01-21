#pragma once
#include "SourceManager.h"
#include <memory>
#include <utility>
#include <vector>

namespace Nome::AST
{

/// This class manages memory for an AST
class CASTContext
{
public:
    explicit CASTContext(CSourceManager* sourceMgr);

    /// Allocate memory from one of the exponentially growing slabs
    [[nodiscard]] void* Allocate(size_t bytes, size_t alignment) const;

    /// We don't free any memory until the whole ASTContext is destructed
    void Deallocate(void* ptr) const { }

    CSourceManager* GetSourceMgr() const { return SourceMgr; }

private:
    mutable size_t InitialSize = 1024 * 32;
    mutable std::vector<std::unique_ptr<char[]>> Slabs;
    mutable std::vector<size_t> SlabUsedCount;
    mutable std::vector<size_t> SlabLeft;

    /// A weak pointer to the owning source manager
    CSourceManager* SourceMgr;
};

} // namespace Nome::AST

inline void* operator new(size_t bytes, const Nome::AST::CASTContext& ctx, size_t alignment = 8)
{
    return ctx.Allocate(bytes, alignment);
}

inline void operator delete(void* ptr, const Nome::AST::CASTContext& ctx, size_t) { ctx.Deallocate(ptr); }

inline void* operator new[](size_t bytes, const Nome::AST::CASTContext& ctx, size_t alignment = 8)
{
    return ctx.Allocate(bytes, alignment);
}

inline void operator delete[](void* ptr, const Nome::AST::CASTContext& ctx, size_t) { ctx.Deallocate(ptr); }
