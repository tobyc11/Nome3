#include "SourceManager.h"
#include "NomLexer.h"
#include "NomParser.h"
#include "SyntaxTreeBuilder.h"
#include "antlr4-runtime.h"
#include <stack>
#include <utility>

namespace Nome
{

using namespace antlrcpp;
using namespace antlr4;

const int CSourceManager::OrigBuf;
const int CSourceManager::AddBuf;

class CMyErrorListener : public BaseErrorListener
{
public:
    bool bDidErrorHappen = false;

private:
    void syntaxError(Recognizer* recognizer, Token* offendingSymbol, size_t line,
                     size_t charPositionInLine, const std::string& msg,
                     std::exception_ptr e) override
    {
        std::cout << "line " << line << ":" << charPositionInLine << " " << msg << std::endl;
        bDidErrorHappen = true;
    }
};

CSourceManager::CSourceManager(std::string mainSource)
    : MainSource(std::move(mainSource))
{
}

bool CSourceManager::ParseMainSource()
{
    std::ifstream ifs(MainSource);
    std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    ifs.close();

    MainSourceBuffer = CStringBuffer(content);
    PieceTable.emplace_back(OrigBuf, 0, content.length());

    ANTLRInputStream input(content);
    NomLexer lexer(&input);
    CommonTokenStream tokens(&lexer);

    NomParser parser(&tokens);
    CMyErrorListener errorListener;
    parser.addErrorListener(&errorListener);
    auto* tree = parser.file();

    CFileBuilder builder(MainSourceBuffer);
    ASTRoot = builder.visitFile(tree);
    ASTContext.SetAstRoot(ASTRoot);

    std::cout << "====== Debug Print AST ======" << std::endl;
    std::cout << *ASTRoot;
    std::cout << "====== End Debug Print AST ======" << std::endl;

    return !errorListener.bDidErrorHappen;
}

void CSourceManager::InsertText(size_t globalOffset, const std::string& text)
{
    auto [pieceIndex, currOff] = FindPieceIndexAndOffset(globalOffset);

    // Append new text into append buffer
    size_t addBufStart = AddBuffer.length();
    AddBuffer.append(text);

    // Split and make 3 new pieces
    auto [bufId, bufStart, origLen] = PieceTable[pieceIndex];
    PieceTable.erase(PieceTable.begin() + pieceIndex);
    std::vector<std::tuple<int, size_t, size_t>> newPieces;
    if (globalOffset != currOff)
        newPieces.emplace_back(bufId, bufStart, globalOffset - currOff);
    newPieces.emplace_back(AddBuf, addBufStart, text.length());
    newPieces.emplace_back(bufId, bufStart + globalOffset - currOff,
                           origLen - (globalOffset - currOff));
    PieceTable.insert(PieceTable.begin() + pieceIndex, newPieces.begin(), newPieces.end());
}

void CSourceManager::RemoveText(size_t globalOffset, size_t length)
{
    auto [pieceIndex, pieceStart] = FindPieceIndexAndOffset(globalOffset);
    auto [bufId, bufStart, pieceLen] = PieceTable[pieceIndex];
    auto offsetInPiece = globalOffset - pieceStart;
    if (offsetInPiece + length <= pieceLen)
    {
        // Removal only affects this piece, might need to split
        // This is the base case where the removed piece does not offshoot
        std::vector<std::tuple<int, size_t, size_t>> newPieces;
        if (offsetInPiece != 0)
            newPieces.emplace_back(bufId, bufStart, offsetInPiece);
        if (offsetInPiece + length < pieceLen)
            newPieces.emplace_back(bufId, bufStart + offsetInPiece + length,
                                   pieceLen - offsetInPiece - length);

        if (newPieces.empty())
            PieceTable.erase(PieceTable.begin() + pieceIndex);
        else if (newPieces.size() == 1)
            PieceTable[pieceIndex] = newPieces[0];
        else
        {
            PieceTable.erase(PieceTable.begin() + pieceIndex);
            PieceTable.insert(PieceTable.begin() + pieceIndex, newPieces.begin(), newPieces.end());
        }
    }
    else
    {
        auto lenWithin = pieceLen - offsetInPiece;
        auto restLen = length - lenWithin;
        RemoveText(globalOffset, lenWithin);
        RemoveText(globalOffset, restLen);
    }
}

std::string CSourceManager::CollectText() const
{
    std::string result;
    const auto& origBuf = MainSourceBuffer.GetAsString();
    for (auto [bufId, bufOffset, pieceLen] : PieceTable)
    {
        // GetPieceText
        if (bufId == OrigBuf)
        {
            result += origBuf.substr(bufOffset, pieceLen);
        }
        else if (bufId == AddBuf)
        {
            result += AddBuffer.substr(bufOffset, pieceLen);
        }
        else
            throw std::runtime_error("bufId corrupted in piece table");
    }
    return result;
}

std::pair<size_t, size_t> CSourceManager::FindPieceIndexAndOffset(size_t globalOffset) const
{
    // GetPieceIndex
    size_t currOff = 0;
    size_t pieceIndex = 0;
    bool bFound = false;
    for (auto [bufId, start, len] : PieceTable)
    {
        auto nextOff = currOff + len;
        if (globalOffset >= currOff && globalOffset < nextOff)
        {
            bFound = true;
            break;
        }
        currOff = nextOff;
        pieceIndex++;
    }
    if (!bFound)
        throw std::runtime_error("Could not insert text because global offset is out of range");
    return { pieceIndex, currOff };
}

std::optional<size_t> CSourceManager::BufOffsetToGlobal(int bufId, size_t bufOffset) const
{
    size_t currLen = 0;
    for (auto [pieceBuf, start, len] : PieceTable)
    {
        if (pieceBuf == bufId && start <= bufOffset && bufOffset < start + len)
            return currLen + bufOffset - start;
        currLen += len;
    }
    return {};
}

std::pair<int, size_t> CSourceManager::GlobalToBufOffset(size_t globalOffset) const
{
    auto [pieceIndex, pieceOffset] = FindPieceIndexAndOffset(globalOffset);
    auto [bufId, bufOffset, pieceLen] = PieceTable[pieceIndex];
    return { bufId, bufOffset + pieceOffset };
}

std::optional<size_t> CSourceManager::RemoveTokens(const std::vector<AST::CToken*>& tokenList)
{
    std::optional<size_t> retVal;
    for (auto* token : tokenList)
    {
        if (token->IsLocInvalid())
            continue; // Or should it err?
        const auto& [bufId, bufOffset] = token->GetLocation();
        size_t globalOffset = BufOffsetToGlobal(bufId, bufOffset).value();
        if (!retVal.has_value())
            retVal = globalOffset;
        size_t length = token->ToString().length();
        RemoveText(globalOffset, length);
    }
    return retVal;
}

size_t CSourceManager::InsertToken(size_t globalOffset, AST::CToken* token)
{
    return InsertToken(globalOffset, token, "", "");
}

size_t CSourceManager::InsertToken(size_t globalOffset, AST::CToken* token,
                                   const std::string& before, const std::string& after)
{
    token->SetLocation(AddBuf, AddBuffer.length() + before.length());
    auto textToInsert = before + token->ToString() + after;
    InsertText(globalOffset, textToInsert);
    return globalOffset + textToInsert.length();
}

bool CSourceManager::AppendCmdAfter(AST::ACommand* parent, AST::ACommand* after,
                                    AST::ACommand* newCommand)
{
    std::vector<AST::CToken*> tokenList;
    after->CollectTokens(tokenList);

    if (tokenList.empty())
        return false;

    AST::CToken* afterToken = tokenList.back();
    const auto& afterLoc = afterToken->GetLocation();
    auto optGlobalOff = BufOffsetToGlobal(afterLoc.BufId, afterLoc.Start);
    if (!optGlobalOff)
        return false;

    tokenList.clear();
    newCommand->CollectTokens(tokenList);
    if (tokenList.empty())
        return false;

    for (auto* t : tokenList)
        if (!t->IsLocInvalid())
            return false;

    if (!parent->AddChildAfter(after, newCommand))
        return false;

    auto off = optGlobalOff.value() + afterToken->ToString().length();
    for (auto* t : tokenList)
    {
        off = InsertToken(off, t, " ", "");
    }
    return true;
}

bool Nome::CSourceManager::AppendCmdEndOfFile(Nome::AST::ACommand* newCommand)
{
    size_t offset = 0;
    if (!ASTRoot->GetCommands().empty())
    {
        std::vector<AST::CToken*> tokenList;
        ASTRoot->GetCommands().back()->CollectTokens(tokenList);

        AST::CToken* afterToken = tokenList.back();
        const auto& afterLoc = afterToken->GetLocation();
        auto optGlobalOff = BufOffsetToGlobal(afterLoc.BufId, afterLoc.Start);
        if (!optGlobalOff)
            return false;
        offset = optGlobalOff.value() + afterToken->ToString().length();
    }

    std::vector<AST::CToken*> tokenList;
    newCommand->CollectTokens(tokenList);

    if (tokenList.empty())
        return false;

    for (auto* t : tokenList)
        if (!t->IsLocInvalid())
            return false;

    ASTRoot->AddChild(newCommand);

    bool first = true;
    for (auto* t : tokenList)
    {
        if (first)
            offset = InsertToken(offset, t, "\n", "");
        else
            offset = InsertToken(offset, t, " ", "");
        first = false;
    }
    return true;
}

void CSourceManager::CommitASTChanges() { }

void CSourceManager::SaveFile() const
{
    auto content = CollectText();
    std::ofstream ofs(GetMainSourcePath());
    ofs << content;
}

}
