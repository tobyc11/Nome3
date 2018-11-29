#pragma once

#include <CompileTimeHash.h>

namespace Nome
{

/*
 * Plan:
 *   App initializes the services in order.
 *   User code can fetch services by uint32_t hash.
 */

enum EAppServiceFlags
{
    ASF_NONE = 0,
    ASF_EVENT_HOOK = 1,
    ASF_RENDER = 2
};

#define DEFINE_APP_SERVICE_TYPE(Name)\
public:\
    static constexpr uint32_t TypeHash()\
    {\
        return #Name##_hash;\
    }\
    \
    static constexpr uint32_t Flags()\
    {\
        return ASF_NONE;\
    }

#define DEFINE_APP_SERVICE_TYPE_FLAGS(Name, SvcFlags)\
public:\
    static constexpr uint32_t TypeHash()\
    {\
        return #Name##_hash;\
    }\
    \
    static constexpr uint32_t Flags()\
    {\
        return SvcFlags;\
    }

class IAppService
{
public:
    DEFINE_APP_SERVICE_TYPE(IAppService)

protected:
    ///Only the generic application manager has access to services
    friend class CApp;

    virtual ~IAppService() = default;

    virtual int Setup() = 0;
    virtual int FrameUpdate() = 0;
    virtual int Cleanup() = 0;

    //Pointer to an implementation defined event structure
    virtual void EventHook(void* event) {}
    virtual void Render() {}
};

}
