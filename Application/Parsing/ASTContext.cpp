#include "ASTContext.h"

namespace Nome::AST
{

CASTContext::CASTContext(CSourceManager* sourceMgr)
    : SourceMgr(sourceMgr)
{
    Slabs.push_back(std::make_unique<char[]>(InitialSize));
    SlabLeft.push_back(InitialSize);
    SlabUsedCount.push_back(0);
    InitialSize *= 2;
}

void* CASTContext::Allocate(size_t bytes, size_t alignment) const
{
    size_t i = 0;
    bool found = false;
    for (i = 0; i < Slabs.size(); ++i)
    {
        // Generously double the requirement to account for potential alignment requirement
        if (SlabLeft[i] > bytes * 2)
        {
            found = true;
            break;
        }
    }
    if (!found)
    {
        while (InitialSize < bytes * 2)
            InitialSize *= 2;
        Slabs.push_back(std::make_unique<char[]>(InitialSize));
        SlabLeft.push_back(InitialSize);
        SlabUsedCount.push_back(0);
        InitialSize *= 2;
    }
    void* p = Slabs[i].get() + SlabUsedCount[i];
    size_t size = SlabLeft[i];
    if (std::align(alignment, bytes, p, size))
    {
        size -= bytes;
        SlabUsedCount[i] += SlabLeft[i] - size;
        SlabLeft[i] = size;
        return p;
    }
    return nullptr;
}

}
