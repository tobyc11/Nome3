#pragma once
#include <QWidget>

class CD3DWidget : public QWidget
{
    Q_OBJECT

public:
    CD3DWidget(QWidget* parent);

    QPaintEngine* paintEngine() const override;
};
