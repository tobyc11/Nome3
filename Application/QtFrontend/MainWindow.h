#pragma once
#include <Parsing/SourceManager.h>
#include <Scene/Scene.h>
#include <Scene/TemporaryMeshManager.h>

#include <QFormLayout>
#include <QLineEdit>
#include <QMainWindow>
#include <QTimer>
#include <iostream>
#include <string>
#include <unordered_map>

namespace Ui
{
class MainWindow;
}

namespace Nome
{

class CCodeWindow;
class CNome3DView;

class CMainWindow : public QMainWindow, public Scene::ISliderObserver
{
    Q_OBJECT

public:
    explicit CMainWindow(QWidget* parent = nullptr, bool bDetached3d = false);
    explicit CMainWindow(const QString& fileToOpen, QWidget* parent = nullptr,
                         bool bDetached3d = false);
    ~CMainWindow() override;

    [[nodiscard]] const tc::TAutoPtr<Scene::CScene>& GetScene() const { return Scene; }

protected:
    void closeEvent(QCloseEvent* event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionReload_triggered();
    void on_actionSave_triggered();

    /* 10/1 Randy commenting out to avoid annoying error message when opening executable
    void on_actionSceneAsObj_triggered();
    void on_actionSceneAsStl_triggered();
    */
    void on_actionMerge_triggered();
    void on_actionSubdivide_triggered();

    /*  10/1 Randy Commenting out because these are not fully implemented right now
    void on_actionPoint_triggered();
    void on_actionInstance_triggered();
    */
    // void on_actionAbout_triggered(); commeting out with new developments


    void on_actionAddFace_triggered();
    void on_actionAddPolyline_triggered(); 

    void on_actionRemoveFace_triggered();

    /* Randy temporarily commenting out due to temporarymeshmanager changes
    void on_actionResetTempMesh_triggered(); */

    void on_actionCommitChanges_triggered();

private:
    // Load nome files into the current window, only call one of them
    void SetupUI();
    void LoadEmptyNomeFile();
    void LoadNomeFile(const std::string& filePath);
    void PostloadSetup();
    void UnloadNomeFile();
    bool hasEnding(std::string const &str, std::string const &end) {
        if (str.length() >= end.length()) {
            return (0 == str.compare(str.length() - end.length(), end.length(), end));
        } else {
            return false;
        }
    };
    // Slider panel management
    void OnSliderAdded(Scene::CSlider& slider, const std::string& name) override;
    void OnSliderRemoving(Scene::CSlider& slider, const std::string& name) override;

    Ui::MainWindow* ui;
    std::unique_ptr<CNome3DView> Nome3DView;
    //QLineEdit* InstName; // Randy decided not to use this for now. This was originally intended to allow users to name their added faces/polylines
    //QLineEdit* MeshName; // Randy decided not to use this for now. This was originally intended to allow users to name their added faces/polylines
    bool bDetached3DView = false;

    // Info about the currently open file
    std::shared_ptr<CSourceManager> SourceMgr;
    bool bIsBlankFile;

    // Nome Context
    tc::TAutoPtr<Scene::CScene> Scene;
    QTimer* SceneUpdateClock = nullptr;
    QTimer* timer = nullptr;

    std::unordered_map<std::string, QTimer*> SliderTimers;
    std::unique_ptr<QWidget> SliderWidget;
    QFormLayout* SliderLayout = nullptr;
    std::unordered_map<std::string, QLayout*> SliderNameToWidget;

    std::unique_ptr<Scene::CTemporaryMeshManager> TemporaryMeshManager;
};

}
