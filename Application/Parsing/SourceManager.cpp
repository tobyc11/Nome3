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

void CSourceManager::InsertToken(size_t globalOffset, AST::CToken* token) {
    token->SetLocation(AddBuf, AddBuffer.length());
    InsertText(globalOffset, token->ToString());
}

void CSourceManager::CommitASTChanges()
{
    std::stack<AST::ACommand*> dfsStack;
    for (auto* cmd : GetASTContext().GetAstRoot()->GetCommands())
        dfsStack.push(cmd);
    while (!dfsStack.empty())
    {
        auto* cmd = dfsStack.top();
        dfsStack.pop();
        for (auto* subCmd : cmd->GetSubCommands())
            dfsStack.push(subCmd);
        if (cmd->IsPendingSave())
        {
            cmd->SetPendingSave(false);
            // How to commit an AST node?
            // 1. Gather all the tokens to form a token vector
            std::vector<AST::CToken*> tokenList;
            cmd->CollectTokens(tokenList);
            std::cout << "---Begin Printing Token List---" << std::endl;
            for (auto* token : tokenList)
                std::cout << token->ToString() << std::endl;
            if (tokenList.empty())
                continue;
            // We either rewrite the entire command, or only specific tokens
            if (!tokenList[0]->IsLocInvalid())
            {
                // Token list contains valid items, let's only rewrite specific ones
                throw std::runtime_error("case unhandled");
            }
            else
            {
                // 2. Generate a string from that token vector, and set token locations
                std::string content;
                for (auto* token : tokenList)
                {
                    content.append(" ");
                    token->SetLocation(1, AddBuffer.length() + content.length());
                    content.append(token->ToString());
                }
                content.append("\n"); // What do we do with CRLF platforms?
                // 3. Insert new string into piece table
                // Find out where to insert the text
                size_t insertOffset = 0;
                if (!dfsStack.empty())
                {
                    // If the stack is empty, then we are the top node, so insert at 0
                    // otherwise insert at the end of the previous node
                    auto* prevToken = dfsStack.top()->GetCloseToken();
                    if (!prevToken)
                    {
                        // The previous node is probably a set or something that doesn't have endset
                        // so we are forced to gather the whole token list and fetch the last
                        // element
                        std::vector<AST::CToken*> tokenListPrev;
                        dfsStack.top()->CollectTokens(tokenListPrev);
                        assert(!tokenListPrev.empty());
                        prevToken = *(tokenListPrev.end() - 1);
                    }
                    auto [bufId, bufOffset] = prevToken->GetLocation();
                    insertOffset = BufOffsetToGlobal(bufId, bufOffset).value();
                    insertOffset += prevToken->ToString().length();
                }
                InsertText(insertOffset, content);
            }
        }
    }
}

void CSourceManager::SaveFile() const
{
    auto content = CollectText();
    std::ofstream ofs(GetMainSourcePath());
    ofs << content;
}

}
