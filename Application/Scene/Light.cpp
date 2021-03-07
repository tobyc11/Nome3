#include "Light.h"
#include "SceneGraph.h"
#include <StringPrintf.h>
#include <StringUtils.h>

namespace Nome::Scene {

DEFINE_META_OBJECT(CLight)
{
    BindNamedArgument(&CLight::type, "type", 0);
    BindNamedArgument(&CLight::R,"color", 0, 0);
    BindNamedArgument(&CLight::G,"color", 0, 1);
    BindNamedArgument(&CLight::B,"color", 0, 2);
}

void CLight::MarkDirty()
{
    // Mark this entity dirty
    Super::MarkDirty();
}

bool CLight::IsInstantiable() { return true; }

bool CLight::IsMesh() { return false; }

CEntity* CLight::Instantiate(CSceneTreeNode* treeNode) { return new CLightInstance(this, treeNode); }


AST::ACommand* CLight::SyncToAST(AST::CASTContext& ctx, bool createNewNode)
{
    if (!createNewNode)
        throw "unimplemented";
    auto* node = ctx.Make<AST::ACommand>(ctx.MakeToken("mesh"), ctx.MakeToken("endmesh"));
    node->PushPositionalArgument(ctx.MakeIdent(GetName()));
    return node;
}

void CLight::UpdateEntity() {
    if (!IsDirty())
        return;

    Super::UpdateEntity();
    LI.color = { R.GetValue(0.0f), G.GetValue(0.0f), B.GetValue(0.0f) };
    LI.type = type.GetValue("NOME_AMBIENT");
    LI.name = GetName();
    SetValid(true);
    //TODO:


}
CLightInstance::CLightInstance(CLight* generator, CSceneTreeNode* stn)
    : LightGenerator(generator)
    , SceneTreeNode(stn)
{
    SetName(tc::StringPrintf("_%s_%s", LightGenerator->GetName().c_str(), GetName().c_str()));
    LightGenerator->InstanceSet.insert(this);

    // We listen to the transformation changes of the associated tree node
    TransformChangeConnection = SceneTreeNode->OnTransformChange.Connect(
        [this] { MarkOnlyDownstreamDirty(); });
}
CLightInstance::~CLightInstance()
{
    // TODO: handle this circular reference stuff
    // SceneTreeNode->OnTransformChange.Disconnect(TransformChangeConnection);
    LightGenerator->InstanceSet.erase(this);
}


void CLightInstance::MarkDirty()
{
    Super::MarkDirty();
}

void CLightInstance::UpdateEntity()
{
    if (!IsDirty())
        return;
    LightGenerator->UpdateEntity();
    Super::UpdateEntity();
    SetValid(LightGenerator->IsEntityValid());
}

}
