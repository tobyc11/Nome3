#include "SourceManager.h"
#include "NomLexer.h"
#include "NomParser.h"
#include "SyntaxTreeBuilder.h"
#include "antlr4-runtime.h"
#include <unordered_map>
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
    ReportErros(content);

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

void CSourceManager::ReportErros(std::string code) {
    size_t pos = 0; 
    std::string delimiter = "\n";
    std::string token; 
    std::vector<std::vector<std::string>> parsedcode;
    std::string delimiter2 = " "; 
    std::string token2; 
    std::unordered_map<std::string, std::string> shapemap;
    std::unordered_map<std::string, std::string> idmap;
    std::unordered_map<std::string, std::string> referencemap;
    shapemap["surface"] = "endsurface";
    shapemap["bank"] = "endbank";
    shapemap["point"] = "endpoint";
    shapemap["controlpoint"] = "endcontrolpoint";
    shapemap["polyline"] = "endpolyline";
    shapemap["face"] = "endface";
    shapemap["beziercurve"] = "endbeziercurve";
    shapemap["bspline"] = "endbspline";
    shapemap["mesh"] = "endmesh";
    shapemap["circle"] = "endcircle";
    shapemap["disk"] = "enddisk";
    shapemap["cylinder"] = "endcylinder";
    shapemap["funnel"] = "endfunnel";
    shapemap["tunnel"] = "endtunnel";
    shapemap["sphere"] = "endsphere";
    shapemap["ellipsoid"] = "endellipsoid";
    shapemap["torus"] = "endtorus";
    shapemap["torusknot"] = "endtorusknot";
    shapemap["mobiusstrip"] = "endmobiusstrip";
    shapemap["dupin"] = "enddupin";
    shapemap["sweep"] = "endsweep";
    shapemap["instance"] = "endinstance";
    shapemap["group"] = "endgroup";
    shapemap["subdivision"] = "endsubdivision";
    shapemap["offset"] = "endoffset";
    shapemap["background"] = "endbackground";
    shapemap["foreground"] = "endforeground";
    shapemap["insidefaces"] = "endinsidefaces";
    shapemap["outsidefaces"] = "endoutsidefaces";
    shapemap["offsetfaces"] = "endoffsetfaces";
    shapemap["frontfaces"] = "endfrontfaces";
    shapemap["backfaces"] = "endbackfaces";
    shapemap["rimfaces"] = "endrimfaces";
    shapemap["delete"] = "enddelete";
    shapemap["subdivision"] = "endsubdivision";
    shapemap["offset"] = "endoffset";
    shapemap["include"] = "endinclude";
    while((pos = code.find(delimiter)) != std::string::npos) {
        token = code.substr(0, pos);
        std::vector<std::string> spaces;
        size_t pos2 = 0; 
        while((pos2 = token.find(delimiter2)) != std::string::npos) {
            token2 = token.substr(0, pos2);
            if(token2.find_first_not_of(' ') != std::string::npos) {
                spaces.push_back(token2);
            }
            token.erase(0, pos2 + delimiter2.length());
        }
        spaces.push_back(token);
        parsedcode.push_back(spaces);
        code.erase(0, pos + delimiter.length());
    }
    std::vector<std::string> spaces;
    size_t pos2 = 0;
    while((pos2 = code.find(delimiter2)) != std::string::npos) {
        token2 = code.substr(0, pos2);
        if(token2.find_first_not_of(' ') != std::string::npos) {
            spaces.push_back(token2);
        } 
        code.erase(0, pos2 + delimiter2.length());
    }
    spaces.push_back(code);
    parsedcode.push_back(spaces);
    std::cout << parsedcode.size() << std::endl;
    for (int i = 0; i < parsedcode.size(); i++) {
        std::vector<std::string> line = parsedcode.at(i);
        for (int j = 0; j < line.size(); j++) {
            std::string element = line.at(j); 
            std::cout << element + " " + std::to_string(i) + " " + std::to_string(j) << std::endl;
            if (element == "group" || (shapemap.find(element))!= shapemap.end()) { //check for keywords here.
                auto cast = shapemap.find(element);
                std::string endval = cast -> second;
                std::vector<std::string> result;
                if (element == "group") {
                    if (j == line.size() - 1) {
                        result = CheckGroup(parsedcode, idmap, i + 1, 0, shapemap);
                    } else {
                        result = CheckGroup(parsedcode, idmap, i, j + 1, shapemap);
                    }
                    if (result[0] == "error") {
                        return;
                    }
                    i = std::stoi(result[0]);
                    j = std::stoi(result[1]);
                } else if (element == "bank") {
                    if (j == line.size() - 1) {
                        result = CheckBank(parsedcode, referencemap, idmap, i + 1, 0, shapemap);
                    } else {
                        result = CheckBank(parsedcode,referencemap, idmap, i, j + 1, shapemap);
                    }
                    if (result[0] == "error") {
                        return;
                    }
                    i = std::stoi(result[0]);
                    j = std::stoi(result[1]);
                } else {
                    if (j == line.size() - 1) {
                        result = CheckStatement(parsedcode, idmap, endval, i + 1, 0, shapemap);
                    } else {
                        result = CheckStatement(parsedcode, idmap, endval, i, j + 1, shapemap);
                    }
                    if (result[0] == "error") {
                        return;
                    }
                    i = std::stoi(result[0]);
                    j = std::stoi(result[1]);
                    std::string elemid = result[2];
                    idmap[elemid] = "TRUE";
                }
                line = parsedcode.at(i);
                element = line.at(j); 
            } else {
               std::cout << "Error at Line " + std::to_string(i + 1) + ": " + element + " is not a valid function." << std::endl;
               return; 
            }
        }
    }
}

