#include "SourceManager.h"
#include "../QtFrontend/ResourceMgr.h"
#include "NomLexer.h"
#include "NomParser.h"
#include "SyntaxTreeBuilder.h"
#include "antlr4-runtime.h"
#include <fstream>
#include <stack>
#include <unordered_map>
#include <utility>
#include <ctype.h>
#include <regex>


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
        // std::cout << "line " << line << ":" << charPositionInLine << " " << msg << std::endl;
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
    // std::cout << *ASTRoot;
    std::cout << "====== End Debug Print AST ======" << std::endl;

    return !errorListener.bDidErrorHappen;
}
bool CSourceManager::balancedbracket(std::string codeline) {   
    std::stack<char> stk; 
    char x; 
    for (int i = 0; i < codeline.length(); i++) { 
        if (codeline[i] == '(' || codeline[i] == '[' || codeline[i] == '{') { 
            stk.push(codeline[i]); 
            continue; 
        } 
        switch (codeline[i]) { 
        case ')': 
            if (stk.empty()) {
                return false;
            }
            x = stk.top(); 
            stk.pop(); 
            if (x == '{' || x == '[') 
                return false; 
            break; 
        case '}': 
            if (stk.empty()) {
                return false;
            }
            x = stk.top(); 
            stk.pop(); 
            if (x == '(' || x == '[') 
                return false; 
            break; 
        case ']': 
            if (stk.empty()) {
                return false;
            }
            x = stk.top(); 
            stk.pop(); 
            if (x == '(' || x == '{') 
                return false; 
            break; 
        } 
    } 
    return (stk.empty()); 
}

std::vector<std::string> removeDupWord(std::string str) {
    std::string word = "";
    std::vector<std::string> parsedcode;
    for (auto x : str) {
        if (x == ' ') {
            parsedcode.push_back(word);
            word = "";
        }
        else {
            word = word + x;
        }
    }
    parsedcode.push_back(word);
    return parsedcode;
}

