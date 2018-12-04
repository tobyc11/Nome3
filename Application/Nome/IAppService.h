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
    ASF_RENDER = 2,
	ASF_EVENT_LOOP_DRIVER = 4
};

#define DEFINE_APP_SERVICE_TYPE(Name)\
public:\
    static constexpr uint32_t TypeHash()\
    {\
        return #Name##_hash;\
    }\
    \
    virtual uint32_t Flags()\
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
    virtual uint32_t Flags()\
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
	friend class CEventLoopDriver;

    virtual ~IAppService() = default;

    virtual int Setup() = 0;
    virtual int FrameUpdate() = 0;
    virtual int Cleanup() = 0;

    //Pointer to an implementation defined event structure
	virtual bool EventHook(void* event) { return false; }
    virtual void Render() {}
	virtual void RenderPhase2() {}
	//Run the event loop once, pump all the events, etc. Returns true if app is closing
	virtual bool EventLoopOnce() { return true; }
};

}