std::vector<std::string> CSourceManager::CheckStatement(std::vector<std::vector<std::string>> parsedcode,
                                                        std::unordered_map<std::string, std::string> idmap,
                                                        std::string endstatement,
                                                        int i, int j,
                                                        std::unordered_map<std::string, std::string> shapemap) {
    int global_k;
    int global_l;
    bool first_time = true;
    std::string id;
    for (int k = 0; k < parsedcode.size(); k++) {
        if (first_time == true) {
            k = i;
        }
        std::vector<std::string> line = parsedcode.at(k);
        for (int l = 0; l < line.size(); l++) {
            global_k = k;
            global_l = l;
            if (first_time == true) {
                l = j;
                first_time = false;
                id = line.at(l);
                if ((idmap.find(id))!= idmap.end()) {
                    std::cout << "Error at Line " + std::to_string(i + 1) + ": " + id + " is already being used." << std::endl;
                    return {"error"};
                }
            }
            std::string element = line.at(l);
            if ((shapemap.find(element))!= shapemap.end() && element != "instance") {
                std::cout << "Error at Line " + std::to_string(i + 1) + ": " + element + " is a reserved keyword." << std::endl;
                return {"error"};
            }
            if (element == endstatement) {
                std::vector<std::string> ret;
                if (l == line.size() - 1) {
                    ret = {std::to_string(k), std::to_string(l), id};
                } else {
                    ret = {std::to_string(k), std::to_string(l), id};
                }
                return ret; 
            }

        }
    }
    return {std::to_string(global_k), std::to_string(global_l), ""};
}

