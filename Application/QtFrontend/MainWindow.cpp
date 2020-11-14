#include "MainWindow.h"
#include "CodeWindow.h"
#include "FrontendContext.h"
#include "Nome3DView.h"
#include "ui_MainWindow.h"
#include <Scene/ASTSceneAdapter.h>
#include <Scene/Environment.h>
#include <Scene/MeshMerger.h>

#include <QDockWidget>
#include <QScrollArea> // Randy added
#include <QFileDialog>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QSettings>
#include <QSlider>
#include <QPushButton>
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
    this->installEventFilter(Nome3DView.get());
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

/* 10/1 Commenting out to avoid annoying error message on open
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
}*/

void CMainWindow::on_actionMerge_triggered()
{
    // One shot merging, and add a new entity and its corresponding node
    Scene->Update();
    tc::TAutoPtr<Scene::CMeshMerger> merger = new Scene::CMeshMerger("globalMerge"); //CmeshMerger is basically a CMesh, but with a MergeIn method. Merger will contain ALL the merged vertices (from various meshes)
    Scene->ForEachSceneTreeNode([&](Scene::CSceneTreeNode* node) {
        if (node->GetOwner()->GetName() == "globalMergeNode") // If the node owner is a globalMergeNode, skip as that was a previously merger mesh (from a previous Merge process). We only want to merge vertices from our actual (non-merged) meshes.
            return;
        auto* entity = node->GetInstanceEntity(); // Else, get the instance
        if (!entity) // Check to see if the an entity is instantiable (e.g., polyline, funnel, mesh, etc.), and not just an instance identifier.
            entity = node->GetOwner()->GetEntity(); // If it's not instantiable, get entity instead of instance entity

        if (auto* mesh = dynamic_cast<Scene::CMeshInstance*>(entity))  //set "auto * mesh" to this entity. Call MergeIn to set merger's vertices based on mesh's vertices. Reminder: an instance identifier is NOT a Mesh, so only real entities get merged.
            merger->MergeIn(*mesh);
    });
    // TODO: Next 3 lines are super buggy, but needed to perform Catmull w/ replacement. Often crashes when used on larger scenes.
    //Scene = new Scene::CScene();
    //Scene::GEnv.Scene = Scene.Get();
    //PostloadSetup();

    Scene->AddEntity(tc::static_pointer_cast<Scene::CEntity>(merger)); // Merger now has all the vertices set, so we can add it into the scene as a new entity
    auto* sn = Scene->GetRootNode()->FindOrCreateChildNode("globalMergeNode"); //Add it into the Scene Tree by creating a new node called globalMergeNode. Notice, this is the same name everytime you Merge. This means you can only have one merger mesh each time. It will override previous merger meshes with the new vertices. 
    sn->SetEntity(merger.Get()); // Set sn, which is the scene node, to point to entity merger 

}

// only subdivide merge nodes
void CMainWindow::on_actionSubdivide_triggered()
{
    // One shot merging, and add a new entity and its corresponding node
    Scene->Update();
    tc::TAutoPtr<Scene::CMeshMerger> merger = new Scene::CMeshMerger("globalMerge"); 
    Scene->ForEachSceneTreeNode([&](Scene::CSceneTreeNode* node) {
        if (node->GetOwner()->GetName() == "globalMergeNode")
        {
            auto* entity = node->GetInstanceEntity(); // this is non-null if the entity is
                                                      // instantiable like a torus knot or polyline
            if (!entity) // if it's not instantiable, like a face, then get the entity associated
                         // with it
                entity = node->GetOwner()->GetEntity();
            if (auto* mesh = dynamic_cast<Scene::CMeshInstance*>(entity))
            {
                merger->Catmull(*mesh);
            }
        }
        
    });
    Scene->AddEntity(tc::static_pointer_cast<Scene::CEntity>(
        merger)); 
    auto* sn = Scene->GetRootNode()->FindOrCreateChildNode("globalMergeNode"); 
    sn->SetEntity(merger.Get());  
    
}
/* Randy temporarily commenting out. Point and Instance don't work.
void CMainWindow::on_actionPoint_triggered() { }

void CMainWindow::on_actionInstance_triggered() { }

void CMainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, tr("About Nome"),
                       tr("<b>Nome 3.5</b>\n"
                          "Author:\n"
                          "insert author name for current version"));
}*/

void CMainWindow::on_actionAddFace_triggered()
{
    const auto& verts = Nome3DView->GetSelectedVertices();
    if (verts.size() < 3)
    {
        statusBar()->showMessage("Selected vertices are less than 3");
        return;
    }
    TemporaryMeshManager->AddFace(verts);
    Nome3DView->ClearSelectedVertices(); // Randy added 9/27
}

void CMainWindow::on_actionAddPolyline_triggered()
{
    const auto& verts = Nome3DView->GetSelectedVertices();
    if (verts.size() < 2)
    {
        statusBar()->showMessage("Selected vertices are less than 2");
        return;
    }
    TemporaryMeshManager->AddPolyline(verts);
    Nome3DView->ClearSelectedVertices(); // Randy added 9/27
}

void CMainWindow::on_actionRemoveFace_triggered()
{
    const auto& verts = Nome3DView->GetSelectedVertices();
    if (verts.size() < 3)
    {
        statusBar()->showMessage("Selected vertices are less than 3");
        return;
    }
    TemporaryMeshManager->AddFace(verts);
    Nome3DView->ClearSelectedVertices(); // Randy added 9/27
}

