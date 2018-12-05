#pragma once

#include <string>

namespace Nome
{

class CResourceManager
{
public:
    static std::string LocateFile(const std::string& name)
    {
        return "Resources\\" + name;
    }
};

} // namespace Nome
