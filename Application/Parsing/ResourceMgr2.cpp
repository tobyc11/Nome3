#include "ResourceMgr2.h"
#include <PathTools.h>
#include <cassert>
#include <stdexcept>

namespace Nome
{

using tc::FPathTools;

CResourceMgr2& CResourceMgr2::Get()
{
    static CResourceMgr2 mgr;
    return mgr;
}

CResourceMgr2::CResourceMgr2()
{
    ResourcesDir = FPathTools::Join(FPathTools::GetWorkingDirectory(), "Resources");
    if (FPathTools::Exists(ResourcesDir))
        return;

    ResourcesDir = FPathTools::FindParentSubDirectoryRecursively(FPathTools::GetWorkingDirectory(),
                                                                 "Application/Resources");
    if (!ResourcesDir.empty())
        return;

    ResourcesDir = FPathTools::FindParentSubDirectoryRecursively(FPathTools::GetExecutablePath(),
                                                                 "Application/Resources");
    if (!ResourcesDir.empty())
        return;

    ResourcesDir = FPathTools::FindParentSubDirectoryRecursively(FPathTools::GetWorkingDirectory(),
                                                                 "Nome3/Application/Resources");
    if (!ResourcesDir.empty())
        return;

    ResourcesDir = FPathTools::FindParentSubDirectoryRecursively(FPathTools::GetExecutablePath(),
                                                                 "Nome3/Application/Resources");
    if (!ResourcesDir.empty())
        return;

    ResourcesDir = FPathTools::FindParentSubDirectoryRecursively(FPathTools::GetWorkingDirectory(),
                                                                 "Resources");
    if (!ResourcesDir.empty())
        return;

    ResourcesDir =
        FPathTools::FindParentSubDirectoryRecursively(FPathTools::GetExecutablePath(), "Resources");
    if (!ResourcesDir.empty())
        return;

    printf("%s", "Cannot locate the Resources directory.\n");
    throw std::runtime_error("Fatal error occured in resource manager");
}

CResourceMgr2::~CResourceMgr2() = default;

std::string CResourceMgr2::Find(const std::string& filename)
{
    auto fullPath = FPathTools::Join(ResourcesDir, filename);
    assert(FPathTools::Exists(fullPath));
    return fullPath;
}

}
