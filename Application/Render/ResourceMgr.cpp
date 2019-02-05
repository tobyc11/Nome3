#include "ResourceMgr.h"
#include <PathTools.h>
#include <stdexcept>
#include <cassert>

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

	ResourcesDir = FPathTools::FindParentSubDirectoryRecursively(FPathTools::GetWorkingDirectory(), "Resources");
	if (!ResourcesDir.empty())
		return;

	ResourcesDir = FPathTools::FindParentSubDirectoryRecursively(FPathTools::GetExecutablePath(), "Resources");
	if (!ResourcesDir.empty())
		return;

	printf("%s", "Cannot locate the Resources directory.\n");
	throw std::runtime_error("Fatal error occured in resource manager");
}

CResourceMgr::~CResourceMgr()
{
}

std::string CResourceMgr::Find(const std::string& filename)
{
	auto fullPath = FPathTools::Join(ResourcesDir, filename);
	assert(FPathTools::Exists(fullPath));
	return fullPath;
}

}