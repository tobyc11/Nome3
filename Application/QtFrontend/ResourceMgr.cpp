#include "ResourceMgr.h"
#include <PathTools.h>
#include <cassert>
#include <stdexcept>

namespace Nome
{

using tc::FPathTools;

CResourceMgr& CResourceMgr::Get()
{
    static CResourceMgr mgr;
    return mgr;
}

CResourceMgr::CResourceMgr()
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

CResourceMgr::~CResourceMgr() = default;

std::string CResourceMgr::Find(const std::string& filename)
{
    auto fullPath = FPathTools::Join(ResourcesDir, filename);
    assert(FPathTools::Exists(fullPath));
    return fullPath;
}

}
