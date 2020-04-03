#include "MainWindow.h"
#include "CodeWindow.h"
#include "FrontendContext.h"
#include "Nome3DView.h"
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

static CFrontendContext AnonFrontendContext;
CFrontendContext* GFrtCtx = &AnonFrontendContext;

CMainWindow::CMainWindow(QWidget* parent, bool bDetached3d)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , bIsBlankFile(true)
    , bDetached3DView(bDetached3d)
{
    ui->setupUi(this);
    SetupUI();
    LoadEmptyNomeFile();
}

CMainWindow::CMainWindow(const QString& fileToOpen, QWidget* parent, bool bDetached3d)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , bIsBlankFile(false)
    , bDetached3DView(bDetached3d)
{
    ui->setupUi(this);
    SetupUI();
    LoadNomeFile(fileToOpen.toStdString());
}

CMainWindow::~CMainWindow()
{
    GFrtCtx->MainWindow = nullptr;
    UnloadNomeFile();
    delete ui;
}

void CMainWindow::closeEvent(QCloseEvent* event)
{
    if (bDetached3DView)
    {
        Nome3DView->close();
    }
    QWidget::closeEvent(event);
}

void CMainWindow::on_actionNew_triggered()
{
    if (!bIsBlankFile)
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "New File",
                                      "Your existing work will be discarded, continue?",
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

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Nome File"),
                                                    appSettings.value(kDefaultDir).toString(),
                                                    tr("Nome Code (*.nom);;All Files (*)"));
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
    // Saves a list of all faces on the side
    for (auto* node : Scene->GetSelectedNodes())
    {
        if (auto* ent = dynamic_cast<Scene::CMeshInstance*>(node->GetEntity()))
        {
            ent->GetMeshImpl();
        }
    }
    // Call source manager to save the file
    SourceMgr->SaveFile();
    this->setWindowModified(false);
}

void CMainWindow::on_actionSceneAsObj_triggered()
{
    QMessageBox::information(this, tr("Sorry"), tr("This feature is in the works"));
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Scene as Obj"), "",
                                                    tr("Obj Files (*.obj);;All Files (*)"));
}

void CMainWindow::on_actionSceneAsStl_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Scene as Stl"), "",
                                                    tr("Stl Files (*.stl);;All Files (*)"));
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

void CMainWindow::on_actionPoint_triggered() {}

void CMainWindow::on_actionInstance_triggered() {}

void CMainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, tr("About Nome"),
                       tr("<b>Nome 3.0</b>\n"
                          "Author:\n"
                          "Toby Chen"));
}

void CMainWindow::on_actionAddFace_triggered()
{
    const auto& verts = Nome3DView->GetSelectedVertices();
    if (verts.size() < 3)
    {
        statusBar()->showMessage("Selected vertices are less than 3");
        return;
    }
    TemporaryMeshManager->AddFace(verts);
}

void CMainWindow::on_actionResetTempMesh_triggered() { TemporaryMeshManager->ResetTemporaryMesh(); }

void CMainWindow::on_actionCommitTempMesh_triggered()
{
    std::string code = TemporaryMeshManager->CommitTemporaryMesh(
        SourceMgr->GetASTContext(), MeshName->text().toStdString(), InstName->text().toStdString());
    SourceMgr->CommitASTChanges();
    this->setWindowModified(true);
    QInputDialog::getMultiLineText(this, tr("Code"), tr("Please manually copy the code for now:"),
                                   QString::fromStdString(code));
}

void CMainWindow::SetupUI()
{
    GFrtCtx->MainWindow = this;

    // Add vertical layout for main window content
    //  Might not need to do this if layout is in the ui file
    auto* layout = new QVBoxLayout();
    ui->centralwidget->setLayout(layout);

    // Initialize 3D view
    Nome3DView = std::make_unique<CNome3DView>();
    GFrtCtx->NomeView = Nome3DView.get();

    if (!bDetached3DView)
    {
        auto* viewContainer = QWidget::createWindowContainer(Nome3DView.get());
        viewContainer->setObjectName("visualLayerContainer");
        QSize screenSize = Nome3DView->screen()->size();
        viewContainer->setMinimumSize(QSize(640, 480));
        viewContainer->setMaximumSize(screenSize);
        viewContainer->setFocusPolicy(Qt::TabFocus);
        layout->addWidget(viewContainer);
    }
    else
    {
        Nome3DView->show();
    }

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

void CMainWindow::PreloadSetup() {}

void CMainWindow::LoadEmptyNomeFile()
{
    // Called from the constructor
    PreloadSetup();
    setWindowFilePath("untitled.nom");
    bIsBlankFile = true;
    Scene = new Scene::CScene();
    Scene::GEnv.Scene = Scene.Get();
    PostloadSetup();
}

void CMainWindow::LoadNomeFile(const std::string& filePath)
{
    PreloadSetup();

    setWindowFilePath(QString::fromStdString(filePath));
    bIsBlankFile = false;
    SourceMgr = std::make_unique<CSourceManager>(filePath);
    bool parseSuccess = SourceMgr->ParseMainSource();
    if (!parseSuccess)
    {
        auto resp = QMessageBox::question(
            this, "Parser error",
            "The file did not completely successfully parse, do you still want "
            "to continue anyway? (See console for more information!)");
        if (!resp)
        {
            // Does not continue
            UnloadNomeFile();
            LoadEmptyNomeFile();
            return;
        }
    }
    Scene = new Scene::CScene();
    Scene::GEnv.Scene = Scene.Get();
    Scene::CASTSceneAdapter adapter;
    adapter.TraverseFile(SourceMgr->GetASTContext().GetAstRoot(), *Scene);

    PostloadSetup();
}

void CMainWindow::PostloadSetup()
{
    Scene->GetBankAndSet().AddObserver(this);
    Nome3DView->TakeScene(Scene);

    SceneUpdateClock = new QTimer(this);
    SceneUpdateClock->setInterval(100);
    SceneUpdateClock->setSingleShot(false);
    connect(SceneUpdateClock, &QTimer::timeout, [this]() {
        Scene->Update();
        Nome3DView->PostSceneUpdate();
    });
    SceneUpdateClock->start();

    TemporaryMeshManager = std::make_unique<Scene::CTemporaryMeshManager>(Scene);
}

void CMainWindow::UnloadNomeFile()
{
    TemporaryMeshManager.reset(nullptr);
    SceneUpdateClock->stop();
    delete SceneUpdateClock;
    Nome3DView->UnloadScene();
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
        auto* argExpr = slider.GetASTNode()->GetPositionalArgument(1);
        std::vector<AST::CToken*> tokenList;
        argExpr->CollectTokens(tokenList);
        size_t insertLocation = SourceMgr->RemoveTokens(tokenList).value();

        auto* token = SourceMgr->GetASTContext().MakeToken(valueStr);
        auto* expr = SourceMgr->GetASTContext().Make<AST::ANumber>(token);
        slider.GetASTNode()->SetPositionalArgument(1, expr);
        SourceMgr->InsertToken(insertLocation, token);
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