// Randy temporarily commenting out because Reloading serves the same purpose as this.
//void CMainWindow::on_actionResetTempMesh_triggered() { TemporaryMeshManager->ResetTemporaryMesh(); }


void CMainWindow::on_actionCommitChanges_triggered()
{
    TemporaryMeshManager->CommitChanges(
        SourceMgr
            ->GetASTContext()); // 10/1 Randy commented the following out because MeshName and
                                // InstName are not used anymore MeshName->text().toStdString(),
                                // InstName->text().toStdString());
    this->setWindowModified(true);
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

    if (true)// (!bDetached3DView) Temporarily changing this to True to avoid overlapping windows
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

    /* 10/1 Temporarily commenting out due to tempmeshmanager changes
    // Qt Designer won't let us put text boxes into a toolbar, so we do it here
    InstName = new QLineEdit();
    InstName->setText("newInstance");
    MeshName = new QLineEdit();
    MeshName->setText("newMesh");
    ui->toolBar->insertWidget(ui->actionCommitChanges, MeshName);
    ui->toolBar->insertWidget(ui->actionCommitChanges, InstName);*/

    // Connect signals that are not otherwise auto-connected
    connect(ui->actionExit, &QAction::triggered, this, &CMainWindow::close);
    //connect(ui->actionAboutQt, &QAction::triggered, this, &QApplication::aboutQt);
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
        auto resp = QMessageBox::question(
            this, "Parser error",
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
        adapter.TraverseFile(SourceMgr->GetASTContext().GetAstRoot(), *Scene);
    }
    catch (const AST::CSemanticError& e)
    {
        printf("Error encountered during scene generation:\n%s\n", e.what());
        auto resp = QMessageBox::question(
            this, "Scene Generation Error",
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
    Nome3DView->TakeScene(Scene);

    SceneUpdateClock = new QTimer(this);
    SceneUpdateClock->setInterval(50);
    SceneUpdateClock->setSingleShot(false);
    connect(SceneUpdateClock, &QTimer::timeout, [this]() {
        Scene->Update();
        Nome3DView->PostSceneUpdate();
    });
    SceneUpdateClock->start();

    TemporaryMeshManager = std::make_unique<Scene::CTemporaryMeshManager>(Scene, SourceMgr);
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

void CMainWindow::OnSliderAdded(Scene::CSlider& slider, const std::string& name) // adding a single widget at a time
{
    if (!SliderWidget)
    {
        auto* sliderDock = new QDockWidget("Scene Parameter Sliders", this);

        sliderDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
        SliderWidget = std::make_unique<QWidget>();
        SliderLayout = new QFormLayout(SliderWidget.get());

        this->addDockWidget(Qt::LeftDockWidgetArea, sliderDock);
        ui->menubar->addAction(sliderDock->toggleViewAction());


        // Create scroll area for the widget
        QScrollArea* m_pMapInfoScrollArea = new QScrollArea();
        m_pMapInfoScrollArea->setObjectName(QStringLiteral("MapInfoScrollArea"));
        m_pMapInfoScrollArea->setWidgetResizable(true);
        m_pMapInfoScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        m_pMapInfoScrollArea->setFrameStyle(QFrame::NoFrame);
 
        m_pMapInfoScrollArea->setWidget(SliderWidget.get()); // SliderWidget.get()
        sliderDock->setWidget(m_pMapInfoScrollArea );
        SliderWidget.get()->setMinimumSize(280, 1200); //https://www.qtcentre.org/threads/55669-Scroll-Area-inside-Dock-Widget
    }

    auto* sliderName = new QLabel();
    sliderName->setText(QString::fromStdString(name));
    QFont f("Arial", 13);
    sliderName->setFont(f);
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
    sliderDisplay->setFont(f);
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
    std::string sliderID = slider.GetASTNode()->GetPositionalIdentAsString(0);
    if (hasEnding(sliderID, "time")) {
        timer = new QTimer(this);
        SliderTimers.emplace(sliderID, timer);
        QPushButton *start = new QPushButton("Toggle", this);
        start->setText("Start/Stop");
        sliderLayout->addWidget(start);
        connect(start, &QPushButton::clicked, this, [this, &slider]() {
            QTimer* currtimer = SliderTimers.find(slider.GetASTNode()->
                GetPositionalIdentAsString(0))->second;
            timer = currtimer;
            if (!timer->isActive()) {
                timer->start(50);
            } else {
                timer->stop();
            }
        });
        connect(timer, &QTimer::timeout, this, [&slider, sliderDisplay]() {
            float val = slider.GetValue() + slider.GetStep();
            if (val <= slider.GetMax()) {
                slider.SetValue(val);
            } else {
                slider.SetValue(slider.GetMin());
            }
            sliderDisplay->setText(QString::fromStdString(tc::StringPrintf("%.2f", val)));
        });
        timer->start(50);

    }


    SliderLayout->addRow(sliderName, sliderLayout);
    SliderNameToWidget.emplace(name, sliderLayout);
}

bool CMainWindow::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::KeyRelease) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if (keyEvent->key() == Qt::Key_Shift) {
            Nome3DView->FreeVertexSelection();
            return true;
        }
        else
            return false;
    }
    return false;
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