std::vector<std::string> CSourceManager::ParameterCheck(std::vector<std::string> code, std::string type, int numparams, std::unordered_map<std::string, std::string> idmap) {
    int start = 0; 
    int end = 0;
    std::string concatstr = "";
    for (int i = 0; i < code.size(); i++) {
        concatstr += RemoveSpecials(code[i]);
        concatstr += " ";
    }
    for (int i = 0; i < concatstr.length(); i++) {
        if (concatstr[i] == '#') {
            break;
        }
        if (concatstr[i] == '(') {
            start = i;
        }
        if (concatstr[i] == ')') {
            end = i; 
        }
    }
    if (start == 0 || end == 0) {
        return {"0", "false"};
    }
    std::string parenthesiscode = concatstr.substr(start + 1, end-start - 1);
    std::vector<std::string> words{};
    words = removeDupWord(parenthesiscode);
    
    if (numparams != -1 && words.size() != numparams) {
        return {"0", "false"};
    }
    // for(auto elem : idmap)
    // {
    //     std::cout << elem.first << "-" <<elem.second << "\n";
    // }
    // for (int i = 0; i < words.size(); i++) {
    //     std::cout << "Each word: " << words[i] << std::endl; 

    // }
    if (type == "circle" || type == "point") {
        for (int i = 0; i < words.size(); i++) {
        }
    } else if (type == "face") {
        // for (int i = 0; i < words.size(); i++) {
        //     std::cout << "The Word: " << words[i] << std::endl; 
        //     std::cout << "THE ID: " << idmap[words[i]] << std::endl; 

        // }
        for (int i = 0; i < words.size(); i++) {
            if (idmap[words[i]] != "Point") {
                return {std::to_string(i), "false"}; 
            }
        }
    } else if (type == "polyline") {
        for (int i = 0; i < words.size(); i++) {
            if (idmap[words[i]] != "Point") {
                return {std::to_string(i), "false"}; 
            }
        } 
    } else if (type == "surface") {
        if (words.size() > 3) {
            return {"4", "false"}; 
        } else if (words.size() < 3) {
            return {std::to_string(words.size()), "false"}; 
        } else {
            for (int i = 0; i < words.size(); i++) {
                if (isNumber(words[i])) {
                    int num = std::stoi(words[i]);
                    if (num < 0 || num > 1) {
                        return {std::to_string(i), "false"};
                    }

                } else {
                    return {std::to_string(i), "false"}; 
                }
            } 
        }
    }
    return {"0", "true"}; 
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
    std::string thepath = CResourceMgr::Get().Find("DebugDrawLine.xml");
    std::string basepath = thepath.substr(0, 43);
    std::ifstream file (basepath + "Parsing/Nom.g4");
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            std::vector<std::string> spaces;
            std::string str = line.c_str();
            size_t pos = 0; 
            while ((pos = str.find(delimiter2)) != std::string::npos) {
                token = str.substr(0, pos);
                str.erase(0, pos + delimiter.length());
                spaces.push_back(token);
            }
            for (int i = 0; i < spaces.size(); i++) {
                std::string keyword;
                std::string endword;
                if (spaces[i].find("open=") != std::string::npos) {
                    int start = 0; int end = 0;
                    for (int k = 0; k < spaces[i].size(); k++) {
                        if (start == 0 && spaces[i][k] == '\'') {
                            start = k;
                        } else if (start > 0 && spaces[i][k] == '\'') {
                            end = k;
                        }
                    }
                    end--;
                    keyword = spaces[i].substr(start + 1, end - start);
                    if (keyword == "set") {
                        continue;
                    }
                    for (int j = i; j < spaces.size(); j++) {
                        if (spaces[j].find("end=") != std::string::npos) {
                            int start = 0; int end = 0;
                            for (int k = 0; k < spaces[j].size(); k++) {
                                if (start == 0 && spaces[j][k] == '\'') {
                                    start = k;
                                } else if (start > 0 && spaces[j][k] == '\'') {
                                    end = k;
                                }
                            }
                            endword = spaces[j].substr(start + 1, end - start - 1);
                            break;
                            
                        }
                        i = j;
                    }
                    shapemap[keyword] = endword;
                }
            }
        }
        file.close();
    }
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
    for (int i = 0; i < parsedcode.size(); i++) {
        std::vector<std::string> line = parsedcode.at(i);
        for (int j = 0; j < line.size(); j++) {
            std::string element = RemoveSpecials(line.at(j)); 
            if (element.find("#") != std::string::npos && element.at(0) == '#') { //Comment Detection
                j = line.size();
                continue; 
            } 
            if (element.empty()) {
                j = line.size();
                continue; 
            }
            if (element == "group" || (shapemap.find(element))!= shapemap.end()) { //check for keywords here.
                auto cast = shapemap.find(element);
                std::string endval = cast -> second;
                std::vector<std::string> result;
                if (element == "circle") {
                    if (j == line.size() - 1) {
                        result = CheckCircle(parsedcode, idmap, i + 1, 0, shapemap);
                    } else {
                        result = CheckCircle(parsedcode, idmap, i, j + 1, shapemap);
                    }
                    if (result[0] == "error") {
                        return;
                    }
                    i = std::stoi(result[0]);
                    j = std::stoi(result[1]);
                } else if (element == "surface") {
                    if (j == line.size() - 1) {
                        result = CheckSurface(parsedcode, idmap, i + 1, 0, shapemap);
                    } else {
                        result = CheckSurface(parsedcode, idmap, i, j + 1, shapemap);
                    }
                    if (result[0] == "error") {
                        return;
                    }
                    i = std::stoi(result[0]);
                    j = std::stoi(result[1]);

                } else if (element == "polyline") {
                    if (j == line.size() - 1) {
                        result = CheckPolyline(parsedcode, idmap, i + 1, 0, shapemap);
                    } else {
                        result = CheckPolyline(parsedcode, idmap, i, j + 1, shapemap);
                    }
                    if (result[0] == "error") {
                        return;
                    }
                    i = std::stoi(result[0]);
                    j = std::stoi(result[1]);

                } else if (element == "point") {
                    if (j == line.size() - 1) {
                        result = CheckPoint(parsedcode, idmap, i + 1, 0, shapemap);
                    } else {
                        result = CheckPoint(parsedcode, idmap, i, j + 1, shapemap);
                    }
                    if (result[0] == "error") {
                        return;
                    }
                    i = std::stoi(result[0]);
                    j = std::stoi(result[1]);
                } else if (element == "group") {
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
                } else if (element == "subdivision") {
                    if (j == line.size() - 1) {
                        result = CheckSubdivision(parsedcode, idmap, i + 1, 0, shapemap);
                    } else {
                        result = CheckSubdivision(parsedcode, idmap, i, j + 1, shapemap);
                    }
                    if (result[0] == "error") {
                        return;
                    }
                    i = std::stoi(result[0]);
                    j = std::stoi(result[1]);
                } else if (element == "mesh") {
                    if (j == line.size() - 1) {
                        result = CheckMesh(parsedcode, idmap, i + 1, 0, shapemap);
                    } else {
                        result = CheckMesh(parsedcode, idmap, i, j + 1, shapemap);
                    }
                    if (result[0] == "error") {
                        return;
                    }
                    i = std::stoi(result[0]);
                    j = std::stoi(result[1]);
                } else if (element == "face") {
                    if (j == line.size() - 1) {
                        result = CheckFace(parsedcode, idmap, i + 1, 0, shapemap);
                    } else {
                        result = CheckFace(parsedcode, idmap, i, j + 1, shapemap);
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

std::string CSourceManager::RemoveSpecials(std::string str)
{
	int i=0,len=str.length();
	while (i < len)
	{
		char c = str[i];
		if (c >= 33) {
			++i;
		} else {
			str.erase(i,1);
			--len;
		}
	}
	return str;
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
        std::string linestr = "";
        for (int l = 0; l < line.size(); l++) {
            linestr += line.at(l);
        }
        if (!balancedbracket(linestr)) {
            std::cout << "Error at Line " + std::to_string(i + 1) + ": Mismatched Parehthesis" << std::endl;
            return {"error"};
        }
        int l = 0; 
        while (l < line.size()) {
            global_k = k;
            global_l = l;
            if (first_time == true) {
                l = j;
                first_time = false;
                id = RemoveSpecials(line.at(l));
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
            std::string flushedelem = RemoveSpecials(element);
            if (flushedelem == endstatement) {
                std::vector<std::string> ret;
                if (l == line.size() - 1) {
                    ret = {std::to_string(k), std::to_string(l), id};
                } else {
                    ret = {std::to_string(k), std::to_string(l), id};
                }
                return ret; 
            }
            l++; 
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
                id = RemoveSpecials(line.at(l));
                if ((idmap.find(id))!= idmap.end()) {
                    std::cout << "Error at Line " + std::to_string(i + 1) + ": " + id + " is already being used." << std::endl;
                    return {"error"};
                } else {
                    idmap[id] = "TRUE"; 
                }
                continue;
            }
            global_k = k;
            global_l = l;
            std::vector<std::string> result;
            std::string element = RemoveSpecials(line.at(l));

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

                if (result[0] == "error") {
                    return {"error"};
                }
                k = std::stoi(result[0]);
                l = std::stoi(result[1]);
                std::string elemid = RemoveSpecials(result[2]);
                line = parsedcode.at(k);
                //idmap[elemid] = "TRUE";
            }
            else {
                std::cout << "Error at Line " + std::to_string(k) + ": Expected Instance" << std::endl;
                return {"error"};
            }

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
                id = RemoveSpecials(line.at(l));
                if ((idmap.find(id))!= idmap.end()) {
                    std::cout << "Error at Line " + std::to_string(i + 1) + ": " + id + " is already being used." << std::endl;
                    return {"error"};
                }
                continue;
            }
            std::vector<std::string> result;
            std::string element = RemoveSpecials(line.at(l));
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
                    if (line.size() == 6 || line.size() == 7) {
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
                        std::cout << "Error at Line " + std::to_string(k + 1) + ": Expected 5 or (optional) 6 Parameters in Set, Received " + std::to_string(line.size() - 1)  << std::endl;
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
                id = RemoveSpecials(line.at(l));
                if ((idmap.find(id))!= idmap.end()) {
                    std::cout << "Error at Line " + std::to_string(k + 1) + ": " + id + " is already being used." << std::endl;
                }
                idmap[id] = "TRUE";
                second_time = true;
                continue;
            }
            if (second_time == true) {
                obj = line.at(l); 
                if (idmap.find(RemoveSpecials(obj)) == idmap.end()) {
                    // for(auto elem : idmap) {
                    //     std::cout << elem.first << "\n";
                    // }
                    std::cout << "Error at Line " + std::to_string(k + 1) + ": " + obj + " is not defined." << std::endl;
                }
                second_time = false; 
                continue;
            }
            std::string element = RemoveSpecials(line.at(l));
            if (element == "endinstance") {
                std::vector<std::string> ret;
                ret = {std::to_string(k), std::to_string(l), id};
                return ret; 
            } else if (element == "rotate" || element == "scale" || element == "translate") {
                int templ = l+1;
                std::string secondelem = line.at(templ);
                if (secondelem.find('(') == std::string::npos) {
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
    return {"error"};
}

std::vector<std::string> CSourceManager::CheckSubdivision(std::vector<std::vector<std::string>> parsedcode,
                                                        std::unordered_map<std::string, std::string> &idmap,
                                                        int i, int j,
                                                        std::unordered_map<std::string, std::string> shapemap) {
    bool first_time = true;
    std::string id;
    int global_k;
    int global_l;
    int cnt = 1;
    for (int k = 0; k < parsedcode.size(); k++) {
        if (first_time == true) {
            k = i;
        }
        std::vector<std::string> line = parsedcode.at(k);
        for (int l = 0; l < line.size(); l++) {
            if (first_time == true) {
                l = j;
                first_time = false;
                id = RemoveSpecials(line.at(l));
                if ((idmap.find(id))!= idmap.end()) {
                    std::cout << "Error at Line " + std::to_string(i + 1) + ": " + id + " is already being used." << std::endl;
                    return {"error"};
                }
                continue;
            }
            if (cnt == 2 && line.at(l) == "NOME_OFFSET_DEFAULT" || line.at(l) == "NOME_OFFSET_GRID") {
                continue;
            }
            if (cnt == 3 ) {
                if (isNumber(line.at(l))) {
                    continue;
                }
            }
            global_k = k;
            global_l = l;
            std::vector<std::string> result;
            std::string element = RemoveSpecials(line.at(l));
            if (element == "endsubdivision") {
                std::vector<std::string> ret;
                if (l == line.size() - 1) {
                    ret = {std::to_string(k), std::to_string(l)};
                } else {
                    ret = {std::to_string(k), std::to_string(l)};
                }
                return ret; 
            } else if (element == "instance") {
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
            cnt++;
        }
    }
    std::cout << "Error at Line " + std::to_string(i + 1) + ": endsubdivision expected" << std::endl;
    return {std::to_string(global_k), std::to_string(global_l)};
}


std::vector<std::string> CSourceManager::CheckMesh(std::vector<std::vector<std::string>> parsedcode,
                                                        std::unordered_map<std::string, std::string> &idmap,
                                                        int i, int j,
                                                        std::unordered_map<std::string, std::string> shapemap) {
    bool first_time = true;
    std::string id;
    int global_k;
    int global_l;
    int cnt = 1;
    for (int k = 0; k < parsedcode.size(); k++) {
        if (first_time == true) {
            k = i;
        }
        std::vector<std::string> line = parsedcode.at(k);
        for (int l = 0; l < line.size(); l++) {
            if (first_time == true) {
                l = j;
                first_time = false;
                id = RemoveSpecials(line.at(l));
                if ((idmap.find(id))!= idmap.end()) {
                    std::cout << "Error at Line " + std::to_string(i + 1) + ": " + id + " is already being used." << std::endl;
                    return {"error"};
                }
                idmap[id] = "TRUE";
                continue;
            }
            global_k = k;
            global_l = l;
            std::vector<std::string> result;
            std::string element = RemoveSpecials(line.at(l));
            if (element == "endmesh") {
                std::vector<std::string> ret;
                if (l == line.size() - 1) {
                    ret = {std::to_string(k), std::to_string(l)};
                } else {
                    ret = {std::to_string(k), std::to_string(l)};
                }
                return ret; 
            } else if (element == "point") {
                if (l == line.size() - 1) {
                    result = CheckStatement(parsedcode, idmap, "endpoint", k + 1, 0, shapemap);
                } else {
                    result = CheckStatement(parsedcode, idmap, "endpoint", k, l + 1, shapemap);
                }
                if (result[0] == "error") {
                    return {"error"};
                }
            } else if (element == "face") { 
                if (l == line.size() - 1) {
                    result = CheckFace(parsedcode, idmap, k + 1, 0, shapemap);
                } else {
                    result = CheckFace(parsedcode, idmap, k, l + 1, shapemap);
                }
                if (result[0] == "error") {
                    return {"error"};
                }
            }
            else {
                std::cout << "Error at Line " + std::to_string(k + 1) + ": Expected Point or Face" << std::endl;
                return {"error"};
            }
            k = std::stoi(result[0]);
            l = std::stoi(result[1]);
            std::string elemid = result[2];
            idmap[elemid] = "TRUE";
            cnt++;
        }
    }
    std::cout << "Error at Line " + std::to_string(i + 1) + ": endmesh expected" << std::endl;
    return {std::to_string(global_k), std::to_string(global_l)};
}

std::vector<std::string> CSourceManager::CheckCircle(std::vector<std::vector<std::string>> parsedcode,
                                                        std::unordered_map<std::string, std::string> &idmap,
                                                        int i, int j,
                                                        std::unordered_map<std::string, std::string> shapemap) {
    bool first_time = true;
    std::string id;
    int global_k;
    int global_l;
    int cnt = 1;
    for (int k = 0; k < parsedcode.size(); k++) {
        if (first_time == true) {
            k = i;
        }
        std::vector<std::string> line = parsedcode.at(k);
        for (int l = 0; l < line.size(); l++) {
            if (first_time == true) {
                l = j;
                first_time = false;
                id = RemoveSpecials(line.at(l));
                if ((idmap.find(id))!= idmap.end()) {
                    std::cout << "Error at Line " + std::to_string(i + 1) + ": " + id + " is already being used." << std::endl;
                    return {"error"};
                }
                idmap[id] = "TRUE";
                cnt++; 
                continue;
            }
            global_k = k;
            global_l = l;
            std::vector<std::string> result;
            std::string element = RemoveSpecials(line.at(l));
            if (cnt > 4) {
                std::cout << "Error at Line " + std::to_string(k + 1) + ": endcircle expected." << std::endl;
                return {"error"};
            }
            if (cnt == 2 && element.find('(') != std::string::npos) {
                std::vector<std::string> res = ParameterCheck(line, "circle", 2, idmap);
                std::string truthval = res[1];
                int position = std::stoi(res[0]);
                if (truthval != "true") {
                    std::cout << "Error at Line " + std::to_string(i + 1) + ": Invalid Parameters for type circle." << std::endl;
                    return {"error"};
                }
                continue; 

            } else if (element == "endcircle") {
                std::vector<std::string> ret;
                if (l == line.size() - 1) {
                    ret = {std::to_string(k), std::to_string(l)};
                } else {
                    ret = {std::to_string(k), std::to_string(l)};
                }
                return ret;
            } else {
                continue;
            }
            k = std::stoi(result[0]);
            l = std::stoi(result[1]);
            std::string elemid = result[2];
            idmap[elemid] = "TRUE";
            cnt++;
        }
    }
    std::cout << "Error at Line " + std::to_string(i + 1) + ": endcircle expected" << std::endl;
    return {std::to_string(global_k), std::to_string(global_l)};
}


std::vector<std::string> CSourceManager::CheckPolyline(std::vector<std::vector<std::string>> parsedcode,
                                                        std::unordered_map<std::string, std::string> &idmap,
                                                        int i, int j,
                                                        std::unordered_map<std::string, std::string> shapemap) {
    bool first_time = true;
    std::string id;
    int global_k;
    int global_l;
    int cnt = 1;
    for (int k = 0; k < parsedcode.size(); k++) {
        if (first_time == true) {
            k = i;
        }
        std::vector<std::string> line = parsedcode.at(k);
        for (int l = 0; l < line.size(); l++) {
            if (first_time == true) {
                l = j;
                first_time = false;
                id = RemoveSpecials(line.at(l));
                if ((idmap.find(id))!= idmap.end()) {
                    for (int f = 0; f < line.size(); f++) {
                        std::cout << RemoveSpecials(line.at(f)) << ' ';
                        if (f == l - 1) {
                            std::cout << ">>";
                        }
                    }
                    std::cout << "\n";
                    std::cout << "Error at Line " + std::to_string(i + 1) + " at Position " + std::to_string(l) + ": " + id + " is already being used." << std::endl;
                    return {"error"};
                }
                idmap[id] = "Polyline";
                cnt++; 
                continue;
            }
            global_k = k;
            global_l = l;
            std::vector<std::string> result;
            std::string element = RemoveSpecials(line.at(l));
            if (l == 2 && element.find('(') != std::string::npos) {
                std::vector<std::string> line = parsedcode.at(k);
                std::string linestr = "";
                for (int l = 0; l < line.size(); l++) {
                    linestr += line.at(l);
                }
                if (!balancedbracket(linestr)) {
                    for (int f = 0; f < line.size(); f++) {
                        std::cout << RemoveSpecials(line.at(f)) << ' ';
                        if (f == l - 1) {
                            std::cout << ">>";
                        }
                    }
                    std::cout << "\n";
                    std::cout << "Error at Line " + std::to_string(i + 1) + " at Position " + std::to_string(l) + ": Mismatched Parenthesis" << std::endl;
                    return {"error"};
                }
                std::vector<std::string> res = ParameterCheck(line, "polyline", -1, idmap);
                std::string truthval = res[1];
                int position = std::stoi(res[0]);
                if (truthval != "true") {
                    int splitpoint = l + position;
                    for (int f = 0; f < line.size(); f++) {
                        std::cout << RemoveSpecials(line.at(f)) << ' ';
                        if (f == splitpoint - 1) {
                            std::cout << ">>";
                        }
                    }
                    std::cout << "\n";
                    std::cout << "Error at Line " + std::to_string(i + 1) + " at Position " + std::to_string(splitpoint) + ": Invalid Parameters for type Polyline." << std::endl;


                    return {"error"};
                }
                continue; 

            } else if (l == 3 && element == "closed") {
                continue;
            } else if (element == "endpolyline") {
                std::vector<std::string> ret;
                if (l == line.size() - 1) {
                    ret = {std::to_string(k), std::to_string(l)};
                } else {
                    ret = {std::to_string(k), std::to_string(l)};
                }
                return ret;
            } else {
                continue;
            }
        }
    }
    std::cout << "Error at Line " + std::to_string(i + 1) + ": endpolyline expected" << std::endl;
    return {"error"};
}


std::vector<std::string> CSourceManager::CheckSurface(std::vector<std::vector<std::string>> parsedcode,
                                                        std::unordered_map<std::string, std::string> &idmap,
                                                        int i, int j,
                                                        std::unordered_map<std::string, std::string> shapemap) {
    bool first_time = true;
    std::string id;
    int global_k;
    int global_l;
    int cnt = 1;
    for (int k = 0; k < parsedcode.size(); k++) {
        if (first_time == true) {
            k = i;
        }
        std::vector<std::string> line = parsedcode.at(k);
        for (int l = 0; l < line.size(); l++) {
            if (first_time == true) {
                l = j;
                first_time = false;
                id = RemoveSpecials(line.at(l));
                if ((idmap.find(id))!= idmap.end()) {
                    for (int f = 0; f < line.size(); f++) {
                        std::cout << RemoveSpecials(line.at(f)) << ' ';
                        if (f == l - 1) {
                            std::cout << ">>";
                        }
                    }
                    std::cout << "\n";
                    std::cout << "Error at Line " + std::to_string(i + 1) + " at Position " + std::to_string(l) + ": " + id + " is already being used." << std::endl;
                    return {"error"};
                }
                idmap[id] = "Surface";
                cnt++; 
                continue;
            }
            global_k = k;
            global_l = l;
            std::vector<std::string> result;
            std::string element = RemoveSpecials(line.at(l));
            if (l == 2 && element == "color") {
                continue;
            } else if (l == 3 && element.find('(') != std::string::npos) {
                std::vector<std::string> line = parsedcode.at(k);
                std::string linestr = "";
                for (int l = 0; l < line.size(); l++) {
                    linestr += line.at(l);
                }
                if (!balancedbracket(linestr)) {
                    for (int f = 0; f < line.size(); f++) {
                        std::cout << RemoveSpecials(line.at(f)) << ' ';
                        if (f == l - 1) {
                            std::cout << ">>";
                        }
                    }
                    std::cout << "\n";
                    std::cout << "Error at Line " + std::to_string(i + 1) + " at Position " + std::to_string(l) + ": Mismatched Parenthesis" << std::endl;
                    return {"error"};
                }
                std::vector<std::string> res = ParameterCheck(line, "surface", 3, idmap);
                std::string truthval = res[1];
                int position = std::stoi(res[0]);
                if (truthval != "true") {
                    int splitpoint = l + position;
                    for (int f = 0; f < line.size(); f++) {
                        std::cout << RemoveSpecials(line.at(f)) << ' ';
                        if (f == splitpoint - 1) {
                            std::cout << ">>";
                        }
                    }
                    std::cout << "\n";
                    std::cout << "Error at Line " + std::to_string(i + 1) + " at Position " + std::to_string(splitpoint) + ": Invalid Parameters for type Surface." << std::endl;
                    return {"error"};
                }
                continue; 

            } else if (element == "endsurface") {
                std::vector<std::string> ret;
                if (l == line.size() - 1) {
                    ret = {std::to_string(k), std::to_string(l)};
                } else {
                    ret = {std::to_string(k), std::to_string(l)};
                }
                return ret;
            } else {
                    // int splitpoint = l;
                    // for (int f = 0; f < line.size(); f++) {
                    //     std::cout << RemoveSpecials(line.at(f)) << ' ';
                    //     if (f == splitpoint - 1) {
                    //         std::cout << ">>";
                    //     }
                    // }
                    // std::cout << "\n";
                    // std::cout << "Error at Line " + std::to_string(i + 1) + " at Position " + std::to_string(splitpoint) + ": Invalid Phrase for type Surface." << std::endl;
                    // return {"error"};
                    continue; 
            }
            cnt++;
        }
    }
    std::cout << "Error at Line " + std::to_string(i + 1) + ": endsurface expected" << std::endl;
    return {"error"};
}
std::vector<std::string> CSourceManager::CheckPoint(std::vector<std::vector<std::string>> parsedcode,
                                                        std::unordered_map<std::string, std::string> &idmap,
                                                        int i, int j,
                                                        std::unordered_map<std::string, std::string> shapemap) {
    bool first_time = true;
    std::string id;
    int global_k;
    int global_l;
    int cnt = 1;
    for (int k = 0; k < parsedcode.size(); k++) {
        if (first_time == true) {
            k = i;
        }
        std::vector<std::string> line = parsedcode.at(k);
        for (int l = 0; l < line.size(); l++) {
            if (first_time == true) {
                l = j;
                first_time = false;
                id = RemoveSpecials(line.at(l));
                if ((idmap.find(id))!= idmap.end()) {
                    std::cout << "Error at Line " + std::to_string(i + 1) + ": " + id + " is already being used." << std::endl;
                    return {"error"};
                }
                idmap[id] = "Point";
                cnt++; 
                continue;
            }
            global_k = k;
            global_l = l;
            std::vector<std::string> result;
            std::string element = RemoveSpecials(line.at(l));
            if (cnt > 5) {
                std::cout << "Error at Line " + std::to_string(i + 1) + ": endpoint expected" << std::endl;
                return {"error"};
            }
            if (cnt == 2 && element.find('(') != std::string::npos) {
                std::vector<std::string> res = ParameterCheck(line, "point", 3, idmap);
                std::string truthval = res[1];
                int position = std::stoi(res[0]);
                if (truthval != "true") {
                    std::cout << "Error at Line " + std::to_string(i + 1) + ": Invalid Parameters for type point." << std::endl;
                    return {"error"};
                }
                continue; 

            } else if (element == "endpoint") {
                std::vector<std::string> ret;
                if (l == line.size() - 1) {
                    ret = {std::to_string(k), std::to_string(l)};
                } else {
                    ret = {std::to_string(k), std::to_string(l)};
                }
                return ret;
            } else {
                continue;
            }
            k = std::stoi(result[0]);
            l = std::stoi(result[1]);
            std::string elemid = result[2];
            idmap[elemid] = "TRUE";
            cnt++;
        }
    }
    std::cout << "Error at Line " + std::to_string(i + 1) + ": endpoint expected" << std::endl;
    return {std::to_string(global_k), std::to_string(global_l)};
}

std::vector<std::string> CSourceManager::CheckFace(std::vector<std::vector<std::string>> parsedcode,
                                                        std::unordered_map<std::string, std::string> &idmap,
                                                        int i, int j,
                                                        std::unordered_map<std::string, std::string> shapemap) {
    bool first_time = true;
    std::string id;
    int global_k;
    int global_l;
    int cnt = 1;
    for (int k = 0; k < parsedcode.size(); k++) {
        if (first_time == true) {
            k = i;
        }
        std::vector<std::string> line = parsedcode.at(k);
        for (int l = 0; l < line.size(); l++) {
            if (first_time == true) {
                l = j;
                first_time = false;
                id = RemoveSpecials(line.at(l));
                if ((idmap.find(id))!= idmap.end()) {
                    for (int f = 0; f < line.size(); f++) {
                        std::cout << RemoveSpecials(line.at(f)) << ' ';
                        if (f == l - 1) {
                            std::cout << ">>";
                        }
                    }
                    std::cout << "\n";
                    std::cout << "Error at Line " + std::to_string(i + 1) + " at Position " + std::to_string(l) + ": " + id + " is already being used." << std::endl;
                    return {"error"};
                }
                idmap[id] = "Face";
                cnt++; 
                continue;
            }
            global_k = k;
            global_l = l;
            std::vector<std::string> result;
            std::string element = RemoveSpecials(line.at(l));
            if (cnt == 2 && element.find('(') != std::string::npos) {
                std::vector<std::string> line = parsedcode.at(k);
                std::string linestr = "";
                for (int l = 0; l < line.size(); l++) {
                    linestr += line.at(l);
                }
                if (!balancedbracket(linestr)) {
                    for (int f = 0; f < line.size(); f++) {
                        std::cout << RemoveSpecials(line.at(f)) << ' ';
                        if (f == l - 1) {
                            std::cout << ">>";
                        }
                    }
                    std::cout << "\n";
                    std::cout << "Error at Line " + std::to_string(i + 1) + " at Position " + std::to_string(l) + ": Mismatched Parenthesis" << std::endl;
                    return {"error"};
                }
                std::vector<std::string> res = ParameterCheck(line, "face", -1, idmap);
                std::string truthval = res[1];
                int position = std::stoi(res[0]);
                if (truthval != "true") {
                    int splitpoint = l + position;
                    for (int f = 0; f < line.size(); f++) {
                        std::cout << RemoveSpecials(line.at(f)) << ' ';
                        if (f == splitpoint - 1) {
                            std::cout << ">>";
                        }
                    }
                    std::cout << "\n";
                    std::cout << "Error at Line " + std::to_string(i + 1) + " at Position " + std::to_string(splitpoint) + ": Invalid Parameters for type Face." << std::endl;


                    return {"error"};
                }
                continue; 

            } else if (element == "endface") {
                std::vector<std::string> ret;
                if (l == line.size() - 1) {
                    ret = {std::to_string(k), std::to_string(l)};
                } else {
                    ret = {std::to_string(k), std::to_string(l)};
                }
                return ret;
            } else {
                continue;
            }
        }
    }
    std::cout << "Error at Line " + std::to_string(i + 1) + ": endface expected" << std::endl;
    return {"error"};
}

bool CSourceManager::isNumber(std::string s) {
    for (int i = 0; i < s.length(); i++)
        if (s[i] == '.') {
            continue;
        } else if (isdigit(s[i]) == false) {
            return false;
        }
 
    return true;
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