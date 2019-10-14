#include "Node.h"
#include "Connector.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>

namespace Flow::Viz
{

CNode::CNode()
{
    setFlag(ItemIsMovable);
    // setFlag(ItemSendsGeometryChanges);
    setCacheMode(ItemCoordinateCache);
    setZValue(-1);
}

QRectF CNode::boundingRect() const { return QRectF(0, 0, Width, Height); }

void CNode::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    QLinearGradient bgGradient(0, 0, 0, 30);
    bgGradient.setColorAt(0, QColor(200, 200, 200));
    bgGradient.setColorAt(0.5, QColor(55, 55, 55));
    bgGradient.setColorAt(1, QColor(33, 33, 33));

    painter->setPen(Qt::NoPen);
    painter->setBrush(bgGradient);
    painter->drawRect(0, 0, Width, Height);

    // Draw title
    int fontSize = 12;
    painter->setPen(Qt::white);
    painter->setFont(QFont("Sans", fontSize));
    painter->drawText(10, 15 + fontSize / 2, Name);
}

CConnector* CNode::AddInputConnector(const std::string& name, QColor color)
{
    auto* conn = new CConnector(this, QString::fromStdString(name), color, CConnector::CONN_LEFT);
    InputConnectors.insert({ name, conn });

    // Respace the connectors
    const int spacing = 10;
    const int connectorHeight = 20;
    int nextY = 50;
    for (auto& pair : InputConnectors)
    {
        pair.second->setPos(0, nextY);
        nextY += spacing + connectorHeight;
    }

    return conn;
}

CConnector* CNode::AddOutputConnector(const std::string& name, QColor color)
{
    auto* conn = new CConnector(this, QString::fromStdString(name), color, CConnector::CONN_RIGHT);
    OutputConnectors.insert({ name, conn });

    // Respace the connectors
    const int spacing = 10;
    const int connectorHeight = 20;
    int nextY = 50;
    for (auto& pair : OutputConnectors)
    {
        pair.second->setPos(Width, nextY);
        nextY += spacing + connectorHeight;
    }

    return conn;
}

}