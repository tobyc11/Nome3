#pragma once
#include <string>

namespace Nome
{

class CResourceMgr
{
public:
    static CResourceMgr& Get();

    CResourceMgr();
    ~CResourceMgr();

    std::string Find(const std::string& filename);

private:
    std::string ResourcesDir;
};

}
