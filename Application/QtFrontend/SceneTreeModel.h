#pragma once
#include <QAbstractItemModel>
#include <Scene/Scene.h>

namespace Nome
{

class CSceneTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit CSceneTreeModel(Scene::CScene* scene, QObject* parent = nullptr);
    ~CSceneTreeModel() override;

    QVariant data(const QModelIndex& index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;

private:
    Scene::CScene* Scene;
};

}
