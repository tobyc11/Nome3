#pragma once

#include <QGraphicsView>

namespace Flow::Viz
{

class CFlowViz : public QGraphicsView
{
    Q_OBJECT

public:
    CFlowViz(QWidget* parent = nullptr);

protected:
    //void drawBackground(QPainter* painter, const QRectF& rect) override;

private:
};

}
