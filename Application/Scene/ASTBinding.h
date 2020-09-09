#pragma once
#include "BankAndSet.h"
#include "ExprEval.h"
#include <LangUtils.h>
#include <Parsing/SyntaxTree.h>
#include <memory>
#include <stack>
#include <utility>

namespace Nome::Scene
{

// Converts an AST expression tree into a flow node graph
class CExprToNodeGraph : public AST::IExprVisitor
{
public:
    CExprToNodeGraph(AST::AExpr* expr, CBankAndSet& bankAndSet, Flow::TInput<float>* output);

    std::any VisitIdent(AST::AIdent* ident) override;
    std::any VisitNumber(AST::ANumber* number) override;
    std::any VisitUnaryOp(AST::AUnaryOp* unaryOp) override;
    std::any VisitBinaryOp(AST::ABinaryOp* binaryOp) override;
    std::any VisitCall(AST::ACall* call) override;
    std::any VisitVector(AST::AVector* vector) override;
    std::any VisitWrappedExpr(AST::AWrappedExpr* wrapped) override;

private:
    CBankAndSet& BankAndSet;
    std::stack<Flow::TInput<float>*> InputStack;
};

struct CCommandSubpart
{
    std::string NamedArgName;
    int Index;
    int SubIndex = -1;

    AST::AExpr* GetExpr(AST::ACommand* cmd) const;
    bool DoesNamedArgumentExist(AST::ACommand* cmd) const;
};

template <typename TValue> class TBindingTranslator
{
public:
    bool FromASTToValue(AST::ACommand* command, const CCommandSubpart& subpart, TValue& value);
};

class IBindingFunctor
{
public:
    virtual ~IBindingFunctor();
    virtual bool FromASTToObject(AST::ACommand* source, void* object) = 0;

    void SetNamed(std::string name, int index, int subIndex = -1)
    {
        Subpart.NamedArgName = std::move(name);
        Subpart.Index = index;
        Subpart.SubIndex = subIndex;
    }

    void SetPositional(int index, int subIndex = -1)
    {
        Subpart.NamedArgName.clear();
        Subpart.Index = index;
        Subpart.SubIndex = subIndex;
    }

protected:
    CCommandSubpart Subpart;
};

template <typename TMember> class TBindingFunctor : public IBindingFunctor
{
public:
    using TClass = typename tc::TMemberPtrTraits<TMember>::TClass;
    using T = typename tc::TMemberPtrTraits<TMember>::TReturn;

    explicit TBindingFunctor(TMember pMember)
        : Member(pMember)
    {
    }

    bool FromASTToObjectImpl(AST::ACommand* source, TClass* object)
    {
        TBindingTranslator<T> translator;
        return translator.FromASTToValue(source, Subpart, object->*Member);
    }

    bool FromASTToObject(AST::ACommand* source, void* object) override
    {
        return this->FromASTToObjectImpl(source, static_cast<TClass*>(object));
    }

private:
    TMember Member;
};

template <typename TClass, bool CallSuper = true> class CASTBinding
{
public:
    template <typename T> void BindPositionalArgument(T target, int index, int subIndex = -1)
    {
        auto binding = std::make_unique<TBindingFunctor<T>>(target);
        binding->SetPositional(index, subIndex);
        Bindings.push_back(std::move(binding));
    }

    template <typename T>
    void BindNamedArgument(T target, std::string name, int index, int subIndex = -1)
    {
        auto binding = std::make_unique<TBindingFunctor<T>>(target);
        binding->SetNamed(name, index, subIndex);
        Bindings.push_back(std::move(binding));
    }

    bool FromASTToObject(AST::ACommand& command, TClass& object)
    {
        if constexpr (CallSuper)
        {
            using TSuper = typename TClass::Super;
            TSuper::MetaObject.FromASTToObject(command, object);
        }
        bool success = true;
        for (const auto& binding : Bindings)
            if (!binding->FromASTToObject(&command, &object))
                success = false;
        return success;
    }

private:
    std::vector<std::unique_ptr<IBindingFunctor>> Bindings;
};

}
