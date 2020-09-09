#include "ModuleService.h"
#include "Platform.h"
#include <algorithm>

namespace tc
{

// Forward declarations
IModule* OSLoadModule(const std::string& name);
void OSFreeModule(IModule* module);

FModuleService& FModuleService::Get()
{
    static FModuleService* singleton = nullptr;
    if (!singleton)
        singleton = new FModuleService();
    return *singleton;
}

IModule* FModuleService::LoadModule(const std::string& name)
{
    if (LoadedModules.contains(name))
    {
        // Module already loaded
        IModule* module = LoadedModules.map(name);
        ModuleRefCounts[module]++;
        return module;
    }

    IModule* module = LoadModuleStaticDynamic(name);
    if (!module)
        return nullptr;

    LoadedModules.insert(name, module);
    ModuleRefCounts[module] = 0;
    module->Init();
    return module;
}

bool FModuleService::FreeModule(IModule* module)
{
    auto iter = ModuleRefCounts.find(module);
    if (iter == ModuleRefCounts.end())
    {
        // Module not loaded
        return false;
    }
    else
    {
        if (iter->second != 0)
        {
            // Module still referenced
            iter->second--;
            return true;
        }

        // Otherwize we can actually free it
        module->Shutdown();
        FreeModuleStaticDynamic(module);
        LoadedModules.erase(module);
        ModuleRefCounts.erase(iter);
        return true;
    }
}

void FModuleService::RegisterStaticModule(const std::string& name, PFNCREATEMODULE getModuleFn)
{
    StaticModuleRegistry[name] = getModuleFn;
}

IModule* FModuleService::LoadModuleStaticDynamic(const std::string& name)
{
    IModule* module = nullptr;

    auto staticIter = StaticModuleRegistry.find(name);
    if (staticIter != StaticModuleRegistry.end())
    {
        // We found a static module!
        module = staticIter->second();
    }

    if (!module)
        module = OSLoadModule(name);

    return module;
}

void FModuleService::FreeModuleStaticDynamic(IModule* module)
{
    std::string moduleName = LoadedModules.map(module);
    auto staticModuleIter = StaticModuleRegistry.find(moduleName);
    if (staticModuleIter != StaticModuleRegistry.end())
    {
        // This module is static
        return;
    }

    // Otherwise, we have to actually free the module
    OSFreeModule(module);
}

}

#if TC_OS == TC_OS_WINDOWS_NT
// Since Windows.h invovles so much polluion, we include it at the end
#include <Windows.h>
namespace tc
{

static std::unordered_map<IModule*, HMODULE> ToOSHandle;

IModule* OSLoadModule(const std::string& name)
{
    std::string tryName = name + ".dll";
    HMODULE handle = LoadLibrary(tryName.c_str());
    if (handle == NULL)
        return nullptr;

    std::string impFunctionName = "CreateModule" + name;

    PFNCREATEMODULE getModule = (PFNCREATEMODULE)GetProcAddress(handle, impFunctionName.c_str());
    if (getModule == NULL)
    {
        FreeLibrary(handle);
        return nullptr;
    }

    IModule* module = getModule();
    ToOSHandle[module] = handle;
    return module;
}

void OSFreeModule(IModule* module)
{
    auto iter = ToOSHandle.find(module);
    if (iter != ToOSHandle.end())
    {
        FreeLibrary(iter->second);
        ToOSHandle.erase(iter);
        return;
    }
}

}
#endif