std::vector<std::string> CSourceManager::CheckGroup(std::vector<std::vector<std::string>> parsedcode,
                                                        std::unordered_map<std::string, std::string> &idmap,
                                                        int i, int j,
                                                        std::unordered_map<std::string, std::string> shapemap) {
    bool first_time = true;
    std::string id;
    int global_k;
    int global_l;
    for (int k = 0; k < parsedcode.size(); k++) {
        if (first_time == true) {
            k = i;
        }
        std::vector<std::string> line = parsedcode.at(k);
        for (int l = 0; l < line.size(); l++) {
            if (first_time == true) {
                l = j;
                first_time = false;
                id = line.at(l);
                if ((idmap.find(id))!= idmap.end()) {
                    std::cout << "Error at Line " + std::to_string(i + 1) + ": " + id + " is already being used." << std::endl;
                    return {"error"};
                }
                continue;
            }
            global_k = k;
            global_l = l;
            std::vector<std::string> result;
            std::string element = line.at(l);
            if (element == "endgroup") {
                std::vector<std::string> ret;
                if (l == line.size() - 1) {
                    ret = {std::to_string(k), std::to_string(l)};
                } else {
                    ret = {std::to_string(k), std::to_string(l)};
                }
                return ret; 
            }
            if (element == "instance") {
                if (l == line.size() - 1) {
                    result = CheckInstance(parsedcode, idmap, k + 1, 0, shapemap);
                } else {
                    result = CheckInstance(parsedcode, idmap, k, l + 1, shapemap);
                }
            }
            else {
                std::cout << "Error at Line " + std::to_string(k + 1) + ": Expected Instance" << std::endl;
                return {"error"};
            }
            if (result[0] == "error") {
                return {"error"};
            }
            k = std::stoi(result[0]);
            l = std::stoi(result[1]);
            std::string elemid = result[2];
            idmap[elemid] = "TRUE";
        }
    }
    std::cout << "Error at Line " + std::to_string(i + 1) + ": endgroup expected" << std::endl;
    return {std::to_string(global_k), std::to_string(global_l)};
}

std::vector<std::string> CSourceManager::CheckBank(std::vector<std::vector<std::string>> parsedcode,
                                                        std::unordered_map<std::string, std::string> &referencemap,
                                                        std::unordered_map<std::string, std::string> &idmap,
                                                        int i, int j,
                                                        std::unordered_map<std::string, std::string> shapemap) {
    bool first_time = true;
    std::string id;
    for (int k = 0; k < parsedcode.size(); k++) {
        if (first_time == true) {
            k = i;
        }
        std::vector<std::string> line = parsedcode.at(k);
        for (int l = 0; l < line.size(); l++) {
            if (first_time == true) {
                l = j;
                first_time = false;
                id = line.at(l);
                if ((idmap.find(id))!= idmap.end()) {
                    std::cout << "Error at Line " + std::to_string(i + 1) + ": " + id + " is already being used." << std::endl;
                    return {"error"};
                }
                continue;
            }
            std::vector<std::string> result;
            std::string element = line.at(l);
            if (element == "endbank") {
                std::vector<std::string> ret;
                if (l == line.size() - 1) {
                    ret = {std::to_string(k), std::to_string(l)};
                } else {
                    ret = {std::to_string(k), std::to_string(l)};
                }
                return ret; 
            }
            if (l == 0) {
                if (element == "set") {
                    if (line.size() == 6) {
                        std::string secondval = line.at(l + 1);
                        if ((shapemap.find(secondval))!= shapemap.end() && secondval != "instance") {
                            std::cout << "Error at Line " + std::to_string(i + 1) + ": " + secondval + " is a reserved keyword." << std::endl;
                            return {"error"};
                        } else if (idmap.find(secondval) != idmap.end()) {
                            std::cout << "Error at Line " + std::to_string(i + 1) + ": " + secondval + " is already being used as an id." << std::endl;
                            return {"error"};
                        } else if (referencemap.find(secondval) != referencemap.end()) {
                            std::cout << "Error at Line " + std::to_string(i + 1) + ": " + secondval + " is already being used as an reference." << std::endl;
                            return {"error"};
                        }
                        referencemap[secondval] = true; 
                        l+=5;
                    } else {
                        std::cout << "Error at Line " + std::to_string(k + 1) + ": Expected 5 Parameters in Set, Received " + std::to_string(line.size() - 1)  << std::endl;
                        return {"error"};
                    }

                } else {
                    std::cout << "Error at Line " + std::to_string(k + 1) + ": Expected Set at the Start of a New Line." << std::endl;
                    return {"error"};
                }
            }

        }
    }
    std::cout << "Error at Line " + std::to_string(i + 1) + ": endbank expected" << std::endl;
    return {"error"};
}

