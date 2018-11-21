#pragma once

#include <unordered_set>
#include <functional>

namespace Flow
{

class CFlowNode
{
public:
    virtual ~CFlowNode() = default;
};

//Forward declaration
template <typename T> class TOutput;
template <typename T> class TInput;

template <typename T>
class TOutput
{
public:
    TOutput(CFlowNode* owner, std::function<void()> updateRoutine)
        : Owner(owner), UpdateRoutine(updateRoutine)
    {
    }

    //Mark this output dirty, and notify all connected inputs
    void MarkDirty();

    //Unmark dirty
	void UnmarkDirty() { Dirty = false; }

    bool IsDirty() const { return Dirty; }

    //Set a new value, and unmark dirty
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

    unsigned CountConnections() const
    {
        return ConnectedInputs.size();
    }

    //Trigger an update to the value, returns whether the update was successful
    bool Update() const
    {
		if (!IsDirty())
			return true;

        UpdateRoutine();
        return !IsDirty();
    }

	void Connect(TInput<T>& input)
	{
		input.Connect(*this);
	}

private:
    CFlowNode* Owner;
    std::function<void()> UpdateRoutine;

    T Value;
    bool Dirty = true;

	//Friend the corresponding input so that they can access our connections
	friend class TInput<T>;
    std::unordered_set<TInput<T>*> ConnectedInputs;
};

template <typename T>
class TInput
{
public:
    TInput(CFlowNode* owner, std::function<void()> dirtyNotifyRoutine)
        : Owner(owner), DirtyNotifyRoutine(dirtyNotifyRoutine)
    {
    }

    void NotifyDirty()
    {
        DirtyNotifyRoutine();
    }

	void Connect(TOutput<T>& output)
	{
		TOutput<T>* pOut = &output;

		//Don't do anything if nothing changes
		if (pOut == ConnectedOutput)
			return;

		if (ConnectedOutput)
		{
			auto iter = ConnectedOutput->ConnectedInputs.find(this);
			if (iter != ConnectedOutput->ConnectedInputs.end())
				ConnectedOutput->ConnectedInputs.erase(iter);
		}
		ConnectedOutput = pOut;
		if (ConnectedOutput)
		{
			ConnectedOutput->ConnectedInputs.insert(this);
		}

		NotifyDirty();
	}

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
};

template <typename T>
void TOutput<T>::MarkDirty()
{
	//Don't mark dirty if already dirty. Safe to do?
	if (IsDirty())
		return;

	Dirty = true;
	for (auto& conn : ConnectedInputs)
	{
		conn->NotifyDirty();
	}
}

}

//Finally, macros to help implemeting flow nodes

#define DEFINE_INPUT_WITH_MARKDIRTY(Type, Name) \
public:\
Flow::TInput<Type> Name{ this, [=]() {this->Name##MarkedDirty(); } }; \
private:\
void Name##MarkedDirty()

#define DEFINE_OUTPUT_WITH_UPDATE(Type, Name) \
public:\
Flow::TOutput<Type> Name{ this, [=]() {this->Name##Requested(); } }; \
private:\
void Name##Requested()
