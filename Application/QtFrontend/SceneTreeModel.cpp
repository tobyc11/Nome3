#include "SceneTreeModel.h"

Nome::CSceneTreeModel::CSceneTreeModel(Scene::CScene* scene, QObject* parent)
    : QAbstractItemModel(parent)
    , Scene(scene)
{
}

Nome::CSceneTreeModel::~CSceneTreeModel() = default;

QVariant Nome::CSceneTreeModel::data(const QModelIndex& index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        auto* treeNode = (Scene::CSceneTreeNode*)index.internalPointer();
        switch (index.column())
        {
        case 0:
            return treeNode->GetOwner()->GetName().c_str();
        case 2:
            return treeNode->GetPath().c_str();
        case 1:
            if (!treeNode->GetParent())
                return "Root";
            if (treeNode->GetEntity())
                return "Instance";
            if (treeNode->GetOwner()->IsGroup())
                return "Group Definition";
            return "Group Instance";
        }
    }
    return QVariant();
}

Qt::ItemFlags Nome::CSceneTreeModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return QAbstractItemModel::flags(index);
}

QVariant Nome::CSceneTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (section == 0)
        {
            return "Name";
        }
        else if (section == 1)
        {
            return "Kind";
        }
    }
    return QVariant();
}

QModelIndex Nome::CSceneTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!parent.isValid())
    {
        return createIndex(row, column, Scene->GetRootTreeNode().Get());
    }
    auto* treeNode = (Scene::CSceneTreeNode*)parent.internalPointer();
    const auto& children = treeNode->GetChildren();
    assert(children.size() > row);
    auto iter = children.begin();
    for (int i = 0; i < row; i++)
        ++iter;
    return createIndex(row, column, *iter);
}

QModelIndex Nome::CSceneTreeModel::parent(const QModelIndex& index) const
{
    if (index.isValid())
    {
        auto* treeNode = (Scene::CSceneTreeNode*)index.internalPointer();
        if (treeNode == Scene->GetRootTreeNode().Get())
            return QModelIndex();
        auto* tnp = treeNode->GetParent();
        auto* tnpp = tnp->GetParent();
        if (!tnpp)
            return createIndex(0, 0, tnp);
        // Find out the index of tnp under tnpp
        int i = 0;
        auto iter = tnpp->GetChildren().begin();
        for (; iter != tnpp->GetChildren().end(); ++iter)
        {
            if (*iter == tnp)
                return createIndex(i, 0, tnp);
            ++i;
        }
    }
    return QModelIndex();
}

int Nome::CSceneTreeModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid())
        return 1;
    auto* treeNode = (Scene::CSceneTreeNode*)parent.internalPointer();
    int childNodeCount = treeNode->GetChildren().size();
    return childNodeCount;
}

int Nome::CSceneTreeModel::columnCount(const QModelIndex& parent) const { return 2; }
