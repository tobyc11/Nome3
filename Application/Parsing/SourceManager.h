#pragma once
#include "Casting.h"
#include "StringBuffer.h"
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

namespace Nome::AST
{
class CASTContext;
class CToken;
class ACommand;
class AFile;
}

namespace Nome
{

struct CSourceLocation
{
    unsigned BufferId : 1;
    size_t Offset : 63;
};

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
    [[nodiscard]] AST::CASTContext& GetASTContext() { return *ASTContext; }
    [[nodiscard]] AST::AFile* GetAstRoot() const { return ASTRoot; }
    void SetAstRoot(AST::AFile* astRoot) { ASTRoot = astRoot; }

    CSourceLocation InsertText(size_t globalOffset, const std::string& text);
    void RemoveText(size_t globalOffset, size_t length);
    /// Replace text, will invalidate the original source location
    CSourceLocation ReplaceText(const CSourceLocation& location, size_t length, const std::string& text);
    [[nodiscard]] std::string CollectText() const;

    /// Given a global offset, find the piece containing it and the global offset for that piece
    [[nodiscard]] std::pair<size_t, size_t> FindPieceIndexAndOffset(size_t globalOffset) const;
    [[nodiscard]] static bool IsValidLocation(const CSourceLocation& location);
    [[nodiscard]] std::optional<size_t> BufOffsetToGlobal(unsigned bufId, size_t bufOffset) const;
    [[nodiscard]] std::optional<size_t> GetFileOffset(const CSourceLocation& location) const;
    [[nodiscard]] CSourceLocation GlobalToBufOffset(size_t globalOffset) const;
    std::optional<size_t> RemoveTokens(const std::vector<AST::CToken*>& tokenList);

    void SaveFile() const;

private:
    std::string MainSource;
    CStringBuffer MainSourceBuffer;
    /// Members of the tuple: buffer index, buffer offset, length
    std::vector<std::tuple<unsigned, size_t, size_t>> PieceTable;
    std::string AddBuffer;

    std::unique_ptr<AST::CASTContext> ASTContext;
    AST::AFile* ASTRoot {};
};

}
