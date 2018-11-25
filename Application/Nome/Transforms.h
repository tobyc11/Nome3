#pragma once

#include "Flow/FlowNode.h"

#include <Matrix3x4.h>

namespace Nome::Scene
{

using tc::Vector3;
using tc::Quaternion;
using tc::Matrix3x4;

class CTranslate : public Flow::CFlowNode
{
public:
    DEFINE_INPUT_WITH_MARKDIRTY(Matrix3x4, Input)
    {
        Output.MarkDirty();
    }

    DEFINE_INPUT_WITH_MARKDIRTY(float, X)
    {
        Output.MarkDirty();
    }

    DEFINE_INPUT_WITH_MARKDIRTY(float, Y)
    {
        Output.MarkDirty();
    }

    DEFINE_INPUT_WITH_MARKDIRTY(float, Z)
    {
        Output.MarkDirty();
    }

    DEFINE_OUTPUT_WITH_UPDATE(Matrix3x4, Output)
    {
        RecomputeOutput();
    }

private:
    void RecomputeOutput();
};

class CRotate : public Flow::CFlowNode
{
public:
    DEFINE_INPUT_WITH_MARKDIRTY(Matrix3x4, Input)
    {
        Output.MarkDirty();
    }

    DEFINE_INPUT_WITH_MARKDIRTY(float, AxisX)
    {
        Output.MarkDirty();
    }

    DEFINE_INPUT_WITH_MARKDIRTY(float, AxisY)
    {
        Output.MarkDirty();
    }

    DEFINE_INPUT_WITH_MARKDIRTY(float, AxisZ)
    {
        Output.MarkDirty();
    }

    DEFINE_INPUT_WITH_MARKDIRTY(float, Angle)
    {
        Output.MarkDirty();
    }

    DEFINE_OUTPUT_WITH_UPDATE(Matrix3x4, Output)
    {
        RecomputeOutput();
    }

private:
    void RecomputeOutput();
};

class CScale : public Flow::CFlowNode
{
public:
    DEFINE_INPUT_WITH_MARKDIRTY(Matrix3x4, Input)
    {
        Output.MarkDirty();
    }

    DEFINE_INPUT_WITH_MARKDIRTY(float, X)
    {
        Output.MarkDirty();
    }

    DEFINE_INPUT_WITH_MARKDIRTY(float, Y)
    {
        Output.MarkDirty();
    }

    DEFINE_INPUT_WITH_MARKDIRTY(float, Z)
    {
        Output.MarkDirty();
    }

    DEFINE_OUTPUT_WITH_UPDATE(Matrix3x4, Output)
    {
        RecomputeOutput();
    }

private:
    void RecomputeOutput();
};

}
