#pragma once
#include <Parsing/ASTContext.h>
#include <Parsing/SourceManager.h>
#include <Scene/Scene.h>
#include <StrongPointer.h>

#include <QFormLayout>
#include <QMainWindow>
#include <QTimer>
#include <string>
#include <unordered_map>

namespace Ui
{
class MainWindow;
}

namespace Nome
{

using tc::sp;

class CCodeWindow;
class CNome3DView;

class CMainWindow : public QMainWindow, public Scene::ISliderObserver
{
    Q_OBJECT

public:
    explicit CMainWindow(QWidget* parent = nullptr);
    explicit CMainWindow(const std::string& fileToOpen, QWidget* parent = nullptr);
    ~CMainWindow() override;

    [[nodiscard]] const tc::TAutoPtr<Scene::CScene>& GetScene() const { return Scene; }

private slots:
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionReload_triggered();
    void on_actionSave_triggered();
    void on_actionSceneAsObj_triggered();
    void on_actionSceneAsStl_triggered();
    void on_actionMerge_triggered();
    void on_actionPoint_triggered();
    void on_actionInstance_triggered();
    void on_actionAbout_triggered();

private:
    // Load nome files into the current window, only call one of them
    void SetupUI();
    void PreloadSetup();
    void LoadEmptyNomeFile();
    void LoadNomeFile(const std::string& filePath);
    void PostloadSetup();
    void UnloadNomeFile();

    // Slider panel management
    void OnSliderAdded(Scene::CSlider& slider, const std::string& name) override;
    void OnSliderRemoving(Scene::CSlider& slider, const std::string& name) override;

    Ui::MainWindow* ui;
    std::unique_ptr<CNome3DView> Nome3DView;

    bool bIsBlankFile;

    // Nome Context
    sp<CSourceManager> SourceManager;
    CSourceFile* SourceFile = nullptr;
    sp<CASTContext> ASTContext;
    tc::TAutoPtr<Scene::CScene> Scene;
    QTimer* SceneUpdateClock = nullptr;

    std::unique_ptr<QWidget> SliderWidget;
    QFormLayout* SliderLayout = nullptr;
    std::unordered_map<std::string, QLayout*> SliderNameToWidget;
};

}
