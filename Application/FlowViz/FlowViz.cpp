#include "FlowViz.h"
#include "Connector.h"
#include "Node.h"

namespace Flow::Viz
{

CFlowViz::CFlowViz(QWidget* parent)
    : QGraphicsView(parent)
{
    QGraphicsScene* scene = new QGraphicsScene(this);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    setScene(scene);
    setCacheMode(CacheBackground);
    setViewportUpdateMode(BoundingRectViewportUpdate);
    setRenderHint(QPainter::Antialiasing);
    setTransformationAnchor(AnchorUnderMouse);

    setMinimumSize(400, 400);
    setWindowTitle(tr("Dataflow Visualization"));

    auto* node = new CNode();
    node->setPos(-10, 100);
    node->AddInputConnector("new connector");
    node->AddInputConnector("conn2");
    node->AddOutputConnector("asdf");
    node->AddOutputConnector("ghjlk");
    scene->addItem(node);
}

// void CFlowViz::drawBackground(QPainter* painter, const QRectF& rect)
//{
//}

}
