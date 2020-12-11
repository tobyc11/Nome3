#include "CentralGLWidget.h"
#include "MainWindow.h"
#include <QMouseEvent>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <Scene/RenderComponent.h>

namespace Nome
{

static QOpenGLContext* ContextLoadFrom = nullptr;

static void* MyLoadGLProc(const char* name) { return reinterpret_cast<void*>(ContextLoadFrom->getProcAddress(name)); }

void CCentralGLWidget::initializeGL()
{
    // Create a CADRender Context and put it into single threaded mode
    RenderContext = std::make_unique<CRenderContext>(true);
    ContextLoadFrom = context();
    RenderContext->LoadGL(&MyLoadGLProc);
    ContextLoadFrom = nullptr;
}

void CCentralGLWidget::resizeGL(int w, int h)
{
    auto* mainWindow = dynamic_cast<CMainWindow*>(this->parentWidget()->parentWidget());
    const auto& scene = mainWindow->GetScene();
    auto& camera = scene->GetDefaultCamera();
    camera.SetFovY(90.0f);
    camera.SetAspectRatio((float)w / h);
}

void CCentralGLWidget::paintGL()
{
    CDrawList drawList;
    auto* mainWindow = dynamic_cast<CMainWindow*>(this->parentWidget()->parentWidget());
    const auto& scene = mainWindow->GetScene();
    for (Scene::CRenderComponent* renderComp : scene->GetRenderCompList())
        renderComp->Update();
    drawList.Clear(0.0f, 0.0f, 0.0f, 0.0f);
    drawList.SetCamera(scene->GetDefaultCamera().CalculateProjMatrix(), scene->GetDefaultCamera().GetViewMatrix());
    drawList.SetPointLight({ 2.0f, 10.0f, -3.0f });
    drawList.DrawGradientBackground({ 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f });
    drawList.DrawGrid();
    for (Scene::CRenderComponent* renderComp : scene->GetRenderCompList())
        renderComp->Draw(drawList);
    drawList.Wait();
}

void CCentralGLWidget::mousePressEvent(QMouseEvent* event)
{
    bMouseDown = true;
    mouseLast = event->pos();
    event->accept();
}

void CCentralGLWidget::mouseReleaseEvent(QMouseEvent* event)
{
    bMouseDown = false;
    event->accept();
}

void CCentralGLWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (bMouseDown)
    {
        auto* mainWindow = dynamic_cast<CMainWindow*>(this->parentWidget()->parentWidget());
        const auto& scene = mainWindow->GetScene();

        auto mouseNow = event->pos();
        auto mouseDelta = mouseNow - mouseLast;
        scene->GetDefaultCamera().Orbit(Scene::CCamera::EDir::Left, mouseDelta.x() / 10.f);
        scene->GetDefaultCamera().Orbit(Scene::CCamera::EDir::Up, mouseDelta.y() / 10.f);
        mouseLast = mouseNow;
        event->accept();
    }
}

}
