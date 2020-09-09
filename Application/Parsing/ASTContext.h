#pragma once
#include "SyntaxTree.h"
#include <memory>
#include <utility>
#include <vector>

namespace Nome::AST
{

class CASTContext
{
public:
    CASTContext();

    template <typename T, typename... Args> T* Make(Args&&... args)
    {
        size_t i = 0;
        bool found = false;
        for (i = 0; i < Slabs.size(); ++i)
        {
            if (SlabLeft[i] > sizeof(T) * 2)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            Slabs.push_back(std::make_unique<char[]>(InitialSize));
            SlabLeft.push_back(InitialSize);
            SlabUsedCount.push_back(0);
            InitialSize *= 2;
        }
        void* p = Slabs[i].get() + SlabUsedCount[i];
        size_t size = SlabLeft[i];
        if (std::align(alignof(T), sizeof(T), p, size))
        {
            T* result = new (p) T(std::forward<Args>(args)...);
            size -= sizeof(T);
            SlabUsedCount[i] += SlabLeft[i] - size;
            SlabLeft[i] = size;
            return result;
        }
        return nullptr;
    }

    CToken* MakeToken(std::string identifier);
    AIdent* MakeIdent(std::string identifier);
    AVector* MakeVector(const std::vector<AExpr*>& children);

    [[nodiscard]] AFile* GetAstRoot() const { return ASTRoot; }
    void SetAstRoot(AFile* astRoot) { ASTRoot = astRoot; }

private:
    size_t InitialSize = 1024 * 32;
    std::vector<std::unique_ptr<char[]>> Slabs;
    std::vector<size_t> SlabUsedCount;
    std::vector<size_t> SlabLeft;
    AST::AFile* ASTRoot {};
};

}