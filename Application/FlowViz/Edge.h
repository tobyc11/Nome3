#pragma once

#include "Connector.h"

namespace Flow::Viz
{

class CEdge : public QGraphicsItem
{
public:
    CEdge(CConnector* from, CConnector* to);

    enum
    {
        Type = UserType + 2
    };
    int type() const override { return Type; }

private:
    CConnector* From = nullptr;
    CConnector* To = nullptr;
};

}
