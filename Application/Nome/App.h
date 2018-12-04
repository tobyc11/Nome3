#pragma once

#include "IAppService.h"

#include <unordered_map>
#include <vector>

namespace Nome
{

class CApp
{
public:
    enum
    {
        StatusWannaQuit = 1
    };

    //Global function called once to setup the application
    static int AppSetup();

    //Global function called every frame, returns stats flags
    static int AppSingleFrame();

    //Cleanup all resources
    static int AppCleanup();

    virtual void Init() = 0;
    virtual void Shutdown() = 0;

    template <typename T>
    void BootService()
    {
        IAppService* svc = new T();

        //TODO: conflict check
        Services[T::TypeHash()] = svc;

        ServicesInBootOrder.push_back(svc);

        //Handle flags
        if (svc->Flags() & ASF_EVENT_HOOK)
            EventSvcs.push_back(svc);
        if (svc->Flags() & ASF_RENDER)
            RenderSvcs.push_back(svc);

        svc->Setup();
    }

    template <typename T>
    T* GetService()
    {
        auto iter = Services.find(T::TypeHash());
        if (iter != Services.end())
            return static_cast<T*>(iter->second);
        return nullptr;
    }

private:
    void ShutdownServices();
	
	friend class CEventLoopDriver;
    std::unordered_map<uint32_t, IAppService*> Services;
    std::vector<IAppService*> ServicesInBootOrder;

    //Memoized services satisfying certain conditions
    std::vector<IAppService*> EventSvcs;
    std::vector<IAppService*> RenderSvcs;
};

extern CApp* GApp;

#define REGISTER_APPLICATION(TClass)\
static TClass Global##TClass##Instance;\
CApp* GApp = &Global##TClass##Instance;

}
