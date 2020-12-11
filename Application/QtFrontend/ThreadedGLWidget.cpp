#include "ThreadedGLWidget.h"

CThreadedGLWidget::CThreadedGLWidget(QWidget* parent, Qt::WindowFlags f)
    : QOpenGLWidget(parent, f)
{
    CtxThread = new CContextThread(this);
    // Awkward, but Qt requires such
    CtxThread->moveToThread(CtxThread);
    connect(this, &QOpenGLWidget::aboutToCompose, CtxThread, &CContextThread::PauseMessageProc,
            Qt::BlockingQueuedConnection);
    connect(this, &QOpenGLWidget::frameSwapped, CtxThread, &CContextThread::ResumeMessageProc,
            Qt::DirectConnection);
    connect(this, &QOpenGLWidget::aboutToResize, CtxThread, &CContextThread::PauseMessageProc,
            Qt::BlockingQueuedConnection);
    CtxThread->start();
}

CThreadedGLWidget::~CThreadedGLWidget()
{
    CtxThread->exit();
    delete CtxThread;
}

void CThreadedGLWidget::paintEvent(QPaintEvent* e) { }
void CThreadedGLWidget::initializeGL() { QOpenGLWidget::initializeGL(); }
void CThreadedGLWidget::resizeGL(int w, int h) { QOpenGLWidget::resizeGL(w, h); }
void CThreadedGLWidget::paintGL() { QOpenGLWidget::paintGL(); }

CContextThread::CContextThread(CThreadedGLWidget* widget)
    : QThread()
    , OpenGLWidget(widget)
{
    connect(this, &CContextThread::Doorbell, this, &CContextThread::ProcMessage,
            Qt::QueuedConnection);
}

void CContextThread::PauseMessageProc()
{
    if (!bIsProcessing)
        return;
    bIsProcessing = false;
    OpenGLWidget->context()->moveToThread(OpenGLWidget->thread());
}

void CContextThread::ResumeMessageProc()
{
    if (bIsProcessing)
        return;
    OpenGLWidget->context()->moveToThread(this);
    bIsProcessing = true;
    emit Doorbell();
}

void CContextThread::ProcMessage()
{
    if (!bIsProcessing)
        return;
    OpenGLWidget->makeCurrent();
    if (1)
    {
        QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
        f->glClearColor(ClearRed, 0.4f, 0.3f, 0.0f);
        f->glClear(GL_COLOR_BUFFER_BIT);
    }
    OpenGLWidget->doneCurrent();
}
