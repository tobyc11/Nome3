#pragma once
#include "ASTContext.h"
#include "StringBuffer.h"
#include "SyntaxTree.h"
#include <fstream>
#include <optional>
#include <string>
#include <tuple>
#include <unordered_map>
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
    void ReportErros(std::string code); 
    bool balancedbracket(std::string expr);
    bool ParameterCheck(std::vector<std::string> code, std::string type, int numparams, std::unordered_map<std::string, std::string> idmap);
    std::vector<std::string> CheckStatement(std::vector<std::vector<std::string>> parsedcode, std::unordered_map<std::string, std::string> idmap, std::string endstatement, int i, int j, std::unordered_map<std::string, std::string> shapemap);
    std::vector<std::string> CheckInstance(std::vector<std::vector<std::string>> parsedcode, std::unordered_map<std::string, std::string> idmap, int i, int j, std::unordered_map<std::string, std::string> shapemap);
    std::vector<std::string> CheckGroup(std::vector<std::vector<std::string>> parsedcode, std::unordered_map<std::string, std::string>& idmap, int i, int j, std::unordered_map<std::string, std::string> shapemap);
    std::vector<std::string> CheckBank(std::vector<std::vector<std::string>> parsedcode, std::unordered_map<std::string, std::string> &referencemap, std::unordered_map<std::string, std::string> &idmap, int i, int j, std::unordered_map<std::string, std::string> shapemap);
    std::vector<std::string> CheckSubdivision(std::vector<std::vector<std::string>> parsedcode, std::unordered_map<std::string, std::string> &idmap, int i, int j, std::unordered_map<std::string, std::string> shapemap);
    std::vector<std::string> CheckMesh(std::vector<std::vector<std::string>> parsedcode, std::unordered_map<std::string, std::string> &idmap, int i, int j, std::unordered_map<std::string, std::string> shapemap);
    std::vector<std::string> CheckCircle(std::vector<std::vector<std::string>> parsedcode, std::unordered_map<std::string, std::string> &idmap, int i, int j, std::unordered_map<std::string, std::string> shapemap);
    std::vector<std::string> CheckPoint(std::vector<std::vector<std::string>> parsedcode, std::unordered_map<std::string, std::string> &idmap, int i, int j, std::unordered_map<std::string, std::string> shapemap);
    std::vector<std::string> CheckFace(std::vector<std::vector<std::string>> parsedcode, std::unordered_map<std::string, std::string> &idmap, int i, int j, std::unordered_map<std::string, std::string> shapemap);
    std::string RemoveSpecials(std::string str);
    bool isNumber(std::string s);
    int checkcount(std::string str, char letter);
    [[nodiscard]] std::string CollectText() const;
    [[nodiscard]] std::pair<size_t, size_t> FindPieceIndexAndOffset(size_t globalOffset) const;

    [[nodiscard]] std::optional<size_t> BufOffsetToGlobal(int bufId, size_t bufOffset) const;
    [[nodiscard]] std::pair<int, size_t> GlobalToBufOffset(size_t globalOffset) const;
    std::optional<size_t> RemoveTokens(const std::vector<AST::CToken*>& tokenList);
    // Inserts a token into the text buffer, returns the pos after last inserted character
    size_t InsertToken(size_t globalOffset, AST::CToken* token);
    size_t InsertToken(size_t globalOffset, AST::CToken* token, const std::string& before,
                       const std::string& after);

    // Requires that the new command is not in the text yet
    bool AppendCmdAfter(AST::ACommand* parent, AST::ACommand* after, AST::ACommand* newCommand);
    bool AppendCmdEndOfFile(AST::ACommand* newCommand);

    bool AppendText(const std::string& text); // Steven's Add Point
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