#pragma once
#include "ASTContext.h"
#include "StringBuffer.h"
#include "SyntaxTree.h"
#include <optional>
#include <string>
#include <tuple>
#include <vector>

namespace Nome
{

// Abstracts away all the file management mess so that we can focus on
//   the high level bits.
class CSourceManager
{
public:
    static const int OrigBuf = 0;
    static const int AddBuf = 1;

    explicit CSourceManager(std::string mainSource);

    bool ParseMainSource();

    [[nodiscard]] const std::string& GetMainSourcePath() const { return MainSource; }
    [[nodiscard]] AST::CASTContext& GetASTContext() { return ASTContext; }

    void InsertText(size_t globalOffset, const std::string& text);
    void RemoveText(size_t globalOffset, size_t length);
    [[nodiscard]] std::string CollectText() const;
    [[nodiscard]] std::pair<size_t, size_t> FindPieceIndexAndOffset(size_t globalOffset) const;

    [[nodiscard]] std::optional<size_t> BufOffsetToGlobal(int bufId, size_t bufOffset) const;
    [[nodiscard]] std::pair<int, size_t> GlobalToBufOffset(size_t globalOffset) const;

    void CommitASTChanges();
    void SaveFile() const;

private:
    std::string MainSource;
    CStringBuffer MainSourceBuffer;
    std::vector<std::tuple<int, size_t, size_t>> PieceTable;
    std::string AddBuffer;

    AST::CASTContext ASTContext;
    AST::AFile* ASTRoot {};
};

}
