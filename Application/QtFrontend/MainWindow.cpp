#include "MainWindow.h"
#include "CodeWindow.h"
#include "ui_MainWindow.h"

#include <Scene/ASTSceneAdapter.h>
#include <Scene/Environment.h>
#include <Scene/MeshMerger.h>

#include <QDockWidget>
#include <QFileDialog>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QSettings>
#include <QSlider>
#include <QToolBar>
#include <QVBoxLayout>
#include <StringPrintf.h>

namespace Nome
{

CMainWindow::CMainWindow(QWidget* parent, bool bDetached3d)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , bIsBlankFile(true)
{
    ui->setupUi(this);
    SetupUI();
    LoadEmptyNomeFile();
}

CMainWindow::CMainWindow(const QString& fileToOpen, QWidget* parent, bool bDetached3d)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , bIsBlankFile(false)
{
    ui->setupUi(this);
    SetupUI();
    LoadNomeFile(fileToOpen.toStdString());
}

CMainWindow::~CMainWindow()
{
    UnloadNomeFile();
    delete ui;
}

void CMainWindow::closeEvent(QCloseEvent* event) { QWidget::closeEvent(event); }

void CMainWindow::on_actionNew_triggered()
{
    if (!bIsBlankFile)
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "New File", "Your existing work will be discarded, continue?",
                                      QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes)
        {
            UnloadNomeFile();
            LoadEmptyNomeFile();
        }
    }
    else
    {
        UnloadNomeFile();
        LoadEmptyNomeFile();
    }
}

void CMainWindow::on_actionOpen_triggered()
{
    QSettings appSettings;
    const QString kDefaultDir("DefaultDir");

    QString fileName = QFileDialog::getOpenFileName(
        this, tr("Open Nome File"), appSettings.value(kDefaultDir).toString(), tr("Nome Code (*.nom);;All Files (*)"));
    if (fileName.isEmpty())
        return;

    QDir currentDir;
    appSettings.setValue(kDefaultDir, currentDir.absoluteFilePath(fileName));
    if (true /*bIsBlankFile*/)
    {
        UnloadNomeFile();
        LoadNomeFile(fileName.toStdString());
    }
    else
    {
        // Open in new window
        // new CMainWindow(fileName.toStdString());
        // Not possible for now since ImGui supports only one context per process
    }
}

void CMainWindow::on_actionReload_triggered()
{
    UnloadNomeFile();
    if (!SourceMgr || SourceMgr->GetMainSourcePath().empty())
        LoadEmptyNomeFile();
    else
        LoadNomeFile(SourceMgr->GetMainSourcePath());
}

void CMainWindow::on_actionSave_triggered()
{
    // Call source manager to save the file
    SourceMgr->SaveFile();
    this->setWindowModified(false);
}

void CMainWindow::on_actionSceneAsObj_triggered()
{
    QMessageBox::information(this, tr("Sorry"), tr("This feature is in the works"));
    QString fileName =
        QFileDialog::getSaveFileName(this, tr("Save Scene as Obj"), "", tr("Obj Files (*.obj);;All Files (*)"));
}

void CMainWindow::on_actionSceneAsStl_triggered()
{
    QString fileName =
        QFileDialog::getSaveFileName(this, tr("Save Scene as Stl"), "", tr("Stl Files (*.stl);;All Files (*)"));
}

void CMainWindow::on_actionMerge_triggered()
{
    // One shot merging, and add a new entity and its corresponding node
    Scene->Update();
    tc::TAutoPtr<Scene::CMeshMerger> merger = new Scene::CMeshMerger("globalMerge");
    Scene->ForEachSceneTreeNode([&](Scene::CSceneTreeNode* node) {
        if (node->GetOwner()->GetName() == "globalMergeNode")
            return;
        auto* entity = node->GetInstanceEntity();
        if (!entity)
        {
            entity = node->GetOwner()->GetEntity();
        }

        if (auto* mesh = dynamic_cast<Scene::CMeshInstance*>(entity))
        {
            merger->MergeIn(*mesh);
        }
    });

    Scene->AddEntity(tc::static_pointer_cast<Scene::CEntity>(merger));
    auto* sn = Scene->GetRootNode()->FindOrCreateChildNode("globalMergeNode");
    sn->SetEntity(merger.Get());
}

