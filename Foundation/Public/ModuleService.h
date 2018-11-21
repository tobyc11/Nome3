#pragma once
#include "FoundationAPI.h"
#include <string>
#include <map>
#include <unordered_map>

namespace tc
{

class IModule
{
public:
    virtual ~IModule() = default;

	//Never manually call Init and Shutdown, they are managed by the service as well as the ref count
    virtual void Init() = 0;
    virtual void Shutdown() = 0;
};

//For dynamic and static registration
typedef IModule*(*PFNCREATEMODULE)();

template <typename TModule>
class FModuleStaticRegistrant
{
public:
	//Implementation down below
	FModuleStaticRegistrant(const std::string& moduleName);

	static IModule* GetModule()
	{
		return new TModule;
	}
};

#define IMPLEMENT_MODULE(moduleClass, moduleName)\
extern "C" FOUNDATION_API tc::IModule* CreateModule##moduleName() { return new moduleClass; }\
static tc::FModuleStaticRegistrant<moduleClass> ModuleStaticRegistrant##moduleName{#moduleName};\
void DummyFunctionToForceLinking##moduleName() {}

//TODO: we need to be able to check whether we are linking static or dynamic.
//      If dynamic, we omit this.
#define FORCE_LINK_MODULE(moduleName)\
extern void DummyFunctionToForceLinking##moduleName();\
DummyFunctionToForceLinking##moduleName();

//The main module serivce implementation
//TODO: should this be thread safe?

template <typename T1, typename T2>
class FBidirectionalMap
{
public:
	void insert(const T1& t1, const T2& t2)
	{
		Map12.insert(std::make_pair(t1, t2));
		Map21.insert(std::make_pair(t2, t1));
	}

	void erase(const T1& t1)
	{
		auto iter = Map12.find(t1);
		if (iter != Map12.end())
		{
			auto iter2 = std::find_if(Map21.begin(), Map21.end(), [t1](const std::pair<T2, T1>& pair) {
				return pair.second == t1;
			});
			Map12.erase(iter);
			Map21.erase(iter2);
		}
	}

	void erase(const T2& t2)
	{
		auto iter = Map21.find(t2);
		if (iter != Map21.end())
		{
			auto iter2 = std::find_if(Map12.begin(), Map12.end(), [t2](const std::pair<T1, T2>& pair) {
				return pair.second == t2;
			});
			Map21.erase(iter);
			Map12.erase(iter2);
		}
	}

	T2 map(const T1& t1)
	{
		auto iter = Map12.find(t1);
		return iter->second;
	}

	T1 map(const T2& t2)
	{
		auto iter = Map21.find(t2);
		return iter->second;
	}

	bool contains(const T1& t1)
	{
		auto iter = Map12.find(t1);
		return iter != Map12.end();
	}

	bool contains(const T2& t2)
	{
		auto iter = Map21.find(t2);
		return iter != Map21.end();
	}

private:
	std::map<T1, T2> Map12;
	std::map<T2, T1> Map21;
};

class FOUNDATION_API FModuleService
{
public:
	static FModuleService& Get();

    //Load and free modules, keeping track of reference count
    //Can only free when refcount = 0
    IModule* LoadModule(const std::string& name);
    bool FreeModule(IModule* module);

private:
	//For the static registrant to actually register
	template <typename TModule> friend class FModuleStaticRegistrant;
	void RegisterStaticModule(const std::string& name, PFNCREATEMODULE getModuleFn);

	//Does not consider reference count, just loads/releases the module
	IModule* LoadModuleStaticDynamic(const std::string& name);
	void FreeModuleStaticDynamic(IModule* module);

	FBidirectionalMap<std::string, IModule*> LoadedModules;
    std::unordered_map<IModule*, uint32_t> ModuleRefCounts;
	std::unordered_map<std::string, PFNCREATEMODULE> StaticModuleRegistry;
};

template <typename TModule>
FModuleStaticRegistrant<TModule>::FModuleStaticRegistrant(const std::string& moduleName)
{
	FModuleService::Get().RegisterStaticModule(moduleName, &FModuleStaticRegistrant::GetModule);
}

}
