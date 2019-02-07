#pragma once
#include <LightRefBase.h>
#include <memory>
#include <vector>

namespace Nome
{

struct CBlockAllocator
{
    void* Mem;
    void* p;
    size_t BytesLeft;

    CBlockAllocator(size_t size);
    ~CBlockAllocator();

    CBlockAllocator(const CBlockAllocator&) = delete;
    CBlockAllocator& operator=(const CBlockAllocator&) = delete;

    CBlockAllocator(CBlockAllocator&& o);
    CBlockAllocator& operator=(CBlockAllocator&& o);

    void* aligned_alloc(std::size_t a, std::size_t size);

    template <typename T>
    T* aligned_alloc(std::size_t a = alignof(T))
    {
        return aligned_alloc(a, sizeof(T));
    }
};

class AExpr;
class ACommand;

class CASTContext : public tc::TLightRefBase<CASTContext>
{
public:
    ~CASTContext();

    void* Alloc(size_t align, size_t size);

    void AppendCommand(ACommand* command)
    {
        Commands.push_back(command);
    }

    const std::vector<ACommand*>& GetCommands() const
    {
        return Commands;
    }

    AExpr* GetExpr() const;
    void SetExpr(AExpr* value);

private:
    std::vector<CBlockAllocator> Blocks;
    size_t NextBlockSize = 1024;

    //The AST represents a file, which consists of a list of commands
    std::vector<ACommand*> Commands;

    //An ASTContext can also represent a single expression
    AExpr* Expr;
};

}
