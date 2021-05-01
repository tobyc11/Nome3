#pragma once
#include <string>

namespace Nome
{

class CResourceMgr2
{
public:
    static CResourceMgr2& Get();

    CResourceMgr2();
    ~CResourceMgr2();

    std::string Find(const std::string& filename);

private:
    std::string ResourcesDir;
};

}
