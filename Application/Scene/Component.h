#pragma once
#include <memory>

namespace Nome::Scene
{

class CEntity;

class CComponent
{
public:
    [[nodiscard]] bool IsAttached() const { return EntityAttachedTo; }
    [[nodiscard]] CEntity* GetEntity() const { return EntityAttachedTo; }

    // Lifecycle methods
    virtual bool OnAttach();
    virtual void OnBeforeDetach();

private:
    friend class CEntity;

    CEntity* EntityAttachedTo = nullptr;
};

}
