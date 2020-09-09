#pragma once

namespace Nome::Scene
{

class CScene;

struct CEnvironment
{
    // Not reference counting so might be dangling
    CScene* Scene;
};

extern thread_local CEnvironment GEnv;

}
