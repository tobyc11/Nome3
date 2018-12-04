#include "Flow/FlowNode.h"

#include "catch.hpp"

TEST_CASE("FlowNode specializations")
{
    using namespace Flow;
    using tc::TAutoPtr;

    bool target = false;

    TAutoPtr<CFlowNode> flowNode = new CFlowNode();
	TOutput<void> Signal(flowNode, {});
	Signal.SetUpdateRoutine([&]() {
		Signal.UnmarkDirty();
	});

    TInput<void> Slot(flowNode, [&](){
        target = true;
    });

    REQUIRE(!target);

    Signal.Connect(Slot);

    REQUIRE(target);

    target = false;

    Signal.Update();

    REQUIRE(!target);

    Signal.MarkDirty();

    REQUIRE(target);
}
