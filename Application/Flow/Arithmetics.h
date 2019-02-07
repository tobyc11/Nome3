#pragma once

#include "FlowNode.h"

namespace Flow
{
    
template <typename T>
class TNumber : public CFlowNode
{
public:
    TNumber() = default;
    TNumber(T num) : Number(num) {}

    T GetNumber() const { return Number; }
    void SetNumber(T val)
    {
        if (val == Number)
            return;
        Number = val;
        Value.MarkDirty();
    }

    DEFINE_OUTPUT_WITH_UPDATE(T, Value) { Value.UpdateValue(Number); }

private:
    T Number;
};

typedef TNumber<int> CIntNumber;
typedef TNumber<unsigned int> CUIntNumber;
typedef TNumber<float> CFloatNumber;
typedef TNumber<double> CDoubleNumber;

template <typename T, typename TOperator>
class TUnaryOperator : public CFlowNode
{
public:
    DEFINE_INPUT(T, Operand0)
    {
        Result.MarkDirty();
    }

    DEFINE_OUTPUT_WITH_UPDATE(T, Result)
    {
        T op0 = Operand0.GetValue(0);
        TOperator op;
        Result.UpdateValue(op(op0));
    }
};

typedef TUnaryOperator<float, std::negate<float>> CFloatNeg;

template <typename T, typename TOperator>
class TBinaryOperator : public CFlowNode
{
public:
    DEFINE_INPUT(T, Operand0)
    {
        Result.MarkDirty();
    }

    DEFINE_INPUT(T, Operand1)
    {
        Result.MarkDirty();
    }

    DEFINE_OUTPUT_WITH_UPDATE(T, Result)
    {
        T op0 = Operand0.GetValue(0);
        T op1 = Operand1.GetValue(0);
        TOperator op;
        Result.UpdateValue(op(op0, op1));
    }
};

typedef TBinaryOperator<float, std::plus<float>> CFloatAdd;
typedef TBinaryOperator<float, std::minus<float>> CFloatSub;
typedef TBinaryOperator<float, std::multiplies<float>> CFloatMul;
typedef TBinaryOperator<float, std::divides<float>> CFloatDiv;
typedef TBinaryOperator<float, std::modulus<float>> CFloatMod;

template <typename T> class TPowOp
{
public:
    constexpr T operator()(const T& _Left, const T& _Right) const
    {
        return std::pow(_Left, _Right);
    }
};
typedef TBinaryOperator<float, TPowOp<float>> CFloatPow;

}
