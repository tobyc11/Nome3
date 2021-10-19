#pragma once
#include "CentralGLWidget.h"
#include <Parsing/SourceManager.h>
#include <Scene/Scene.h>
#include <Scene/TemporaryMeshManager.h>

#include <QFormLayout>
#include <QLineEdit>
#include <QMainWindow>
#include <QTimer>
#include <QTreeView>
#include <string>
#include <unordered_map>

namespace Ui
{
class MainWindow;
}

namespace Nome
{

class CMainWindow : public QMainWindow, public Scene::ISliderObserver
{
    Q_OBJECT

public:
    explicit CMainWindow(QWidget* parent = nullptr, bool bDetached3d = false);
    explicit CMainWindow(const QString& fileToOpen, QWidget* parent = nullptr, bool bDetached3d = false);
    ~CMainWindow() override;

    [[nodiscard]] const tc::TAutoPtr<Scene::CScene>& GetScene() const { return Scene; }

protected:
    void closeEvent(QCloseEvent* event) override;

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
    void on_actionScene_as_STL_triggered();

    void on_actionAddFace_triggered();
    void on_actionAddPolyline_triggered();
    void on_actionResetTempMesh_triggered();
    void on_actionCommitTempMesh_triggered();

private:
    // Load nome files into the current window, only call one of them
    void SetupUI();
    void LoadEmptyNomeFile();
    void LoadNomeFile(const std::string& filePath);
    void PostloadSetup();
    void UnloadNomeFile();

    // Slider panel management
    void OnSliderAdded(Scene::CSlider& slider, const std::string& name) override;
    void OnSliderRemoving(Scene::CSlider& slider, const std::string& name) override;

    Ui::MainWindow* ui;
    CCentralGLWidget* GLWidget = nullptr;
    QLineEdit* InstName = nullptr;
    QLineEdit* MeshName = nullptr;

    // Info about the currently open file
    std::shared_ptr<CSourceManager> SourceMgr;
    bool bIsBlankFile;

    // Nome Context (mostly saved, persistent states)
    tc::TAutoPtr<Scene::CScene> Scene;
    QTimer* SceneUpdateClock = nullptr;

    // Below are runtime states (not saved of course)
    std::unique_ptr<QWidget> SliderWidget;
    QFormLayout* SliderLayout = nullptr;
    std::unordered_map<std::string, QLayout*> SliderNameToWidget;
    QTreeView* SceneTreeView = nullptr;

    std::unique_ptr<Scene::CTemporaryMeshManager> TemporaryMeshManager;
};

}
