#pragma once

#include <QGraphicsItem>

namespace Flow::Viz
{

class CNode;

class CConnector : public QGraphicsItem
{
public:
    enum EDirection { CONN_LEFT, CONN_RIGHT };

    CConnector(CNode* parent, QString name, QColor color = Qt::blue, EDirection dir = CONN_LEFT);

    enum { Type = UserType + 3 };
    int type() const override { return Type; }

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    CNode* GetParent() const { return Parent; }
    
    QString GetName() const { return Name; }

    QColor GetColor() const { return Color; }
    void SetColor(QColor value) { Color = value; }

    EDirection GetDirection() const { return Direction; }

private:
    CNode* Parent;
    QString Name;
    QColor Color;
    EDirection Direction;
};

}
