#pragma once
#include "NomeViewWidget.h"
#include <Parsing/SourceManager.h>
#include <Parsing/ASTContext.h>
#include <StrongPointer.h>
#include <Scene/Scene.h>

#include <QMainWindow>
#include <string>

namespace Ui {
class MainWindow;
}

namespace Nome
{

using tc::sp;

class CCodeWindow;

class CMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit CMainWindow(QWidget* parent = nullptr);
    CMainWindow(const std::string& fileToOpen, QWidget* parent = nullptr);
    ~CMainWindow();

private slots:
	void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionPoint_triggered();
    void on_actionInstance_triggered();
	void IdleProcess();

private:
	//Load nome files into the current window, only call one of them
    void LoadEmptyNomeFile();
    void LoadNomeFile(const std::string& filePath);
	void UnloadNomeFile();

    Ui::MainWindow *ui;

    bool bIsBlankFile;

    //UI components
    CCodeWindow* CodeWindow;
	CNomeViewWidget* ViewWidget;
	QTimer* IdleTimer;

    //Nome Context
    sp<CSourceManager> SourceManager;
	sp<CSourceFile> SourceFile;
    sp<CASTContext> ASTContext;
	tc::TAutoPtr<Scene::CScene> Scene;
};

}