std::vector<std::string> CSourceManager::CheckInstance(std::vector<std::vector<std::string>> parsedcode,
                                                        std::unordered_map<std::string, std::string> idmap,
                                                        int i, int j,
                                                        std::unordered_map<std::string, std::string> shapemap) {
    bool first_time = true;
    bool second_time = false;
    std::string id;
    std::string obj;
    for (int k = 0; k < parsedcode.size(); k++) {
        if (first_time == true) {
            k = i;
        }
        std::vector<std::string> line = parsedcode.at(k);
        for (int l = 0; l < line.size(); l++) {
            if (first_time == true) {
                l = j;
                first_time = false;
                id = line.at(l);
                if ((idmap.find(id))!= idmap.end()) {
                    std::cout << "Error at Line " + std::to_string(k + 1) + ": " + id + " is already being used." << std::endl;
                }
                second_time = true;
                continue;
            }
            if (second_time == true) {
                obj = line.at(l); 
                if ((idmap.find(obj)) == idmap.end()) {
                    std::cout << "Error at Line " + std::to_string(k + 1) + ": " + obj + " is not defined." << std::endl;
                }
                second_time = false; 
                continue;
            }
            std::string element = line.at(l);
            if (element == "endinstance") {
                std::vector<std::string> ret;
                ret = {std::to_string(k), std::to_string(l), id};
                return ret; 
            } else if (element == "rotate" || element == "scale" || element == "translate") {
                int templ = l+1;
                std::string secondelem = line.at(templ);
                if (secondelem.find('(') != std::string::npos && secondelem.find(')') == std::string::npos) {
                    if (checkcount(secondelem, '(') > 1) {
                        std::cout << "Error at Line " + std::to_string(k + 1) + " Mismatched Parenthesis. Parenthesis must close at the same line." << std::endl;
                        return {"error"};
                    }
                    templ++;
                    secondelem = line.at(templ);
                    while (secondelem.find(')') == std::string::npos) {
                        if (templ == line.size() - 1) {
                            std::cout << "Error at Line " + std::to_string(k + 1) + " Mismatched Parenthesis. Parenthesis must close at the same line." << std::endl;
                            return {"error"};
                        } else if(secondelem.find('(') != std::string::npos) {
                            std::cout << "Error at Line " + std::to_string(k + 1) + " Mismatched Parenthesis. Parenthesis must close at the same line." << std::endl;
                            return {"error"};
                        }
                        templ++;
                        secondelem = line.at(templ);
                    }
                    l = templ;
                    if (checkcount(line.at(l), ')') > 1)  {
                        std::cout << "Error at Line " + std::to_string(k + 1) + " Mismatched Parenthesis. Parenthesis must close at the same line." << std::endl;
                        return {"error"};

                    }
                } else if (secondelem.find('(') != std::string::npos && secondelem.find(')') != std::string::npos) {
                    if (checkcount(secondelem, ')') > 1 || checkcount(secondelem, '(') > 1) {
                        std::cout << "Error at Line " + std::to_string(k + 1) + " Mismatched Parenthesis. Parenthesis must close at the same line." << std::endl;
                        return {"error"};
                    }
                } else {
                    std::cout << "Error at Line " + std::to_string(k + 1) + "Expected Start of Parenthesis" << std::endl;
                    return {"error"};
                }
            } else if (element == "surface" || element == "LOD" || element == "shading") {
                int templ = l+1;
                std::string secondelem = line.at(templ);
                if ((shapemap.find(secondelem))!= shapemap.end()) {
                    std::cout << "Error at Line " + std::to_string(k + 1) + ": " + element + " is a reserved keyword." << std::endl;
                }
                l = templ; 
            }
        }
    }
    std::cout << "Error at Line " + std::to_string(i + 1) + ": endinstance expected" << std::endl;
    return {};
}

int CSourceManager::checkcount(std::string str, char letter) {
    int count = 0;
    for (int i = 0; i < str.size(); i++)
    {
        if (str[i] ==  letter)
        {
            ++ count;
        }
    }
    return count;
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

// Steven's Add Point
bool CSourceManager::AppendText(const std::string& text)
{
    std::ofstream ofs(GetMainSourcePath());
    InsertText(CollectText().length() - 1, text);
    SaveFile();
    return true; // Randy added this
}

}