void CMainWindow::on_actionPoint_triggered() { }

void CMainWindow::on_actionInstance_triggered() { }

void CMainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, tr("About Nome"),
                       tr("<b>Nome 3.0</b>\n"
                          "Author:\n"
                          "Toby Chen"));
}

void CMainWindow::on_actionAddFace_triggered()
{
    // const auto& verts = Nome3DView->GetSelectedVertices();
    std::vector<std::string> verts;
    if (verts.size() < 3)
    {
        statusBar()->showMessage("Selected vertices are less than 3");
        return;
    }
    TemporaryMeshManager->AddFace(verts);
}

void CMainWindow::on_actionAddPolyline_triggered()
{
    // const auto& verts = Nome3DView->GetSelectedVertices();
    std::vector<std::string> verts;
    if (verts.size() < 2)
    {
        statusBar()->showMessage("Selected vertices are less than 2");
        return;
    }
    TemporaryMeshManager->AddPolyline(verts);
}

void CMainWindow::on_actionResetTempMesh_triggered() { TemporaryMeshManager->ResetTemporaryMesh(); }

void CMainWindow::on_actionCommitTempMesh_triggered()
{
    TemporaryMeshManager->CommitTemporaryMesh(SourceMgr->GetASTContext(), MeshName->text().toStdString(),
                                              InstName->text().toStdString());
    this->setWindowModified(true);
}

void CMainWindow::SetupUI()
{
    // Add vertical layout for main window content
    //  Might not need to do this if layout is in the ui file
    auto* layout = new QVBoxLayout();
    layout->setMargin(0);
    ui->centralwidget->setLayout(layout);

    // Initialize 3D view
    GLWidget = new CCentralGLWidget(ui->centralwidget);
    layout->addWidget(GLWidget);

    // Qt Designer won't let us put text boxes into a toolbar, so we do it here
    InstName = new QLineEdit();
    InstName->setText("newInstance");
    MeshName = new QLineEdit();
    MeshName->setText("newMesh");
    ui->toolBar->insertWidget(ui->actionCommitTempMesh, MeshName);
    ui->toolBar->insertWidget(ui->actionCommitTempMesh, InstName);

    // Connect signals that are not otherwise auto-connected
    connect(ui->actionExit, &QAction::triggered, this, &CMainWindow::close);
    connect(ui->actionAboutQt, &QAction::triggered, this, &QApplication::aboutQt);
}

void CMainWindow::LoadEmptyNomeFile()
{
    // Called from the constructor
    setWindowFilePath("untitled.nom");
    bIsBlankFile = true;
    Scene = new Scene::CScene();
    Scene::GEnv.Scene = Scene.Get();
    PostloadSetup();
}

void CMainWindow::LoadNomeFile(const std::string& filePath)
{
    setWindowFilePath(QString::fromStdString(filePath));
    bIsBlankFile = false;
    SourceMgr = std::make_shared<CSourceManager>(filePath);
    bool parseSuccess = SourceMgr->ParseMainSource();
    if (!parseSuccess)
    {
        auto resp = QMessageBox::question(this, "Parser error",
                                          "The file did not completely successfully parse, do you still want "
                                          "to continue anyway? (See console for more information!)");
        if (resp != QMessageBox::Yes)
        {
            // Does not continue
            LoadEmptyNomeFile();
            return;
        }
    }
    Scene = new Scene::CScene();
    Scene::GEnv.Scene = Scene.Get();
    Scene::CASTSceneAdapter adapter;
    try
    {
        adapter.TraverseFile(SourceMgr->GetAstRoot(), *Scene);
    }
    catch (const AST::CSemanticError& e)
    {
        printf("Error encountered during scene generation:\n%s\n", e.what());
        auto resp = QMessageBox::question(this, "Scene Generation Error",
                                          "See console for details. Do you want to keep what you already have?");
        if (resp != QMessageBox::Yes)
        {
            LoadEmptyNomeFile();
            return;
        }
    }

    PostloadSetup();
}

