#include "D3DWidget.h"

CD3DWidget::CD3DWidget(QWidget* parent) : QWidget(parent)
{
	setAttribute(Qt::WA_OpaquePaintEvent);
	setUpdatesEnabled(false);
}

QPaintEngine* CD3DWidget::paintEngine() const
{
	return nullptr;
}
