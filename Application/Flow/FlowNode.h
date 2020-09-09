#pragma once

#include <AutoPtr.h>

#include <functional>
#include <unordered_set>
#include <utility>

namespace Flow
{

class CFlowNode : public tc::FRefCounted
{
public:
    ~CFlowNode() override = default;
};

// Forward declaration
template <typename T> class TOutput;
template <typename T> class TInput;

template <typename T> class TOutput
{
public:
    TOutput(CFlowNode* owner, std::function<void()> updateRoutine)
        : Owner(owner)
        , UpdateRoutine(std::move(updateRoutine))
    {
    }

    // Mark this output dirty, and notify all connected inputs
    void MarkDirty();

    // Unmark dirty
    void UnmarkDirty() { Dirty = false; }

    [[nodiscard]] bool IsDirty() const { return Dirty; }

    // Set a new value, and unmark dirty
    void UpdateValue(T val)
    {
        Value = val;
        UnmarkDirty();
    }

    T GetValue(const T& defaultValue) const
    {
        if (Update())
            return Value;
        return defaultValue;
    }

    [[nodiscard]] size_t CountConnections() const { return ConnectedInputs.size(); }

    // Trigger an update to the value, returns whether the update was successful
    bool Update() const
    {
        if (!IsDirty())
            return true;

        UpdateRoutine();
        return !IsDirty();
    }

    void Connect(TInput<T>& input);

    CFlowNode* GetOwner() const { return Owner; }

private:
    CFlowNode* Owner;
    std::function<void()> UpdateRoutine;

    T Value;
    bool Dirty = true;

    // Friend the corresponding input so that they can access our connections
    friend class TInput<T>;
    std::unordered_set<TInput<T>*> ConnectedInputs;
};

template <typename T> class TInput
{
public:
    TInput(CFlowNode* owner, std::function<void()> dirtyNotifyRoutine)
        : Owner(owner)
        , DirtyNotifyRoutine(std::move(dirtyNotifyRoutine))
    {
    }

    ~TInput() { Disconnect(); }

    void NotifyDirty() { DirtyNotifyRoutine(); }

    void Connect(TOutput<T>& output)
    {
        TOutput<T>* pOut = &output;

        // Don't do anything if nothing changes
        if (pOut == ConnectedOutput)
            return;

        Disconnect();

        ConnectedOutput = pOut;
        if (ConnectedOutput)
        {
            ConnectedOutput->ConnectedInputs.insert(this);
        }
        ConnectedOutput->Owner->AddRef();

        NotifyDirty();
    }

    // template <typename TDerived>
    // std::enable_if_t<std::is_convertible<TDerived, T>::value> Connect(TOutput<TDerived>& output)
    //{
    //}

    void Disconnect()
    {
        if (ConnectedOutput)
        {
            auto iter = ConnectedOutput->ConnectedInputs.find(this);
            if (iter != ConnectedOutput->ConnectedInputs.end())
                ConnectedOutput->ConnectedInputs.erase(iter);
            ConnectedOutput->Owner->Release();
        }
        ConnectedOutput = nullptr;
    }

    [[nodiscard]] bool IsConnected() const { return ConnectedOutput; }

    T GetValue(const T& defaultValue) const
    {
        if (ConnectedOutput)
            return ConnectedOutput->GetValue(defaultValue);
        return defaultValue;
    }

private:
    CFlowNode* Owner;
    std::function<void()> DirtyNotifyRoutine;

    TOutput<T>* ConnectedOutput = nullptr;

    // Only used by InputArray
    template <typename TT> friend class TInputArray;
    void SetDirtyNotifyRoutine(std::function<void()> routine)
    {
        DirtyNotifyRoutine = std::move(routine);
    }
};

template <typename T> void TOutput<T>::MarkDirty()
{
    // Don't mark dirty if already dirty. Safe to do?
    if (IsDirty())
        return;

    Dirty = true;
    for (auto& conn : ConnectedInputs)
    {
        conn->NotifyDirty();
    }
}

template <typename T> void TOutput<T>::Connect(TInput<T>& input) { input.Connect(*this); }

template <> class TInput<void>;

template <> class TOutput<void>
{
public:
    TOutput(CFlowNode* owner, std::function<void()> updateRoutine)
        : Owner(owner)
        , UpdateRoutine(std::move(updateRoutine))
    {
    }

    // Workaround for lambda chicken egg problem
    void SetUpdateRoutine(std::function<void()> updateRoutine)
    {
        UpdateRoutine = std::move(updateRoutine);
    }

    // Mark this output dirty, and notify all connected inputs
    void MarkDirty();

    // Unmark dirty
    void UnmarkDirty() { Dirty = false; }

    bool IsDirty() const { return Dirty; }

    size_t CountConnections() const { return ConnectedInputs.size(); }

    // Trigger an update to the value, returns whether the update was successful
    bool Update() const
    {
        if (!IsDirty())
            return true;

        UpdateRoutine();
        return !IsDirty();
    }

    void Connect(TInput<void>& input);

private:
    CFlowNode* Owner;
    std::function<void()> UpdateRoutine;

    bool Dirty = true;

    // Friend the corresponding input so that they can access our connections
    friend class TInput<void>;
    std::unordered_set<TInput<void>*> ConnectedInputs;
};

template <> class TInput<void>
{
public:
    TInput(CFlowNode* owner, std::function<void()> dirtyNotifyRoutine)
        : Owner(owner)
        , DirtyNotifyRoutine(std::move(dirtyNotifyRoutine))
    {
    }

    ~TInput() { Disconnect(); }

    void NotifyDirty() { DirtyNotifyRoutine(); }

    void Connect(TOutput<void>& output)
    {
        TOutput<void>* pOut = &output;

        // Don't do anything if nothing changes
        if (pOut == ConnectedOutput)
            return;

        Disconnect();

        ConnectedOutput = pOut;
        ConnectedOutput->ConnectedInputs.insert(this);
        ConnectedOutput->Owner->AddRef();

        NotifyDirty();
    }

    void Disconnect()
    {
        if (ConnectedOutput)
        {
            auto iter = ConnectedOutput->ConnectedInputs.find(this);
            if (iter != ConnectedOutput->ConnectedInputs.end())
                ConnectedOutput->ConnectedInputs.erase(iter);
            ConnectedOutput->Owner->Release();
        }
    }

private:
    CFlowNode* Owner;
    std::function<void()> DirtyNotifyRoutine;

    TOutput<void>* ConnectedOutput = nullptr;
};

} /* namespace Flow */

// Finally, macros to help implementing flow nodes

/// Define an input slot, the function is called when an upstream output is marked dirty
#define DEFINE_INPUT(Type, Name)                                                                   \
public:                                                                                            \
    Flow::TInput<Type> Name { this, [=]() { this->Name##MarkedDirty(); } };                        \
                                                                                                   \
private:                                                                                           \
    void Name##MarkedDirty()

/// Define an output slot, the function is called when the value is requested
#define DEFINE_OUTPUT_WITH_UPDATE(Type, Name)                                                      \
public:                                                                                            \
    Flow::TOutput<Type> Name { this, [=]() { this->Name##Update(); } };                            \
                                                                                                   \
private:                                                                                           \
    void Name##Update()
