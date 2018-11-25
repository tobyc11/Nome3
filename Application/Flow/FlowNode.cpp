#include "FlowNode.h"

namespace Flow
{

void TOutput<void>::MarkDirty()
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

void TOutput<void>::Connect(TInput<void>& input)
{
    input.Connect(*this);
}

}
