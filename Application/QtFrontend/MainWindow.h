#pragma once
#include <Parsing/SourceManager.h>
#include <Scene/Scene.h>
#include <Scene/TemporaryMeshManager.h>
#include <Scene/MeshMerger.h>

#include <QFormLayout>
#include <QLineEdit>
#include <QMainWindow>
#include <QTimer>
#include <QElapsedTimer>
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
    void on_actionExportAsStl_triggered();

    // 10/11 Xinyu to add save to stl capability
    // void on_actionSceneAsObj_triggered();


    void on_actionMerge_triggered();
    void on_actionSubdivide_triggered();

    /*  10/1 Randy Commenting out because these are not fully implemented right now
    void on_actionPoint_triggered();
    void on_actionInstance_triggered();
    */
    // void on_actionAbout_triggered(); commeting out with new developments

    void on_actionAddPoint_triggered(); // Steven's Add Point

    void on_actionAddFace_triggered();
    void on_actionAddPolyline_triggered(); 

     void on_actionRemoveFace_triggered(); // Randy added this. Buggy implementation.
    // void on_actionSharpenFace_triggered(); // Randy added this. Not currently used.

    /* Randy temporarily commenting out due to temporarymeshmanager changes
    void on_actionResetTempMesh_triggered(); */

    void displayRay(); // Randy added this on 2/26
    void displayPoint(); // Randy added this on 3/20
    void on_actionToggleRenderRay_triggered(); // Randy added this on 2/26 for adding points via ray

    void on_actionCommitChanges_triggered();

    void on_actionShowFacets_triggered(); // Randy added this
    void on_actionToggleVertexSelection_triggered(); // Randy added this on 11/5
    void on_actionToggleSharpVertexSelection_triggered(); // Randy added this on 6/20/2021
    void on_actionToggleEdgeSelection_triggered(); // Randy added this on 11/5
    void on_actionTogglePolylineSelection_triggered(); // Randy added this on 12/22
    void on_actionToggleFaceSelection_triggered(); // Randy added this on 11/5

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
    QElapsedTimer* elapsedRender;

    std::unique_ptr<QWidget> SliderWidget;
    QFormLayout* SliderLayout = nullptr;
    std::unordered_map<std::string, QLayout*> SliderNameToWidget;

    std::unique_ptr<Scene::CTemporaryMeshManager> TemporaryMeshManager;
};

}
