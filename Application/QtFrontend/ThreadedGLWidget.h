#pragma once
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QThread>

class CContextThread;

// This is currently unused. It is left here for future reference.
class CThreadedGLWidget : public QOpenGLWidget
{
public:
    explicit CThreadedGLWidget(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    ~CThreadedGLWidget() override;

protected:
    void paintEvent(QPaintEvent* e) override;
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
    // This is the thread that does graphics processing and owns the context
    CContextThread* CtxThread = nullptr;
};

class CContextThread : public QThread
{
    Q_OBJECT

public:
    explicit CContextThread(CThreadedGLWidget* widget);

    // Not just arbitrary messages, message here refers to GL call notifications
    void PauseMessageProc();
    // Must be called on the main thread
    void ResumeMessageProc();

    void PingPong(float newF)
    {
        ClearRed = newF;
        emit Doorbell();
    }

protected:
    void ProcMessage();

Q_SIGNALS:
    void Doorbell();

private:
    CThreadedGLWidget* OpenGLWidget;
    bool bIsProcessing = false;
    float ClearRed = 0.1f;
};
