#pragma once

#include <QGraphicsItem>

#include <map>
#include <string>

namespace Flow::Viz
{

class CConnector;

class CNode : public QGraphicsItem
{
public:
    CNode();

    enum
    {
        Type = UserType + 1
    };
    int type() const override { return Type; }

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    // Interfaces exposed for consumers
    std::string GetName() const { return Name.toStdString(); }
    void SetName(const std::string& str) { Name = QString::fromStdString(str); }

    CConnector* AddInputConnector(const std::string& name, QColor color = Qt::blue);
    CConnector* AddOutputConnector(const std::string& name, QColor color = Qt::blue);

protected:
    // QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    // void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    // void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    QString Name;

    int Width = 150;
    int Height = 200;

    std::map<std::string, CConnector*> InputConnectors;
    std::map<std::string, CConnector*> OutputConnectors;
};

}