void CMainWindow::PostloadSetup()
{
    Scene->GetBankAndSet().AddObserver(this);

    SceneUpdateClock = new QTimer(this);
    SceneUpdateClock->setInterval(32);
    SceneUpdateClock->setSingleShot(false);
    connect(SceneUpdateClock, &QTimer::timeout, [this]() {
        Scene->Update();
        GLWidget->update();
    });
    SceneUpdateClock->start();

    TemporaryMeshManager = std::make_unique<Scene::CTemporaryMeshManager>(Scene, SourceMgr);
}

void CMainWindow::UnloadNomeFile()
{
    TemporaryMeshManager.reset(nullptr);
    SceneUpdateClock->stop();
    delete SceneUpdateClock;
    assert(Scene->GetRefCount() == 1);
    Scene = nullptr;
}

void CMainWindow::OnSliderAdded(Scene::CSlider& slider, const std::string& name)
{
    if (!SliderWidget)
    {
        auto* sliderDock = new QDockWidget("Scene Parameter Sliders", this);
        sliderDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
        SliderWidget = std::make_unique<QWidget>();
        SliderLayout = new QFormLayout(SliderWidget.get());
        sliderDock->setWidget(SliderWidget.get());
        this->addDockWidget(Qt::LeftDockWidgetArea, sliderDock);
        ui->menubar->addAction(sliderDock->toggleViewAction());
    }

    auto* sliderName = new QLabel();
    sliderName->setText(QString::fromStdString(name));

    auto* sliderLayout = new QHBoxLayout();

    auto* sliderBar = new QSlider();
    int numSteps = ceil((slider.GetMax() - slider.GetMin()) / slider.GetStep());
    int currTick = round((slider.GetValue() - slider.GetMin()) / slider.GetStep());
    sliderBar->setMinimum(0);
    sliderBar->setMaximum(numSteps);
    sliderBar->setValue(currTick);
    sliderBar->setOrientation(Qt::Horizontal);
    sliderBar->setTickPosition(QSlider::TicksBelow);
    sliderLayout->addWidget(sliderBar);

    auto* sliderDisplay = new QLineEdit();
    sliderDisplay->setText(QString("%1").arg(slider.GetValue()));
    sliderLayout->addWidget(sliderDisplay);

    sliderLayout->setStretchFactor(sliderBar, 4);
    sliderLayout->setStretchFactor(sliderDisplay, 1);

    connect(sliderBar, &QAbstractSlider::valueChanged, [&, sliderDisplay](int value) {
        // Every "1" in value represents a step, since the slider only allows integers
        float fval = (float)value * slider.GetStep() + slider.GetMin();
        auto valueStr = tc::StringPrintf("%.2f", fval);
        sliderDisplay->setText(QString::fromStdString(valueStr));
        slider.SetValue(fval);
        // Update AST for the new value
        auto* argValue = slider.GetASTNode()->GetPositionalArgument(1);
        std::vector<AST::CToken*> tokenList = argValue->ToTokenList();
        size_t insertLocation = SourceMgr->RemoveTokens(tokenList).value();

        auto loc = SourceMgr->InsertText(insertLocation, valueStr);
        auto* expr = new (SourceMgr->GetASTContext())
            AST::ANumber(AST::CToken::Create(SourceMgr->GetASTContext(), loc, valueStr.length()));
        argValue->ClearChildren();
        argValue->AddChild(expr);
    });

    SliderLayout->addRow(sliderName, sliderLayout);
    SliderNameToWidget.emplace(name, sliderLayout);
}

void CMainWindow::OnSliderRemoving(Scene::CSlider& slider, const std::string& name)
{
    auto iter = SliderNameToWidget.find(name);
    assert(iter != SliderNameToWidget.end());

    auto* widget = iter->second;

    SliderNameToWidget.erase(iter);
    SliderLayout->removeRow(widget);
}

}
