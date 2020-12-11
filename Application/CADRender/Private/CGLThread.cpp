#include "CGLThread.h"
#include <glad/glad.h>
#if CONFIG_IMGUI_INTEGRATION
#include <imgui_impl_opengl3.h>
#endif /* CONFIG_IMGUI_INTEGRATION */

void CGLThread::Run()
{
    assert(!bIsSingleThreaded);
    bDone = false;
    OSThread = std::thread(&CGLThread::ThreadMain, this);
}

void CGLThread::Stop()
{
    assert(!bIsSingleThreaded);
    bDone = true;
    TaskPushCV.notify_one();
    Join();
}

void CGLThread::Join()
{
    assert(!bIsSingleThreaded);
    OSThread.join();
}

void CGLThread::SetSingleThreaded(bool st) { bIsSingleThreaded = st; }

CBufferHandle CGLThread::MakeBuffer(int purpose, size_t size, const void* initialData)
{
    GLuint bufferId;
    std::function<void()> task([&]() {
        glGenBuffers(1, &bufferId);
        glBindBuffer(purpose, bufferId);
        glBufferData(purpose, size, initialData, GL_STATIC_DRAW);
    });
    auto future = QueueTask(std::move(task), true);
    WaitForTask(future);
    return CBufferHandle { bufferId, purpose };
}

void* CGLThread::MapBufferRange(CBufferHandle& handle, size_t offset, size_t length, int access)
{
    void* retMap = nullptr;
    std::function<void()> task([&]() {
        glBindBuffer(handle.BindTarget, handle.BufferId);
        retMap = glMapBufferRange(handle.BindTarget, offset, length, access);
    });
    QueueTask(std::move(task), true, true);
    return retMap;
}

void CGLThread::FlushMappedBufferRange(CBufferHandle& handle, size_t offset, size_t length)
{
    std::function<void()> task([&]() {
        glBindBuffer(handle.BindTarget, handle.BufferId);
        glFlushMappedBufferRange(handle.BindTarget, offset, length);
    });
    QueueTask(std::move(task), true, true);
}

void CGLThread::UnmapBuffer(CBufferHandle& handle)
{
    std::function<void()> task([&]() {
        glBindBuffer(handle.BindTarget, handle.BufferId);
        glUnmapBuffer(handle.BindTarget);
    });
    QueueTask(std::move(task), true, true);
}

void CGLThread::DestroyBuffer(CBufferHandle& handle)
{
    std::function<void()> task([&]() {
        glBindBuffer(handle.BindTarget, 0);
        glDeleteBuffers(1, &handle.BufferId);
        handle.BufferId = 0;
        handle.BindTarget = 0;
    });
    QueueTask(std::move(task), true, true);
}

time_marker_t CGLThread::QueueTask(std::function<void()> task, bool bFast, bool bWait)
{
    // I am confident that the highest bit of this 64bit integer won't be used for time keeping
    // So I am using it to designate the fast/normal queue
    time_marker_t marker;
    {
        if (bFast)
        {
            marker = FastQueue.Push(std::move(task));
            marker = marker | (1ull << 63u);
        }
        else
        {
            marker = NormalQueue.Push(std::move(task));
        }
        std::unique_lock<std::mutex> lk(TaskPushMutex); // Is this needed? Is there any performance impact?
        TaskPushCV.notify_all();
    }
    if (bIsSingleThreaded)
        SingleThreadDrainTaskQueues();
    if (bWait)
        WaitForTask(marker, true);
    return marker;
}

void CGLThread::WaitForTask(time_marker_t marker, bool bSpinWait)
{
    if (marker >> 63u)
        FastQueue.Wait(marker & ~(1ull << 63u), bSpinWait);
    else
        NormalQueue.Wait(marker, bSpinWait);
}

void CGLThread::AsyncSwapBuffers(std::function<void()> callback)
{
    std::function<void()> task([this, cb { std::move(callback) }]() {
        SwapBuffers();
        cb();
    });
    QueueTask(std::move(task), false);
}

#if CONFIG_IMGUI_INTEGRATION
void CGLThread::InitializeForImGui()
{
    std::function<void()> task([&]() { ImGui_ImplOpenGL3_Init("#version 330 core"); });
    QueueTask(std::move(task), true, true);
}

void CGLThread::NewFrameImGui()
{
    std::function<void()> task([&]() { ImGui_ImplOpenGL3_NewFrame(); });
    QueueTask(std::move(task), true, true);
}

void CGLThread::RenderImGui(ImDrawData* drawData)
{
    std::function<void()> task([=]() { ImGui_ImplOpenGL3_RenderDrawData(drawData); });
    QueueTask(std::move(task), false);
}

void CGLThread::ShutdownImGui()
{
    std::function<void()> task([&]() { ImGui_ImplOpenGL3_Shutdown(); });
    QueueTask(std::move(task), true, true);
}
#endif /* CONFIG_IMGUI_INTEGRATION */

void CGLThread::ThreadMain()
{
    MakeCurrent();
    gladLoadGL();
    const auto* ver = glGetString(GL_VERSION);
    printf("%s\n", ver);
    while (!bDone)
    {
        // Honestly I don't know what kind of scheduling algorithm is good here

        // Check whether we have something to do
        std::unique_lock<std::mutex> lk(TaskPushMutex);
        bool bDidSomething = false;

        while (!FastQueue.IsEmpty())
        {
            FastQueue.Peek()();
            FastQueue.Pop();
            bDidSomething = true;
        }

        while (!NormalQueue.IsEmpty())
        {
            NormalQueue.Peek()();
            NormalQueue.Pop();
            bDidSomething = true;
        }

        if (!bDidSomething)
            TaskPushCV.wait(lk);
    }
}

void CGLThread::SingleThreadDrainTaskQueues()
{
    assert(bIsSingleThreaded);
    while (!FastQueue.IsEmpty())
    {
        FastQueue.Peek()();
        FastQueue.Pop();
    }

    while (!NormalQueue.IsEmpty())
    {
        NormalQueue.Peek()();
        NormalQueue.Pop();
    }
}
