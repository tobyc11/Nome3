#pragma once
#include "FoundationAPI.h"
#include <string>
#include <vector>
namespace tc
{
static const int CONVERSION_BUFFER_LENGTH = 128;
static const int MATRIX_CONVERSION_BUFFER_LENGTH = 256;
class FOUNDATION_API FStringUtils
{
public:
    static bool EndsWith(const std::string& str, const std::string& ending)
    {
        if(ending.length() > str.length())
            return false;
        for(size_t i = 0; i < ending.length(); i++)
            if(str[str.length() - ending.length() + i] != ending[i])
                return false;
        return true;
    }
    
    static std::vector<std::string> Split(const std::string& str, const std::string& delim)
    {
        std::vector<std::string> result;
        size_t pos = 0, dpos;
        while((dpos = str.find(delim, pos)) != std::string::npos)
        {
            result.push_back(str.substr(pos, dpos - pos));
            pos = dpos + delim.length();
        }
        result.push_back(str.substr(pos));
        return result;
    }
    
    template <class IT>
    static std::string Combine(IT ibegin, IT iend, const std::string& delim)
    {
        std::string result;
        result += *ibegin;
        for(ibegin++; ibegin != iend; ++ibegin)
        {
            result += delim;
            result += *ibegin;
        }
        return result;
    }

	//========= Copyright Valve Corporation ============//
	static std::string UTF16to8(const wchar_t* in);
	static std::wstring UTF8to16(const char* in);
};
}
