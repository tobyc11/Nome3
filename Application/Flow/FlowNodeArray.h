#pragma once
#include "FlowNode.h"
#include <vector>

namespace Flow
{

template <typename T> class TInputArray
{
public:
    TInputArray(CFlowNode* owner, std::function<void(TInput<T>*)> dirtyNotifyRoutine)
        : Owner(owner)
        , DirtyNotifyRoutine(std::move(dirtyNotifyRoutine))
    {
    }

    ~TInputArray() { DisconnectAll(); }

    void Connect(TOutput<T>& output)
    {
        TInput<T>* input = new TInput<T>(Owner, std::function<void()>());
        input->SetDirtyNotifyRoutine([this, input]() { DirtyNotifyRoutine(input); });
        input->Connect(output);
        InputArray.push_back(input);
    }

    void DisconnectAll()
    {
        for (TInput<T>* input : InputArray)
        {
            delete input;
        }
        InputArray.clear();
    }

    bool IsConnected() const { return !InputArray.empty(); }

    size_t GetSize() const { return InputArray.size(); }

    T GetValue(size_t index, const T& defaultValue) const
    {
        if (index < InputArray.size())
            return InputArray[index]->GetValue(defaultValue);
        return defaultValue;
    }

    template <typename U>
    std::vector<U> MapOutput(std::function<U(const TOutput<T>&)> f) const
    {
        std::vector<U> result;
        for (TInput<T>* input : InputArray)
        {
            // Assuming ConnectedOutput is not nullptr
            result.push_back(f(*input->ConnectedOutput));
        }
        return result;
    }

private:
    CFlowNode* Owner;
    std::function<void(TInput<T>*)> DirtyNotifyRoutine;

    std::vector<TInput<T>*> InputArray;
};

}

#define DEFINE_INPUT_ARRAY(Type, Name)                                                             \
public:                                                                                            \
    Flow::TInputArray<Type> Name { this, [=](Flow::TInput<Type>* input) {                          \
                                      this->Name##MarkedDirty(input);                              \
                                  } };                                                             \
                                                                                                   \
private:                                                                                           \
    inline void Name##MarkedDirty(Flow::TInput<Type>* input)
