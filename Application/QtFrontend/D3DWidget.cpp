#include "D3DWidget.h"

CD3DWidget::CD3DWidget(QWidget* parent) : QWidget(parent)
{
}

QPaintEngine* CD3DWidget::paintEngine() const
{
	return nullptr;
}
