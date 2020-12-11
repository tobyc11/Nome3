#pragma once
#include <condition_variable>
#include <cstdint>
#include <list>
#include <mutex>

using time_marker_t = uint64_t;

template <typename TQueueItem> class CTimelineQueue
{
public:
    time_marker_t Push(TQueueItem item)
    {
        std::unique_lock<std::mutex> lk(QueueDataMutex);
        time_marker_t marker = NextMarkerValue;
        NextMarkerValue += 1;
        ItemQueue.emplace_back(marker, item);
        return marker;
    }

    bool IsEmpty()
    {
        std::unique_lock<std::mutex> lk(QueueDataMutex);
        return ItemQueue.empty();
    }

    TQueueItem& Peek()
    {
        std::unique_lock<std::mutex> lk(QueueDataMutex);
        return ItemQueue.front().second;
    }

    time_marker_t PeekMarker()
    {
        std::unique_lock<std::mutex> lk(QueueDataMutex);
        return MarkerOfLastFinished + 1;
    }

    void Pop()
    {
        std::unique_lock<std::mutex> lk(QueueDataMutex);
        if (ItemQueue.empty())
            return;
        const auto& front = ItemQueue.front();
        // assert(MarkerOfLastFinished + 1 = front.first);
        MarkerOfLastFinished = front.first;
        ItemQueue.pop_front();
        WaitCV.notify_all();
    }

    void Wait(time_marker_t marker, bool bSpinWait = false)
    {
        std::unique_lock<std::mutex> lk(QueueDataMutex);
        // No waiting if marker is below already finished tasks
        if (marker <= MarkerOfLastFinished)
            return;
        // Wait until the condition is no longer true
        while (marker > MarkerOfLastFinished)
            WaitCV.wait(lk);
    }

private:
    std::mutex QueueDataMutex;
    std::list<std::pair<time_marker_t, TQueueItem>> ItemQueue;
    time_marker_t NextMarkerValue = 1;
    time_marker_t MarkerOfLastFinished = 0;
    std::condition_variable WaitCV;
};
