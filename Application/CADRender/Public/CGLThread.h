#pragma once
#include "Buffer.h"
#include "TimelineQueue.h"
#include <functional>
#if CONFIG_IMGUI_INTEGRATION
#include <imgui.h>
#endif /* CONFIG_IMGUI_INTEGRATION */
#include <thread>

// This is more like a device now that I think about it
// Logically acts like a client interface to the server that is the GPU
class CGLThread
{
public:
    CGLThread(std::function<void()>&& makeCurrent, std::function<void()>&& swapBuffers)
        : MakeCurrent(makeCurrent)
        , SwapBuffers(swapBuffers)
    {
    }
    void Run();
    void Stop();
    void Join();
    void SetSingleThreaded(bool st);

    CBufferHandle MakeBuffer(int purpose, size_t size, const void* initialData = nullptr);
    void* MapBufferRange(CBufferHandle& handle, size_t offset, size_t length, int access);
    void FlushMappedBufferRange(CBufferHandle& handle, size_t offset, size_t length);
    void UnmapBuffer(CBufferHandle& handle);
    void DestroyBuffer(CBufferHandle& handle);

    // Essential methods to support async tasks
    // Can operate in single-threaded mode
    time_marker_t QueueTask(std::function<void()> task, bool bFast = false, bool bWait = false);
    void WaitForTask(time_marker_t marker, bool bSpinWait = false);
    void AsyncSwapBuffers(std::function<void()> callback);

#if CONFIG_IMGUI_INTEGRATION
    void InitializeForImGui();
    void NewFrameImGui();
    void RenderImGui(ImDrawData* drawData);
    void ShutdownImGui();
#endif /* CONFIG_IMGUI_INTEGRATION */

protected:
    void ThreadMain();
    void SingleThreadDrainTaskQueues();

private:
    std::function<void()> MakeCurrent, SwapBuffers;
    std::thread OSThread;
    bool bDone {};
    bool bIsSingleThreaded = false;
    // Thread safe timeline queues
    std::mutex TaskPushMutex;
    std::condition_variable TaskPushCV;
    CTimelineQueue<std::function<void()>> NormalQueue, FastQueue;
};
