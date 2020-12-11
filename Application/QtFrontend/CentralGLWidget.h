#pragma once
#include <QOpenGLWidget>
#include <RenderContext.h>

namespace Nome
{

class CCentralGLWidget : public QOpenGLWidget
{
public:
    using QOpenGLWidget::QOpenGLWidget;

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    std::unique_ptr<CRenderContext> RenderContext;
    bool bMouseDown = false;
    QPoint mouseLast;
};

}
