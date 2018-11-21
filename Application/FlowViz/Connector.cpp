#include "Connector.h"
#include "Node.h"

#include <QPainter>

namespace Flow::Viz
{

CConnector::CConnector(CNode* parent, QString name, QColor color, EDirection dir)
	: QGraphicsItem(parent), Parent(parent), Name(name), Color(color), Direction(dir)
{
}

QRectF CConnector::boundingRect() const
{
	return QRectF(-40, -40, 40, 40);
}

void CConnector::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	painter->setPen(Qt::NoPen);
	painter->setBrush(Color);

	static const QPoint points_l[] = {
		QPoint(-15, 0),
		QPoint(-7, 10),
		QPoint(10, 10),
		QPoint(10, -10),
		QPoint(-7, -10)
	};
	static const QPoint points_r[] = {
		QPoint(15, 0),
		QPoint(7, 10),
		QPoint(-10, 10),
		QPoint(-10, -10),
		QPoint(7, -10)
	};

	painter->setPen(Qt::white);
	painter->setFont(QFont("Sans", 10));
	if (Direction == CONN_LEFT)
	{
		painter->drawPolygon(points_l, 5);
		painter->drawText(15, 5, Name);
	}
	else
	{
		painter->drawPolygon(points_r, 5);
		painter->drawText(-15, 5, Name);
	}
}

}
