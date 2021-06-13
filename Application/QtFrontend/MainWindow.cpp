#include "MainWindow.h"
#include "CodeWindow.h"
#include "FrontendContext.h"
#include "Nome3DView.h"
#include "ui_MainWindow.h"
#include <Scene/ASTSceneAdapter.h>
#include <Scene/Environment.h>


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

#include <QTableWidget> // Steven's Add Point
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
    tc::TAutoPtr<Scene::CMeshMerger> merger = new Scene::CMeshMerger(
        "globalMerge"); // CmeshMerger is basically a CMesh, but with a MergeIn method. Merger will
                        // contain ALL the merged vertices (from various meshes)

    Scene->ForEachSceneTreeNode([&](Scene::CSceneTreeNode* node) {
        if (node->GetOwner()->GetName() == "globalMergeNode") // If the node owner is a globalMergeNode, skip as that was a
                                  // previously merger mesh (from a previous Merge iteration). We only
                                  // want to merge vertices from our actual (non-merged) meshes.
            return;
        auto* entity = node->GetInstanceEntity(); // Else, get the instance
        if (!entity) // Check to see if the an entity is instantiable
        {
            entity = node->GetOwner()->GetEntity(); // If it's not instantiable, get entity instead
        } 
        if (auto* mesh = dynamic_cast<Scene::CMeshInstance*>(entity))
        { // set "auto * mesh" to this entity. Call MergeIn to set merger's vertices based on mesh's
          // vertices. Reminder: an instance identifier is NOT a Mesh, so only real entities get
          // merged.
            merger->MergeIn(*mesh);
            entity->isMerged = true;
        }
    });

    Scene->Update();
    // TODO: 10/22 added.  These lines work to reset the scene
    Scene->ForEachSceneTreeNode([&](Scene::CSceneTreeNode* node) {
        if (node->GetOwner()->GetName() != "globalMergeNode")
            node->GetOwner()->SetEntity(nullptr);
    });


    Scene->AddEntity(tc::static_pointer_cast<Scene::CEntity>(
        merger)); // Merger now has all the vertices set, so we can add it into the scene as a new
                  // entity
    auto* sn = Scene->GetRootNode()->FindOrCreateChildNode("globalMergeNode"); // Add it into the Scene Tree by creating a new node called
                            // globalMergeNode. Notice, this is the same name everytime you Merge.
                            // This means you can only have one merger mesh each time. It will
                            // override previous merger meshes with the new vertices.
    sn->SetEntity(merger.Get()); // Set sn, which is the scene node, to point to entity merger
}

// only subdivide merge nodes
void CMainWindow::on_actionSubdivide_triggered()
{
    // One shot merging, and add a new entity and its corresponding node
    Scene->Update();
    Scene->ForEachSceneTreeNode([&](Scene::CSceneTreeNode* node) {
        if (node->GetOwner()->GetName() == "globalMergeNode")
        {

            auto* entity = node->GetOwner()->GetEntity();
            if (auto* mesh = dynamic_cast<Scene::CMeshMerger*>(entity))
            {
                bool ok;
                int sub_level = QInputDialog::getInt(this, tr("Please enter the level of subdivision"),
                                                     tr("Subdivision Level:"), 3, 0, 10, 1, &ok);
                if (ok && sub_level > 0 && sub_level < 10)
                    mesh->setSubLevel(sub_level);
                else
                    mesh->setSubLevel(3);
                mesh->Catmull(); // TODO: pass in level argument
                mesh->MarkDirty();

            }
        }
    });

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

// Steven's Add Point
void CMainWindow::on_actionAddPoint_triggered()
{
    auto* dialog = new QDialog(GFrtCtx->MainWindow);
    dialog->setModal(true);
    auto* layout1 = new QHBoxLayout(dialog);
    auto* table = new QTableWidget();
    const char* xyz[3] = { "X: ", "Y: ", "Z: " };
    std::vector<QLineEdit*> getLines;
    for (auto* s : xyz)
    {
        QLabel* tempLabel = new QLabel(tr(s));
        QLineEdit* tempInput = new QLineEdit("");
        layout1->addWidget(tempLabel);
        layout1->addWidget(tempInput);
        getLines.push_back(tempInput);
    }
    auto* layout2 = new QVBoxLayout();
    auto* btnOk = new QPushButton();
    btnOk->setText("OK");
    connect(btnOk, &QPushButton::clicked, [this, dialog, getLines]() {
        std::vector<std::string> pos;
        for (auto* getline : getLines)
        {
            pos.push_back(getline->text().toStdString().c_str());
        }
        TemporaryMeshManager->AddPoint(pos);
        dialog->close();
    });
    auto* btnCancel = new QPushButton();
    btnCancel->setText("Cancel");
    connect(btnCancel, &QPushButton::clicked, dialog, &QWidget::close);
    layout2->addWidget(btnOk);
    layout2->addWidget(btnCancel);
    layout1->addLayout(layout2);
    dialog->show();
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

    const auto& faces = Nome3DView->GetSelectedFaces();
    TemporaryMeshManager->RemoveFace(faces);
    Nome3DView->ClearSelectedFaces(); // Randy added 9/27
}

// Randy temporarily commenting out because Reloading serves the same purpose as this.
//void CMainWindow::on_actionResetTempMesh_triggered() { TemporaryMeshManager->ResetTemporaryMesh(); }


// Randy added on 2/26 for adding vertices via a ray    
void CMainWindow::displayRay()
{
    std::vector<std::string> allPointNames;
    const auto& vertPositions =Nome3DView->GetRayVertPositions(); //  vector containing vector of string positions
    auto origin = vertPositions[0];
    auto hitPoint = vertPositions[1];
    std::vector<std::string> originString = { std::to_string(origin.x), std::to_string(origin.y), std::to_string(origin.z) };

    std::vector<std::string> intersectionString = { std::to_string(hitPoint.x), std::to_string(hitPoint.y), std::to_string(hitPoint.z) };

    std::string originName = TemporaryMeshManager->AddPoint(originString);
    std::string intersectionName = TemporaryMeshManager->AddPoint(intersectionString);

    allPointNames.push_back(originName);

    std::vector<tc::Vector3> pointsInBtwn;
    int numPoints = 10;
    for (float i = 0.1; i < 1; i+=1.0f/numPoints) {
        tc::Vector3 vecInBtwn = {origin.x +i*(hitPoint.x-origin.x), origin.y +i*(hitPoint.y-origin.y), origin.z +i*(hitPoint.z-origin.z)}; //https://math.stackexchange.com/questions/428766/how-do-i-find-out-the-coordinates-of-every-point-between-two-points
        std::vector<std::string> vecInBtwnString { std::to_string(vecInBtwn.x), std::to_string(vecInBtwn.y), std::to_string(vecInBtwn.z)};
        //std::string vecInBtwnName = TemporaryMeshManager->AddPoint(vecInBtwnString);
        //allPointNames.push_back(vecInBtwnName);
    }

    allPointNames.push_back(intersectionName);

    TemporaryMeshManager->AddInteractivePolyline(allPointNames); // add selectable polyline (a polyline used to interactively add points)
   
    //Nome3DView->PickVertexBool = !Nome3DView->PickVertexBool; // fix this later
    Nome3DView->ClearSelectedEdges(); ////TODO: CHANGE THSI.  Commented out. Only clear points after point has been selected
}

// used for adding a point into the scene
void CMainWindow::displayPoint()
{
    std::vector<std::string> allPointNames;
    const auto& vertPosition =
        Nome3DView->GetInteractivePoint(); //  vector containing vector of string positions
    auto point = vertPosition[0];

    std::vector<std::string> pointString = { std::to_string(point.x), std::to_string(point.y),
                                              std::to_string(point.z) };


    std::string pointName = TemporaryMeshManager->AddPoint(pointString);
    std::cout << "DONE DISPLAYING POINT" << std::endl;
    Nome3DView->PickVertexBool = !Nome3DView->PickVertexBool; // TODO: weird logic to show point. This is NOT GOOD. because messes up vertex selection button
    Nome3DView->ClearInteractivePoint(); // No longer need added point
    Nome3DView->ClearRenderedRay();  // No longer need origin and direction points
}



// Toggle on/off Ray Rendering
void CMainWindow::on_actionToggleRenderRay_triggered()
{
    Nome3DView->RenderRayBool = !Nome3DView->RenderRayBool;
    // mark all mesh instances dirty. Added on 11/26
    Scene->ForEachSceneTreeNode([&](Scene::CSceneTreeNode* node) {
        auto* entity = node->GetInstanceEntity();
        if (!entity)
            entity = node->GetOwner()->GetEntity();

        if (auto* mesh = dynamic_cast<Scene::CMeshInstance*>(entity))
            mesh->MarkDirty();
    });
}

void CMainWindow::on_actionCommitChanges_triggered()
{
    TemporaryMeshManager->CommitChanges(SourceMgr->GetASTContext());
    // InstName and MeshName are not used anymore
    // MeshName->text().toStdString(),
    // InstName->text().toStdString());
    this->setWindowModified(true);
}

// Toggle on/off Face facets/edges coloring
void CMainWindow::on_actionShowFacets_triggered()
{
    Nome3DView->WireFrameMode = !(Nome3DView->WireFrameMode);

    // mark all mesh instances dirty
    Scene->ForEachSceneTreeNode([&](Scene::CSceneTreeNode* node) {
        auto* entity = node->GetInstanceEntity();
        if (!entity)
            entity = node->GetOwner()->GetEntity();

        if (auto* mesh = dynamic_cast<Scene::CMeshInstance*>(entity))
            mesh->MarkDirty();
    });
}



// Toggle on/off Vertex selection
void CMainWindow::on_actionToggleVertexSelection_triggered()
{
    Nome3DView->PickVertexBool = !Nome3DView->PickVertexBool;
    // mark all mesh instances dirty. Added on 11/26
    Scene->ForEachSceneTreeNode([&](Scene::CSceneTreeNode* node) {
        auto* entity = node->GetInstanceEntity();
        if (!entity)
            entity = node->GetOwner()->GetEntity();

        if (auto* mesh = dynamic_cast<Scene::CMeshInstance*>(entity))
            mesh->MarkDirty();
    });
    std::cout << "at end of toggle vert" << std::endl;
}


// Toggle on/off Edge Selection
void CMainWindow::on_actionToggleEdgeSelection_triggered()
{
    Nome3DView->PickEdgeBool = !Nome3DView->PickEdgeBool;
}

// Toggle on/off Polyline Selection
void CMainWindow::on_actionTogglePolylineSelection_triggered()
{
    Nome3DView->PickPolylineBool = !Nome3DView->PickPolylineBool;
}

// Toggle on/off Face Selection
void CMainWindow::on_actionToggleFaceSelection_triggered()
{
    Nome3DView->PickFaceBool = !Nome3DView->PickFaceBool;
    // mark all mesh instances dirty. Added on 11/26
    Scene->ForEachSceneTreeNode([&](Scene::CSceneTreeNode* node) {
        auto* entity = node->GetInstanceEntity();
        if (!entity)
            entity = node->GetOwner()->GetEntity();

        if (auto* mesh = dynamic_cast<Scene::CMeshInstance*>(entity))
            mesh->MarkDirty();
    });
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
//        QPushButton* start = new QPushButton("Toggle", this);
//        start->setText("Start/Stop");
//        layout->addWidget(start);
//        connect(start, &QPushButton::clicked, [&]() {
//            if (!timer->isActive()) {
//                timer->start();
//            } else {
//                timer->stop();
//            }
//        });
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

    bool parseSuccess = SourceMgr->ParseMainSource(); // AST is created with this function call. If
                                                      // want to add #include, must combine files
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
        auto includeFileNames = adapter.GetIncludes(SourceMgr->GetASTContext().GetAstRoot(), *Scene); // randy added includeFileNames variable on 11/30. Currently assumes included
                     // file names are in same directory as original

        // TODO: In the future, allow included files to be in different directories
        for (auto fileName : includeFileNames)
        {
            auto nofileNamepath = filePath.substr(0, filePath.find_last_of("/") + 1);
            auto testSourceMgr = std::make_shared<CSourceManager>(
                nofileNamepath + fileName); 
            bool testparseSuccess = testSourceMgr->ParseMainSource();
            adapter.TraverseFile(testSourceMgr->GetASTContext().GetAstRoot(), *Scene);
        }



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

    elapsedRender = new QElapsedTimer();
    SceneUpdateClock = new QTimer(this);
    SceneUpdateClock->setInterval(50);
    SceneUpdateClock->setSingleShot(false);
    elapsedRender->start();
    connect(SceneUpdateClock, &QTimer::timeout, [this]() {
        Scene->Update();
        Nome3DView->PostSceneUpdate();
        // Randy added this on 11/5 for edge selection
        if (!Nome3DView->GetSelectedEdgeVertices().empty())
        {
            TemporaryMeshManager->SelectOrDeselectPolyline(Nome3DView->GetSelectedEdgeVertices());
            Nome3DView->ClearSelectedEdges(); // TODO: This is assuming can only add one edge a time
        }
        if (Nome3DView->RayCasted) {
            displayRay(); // Display the ray in GUI
            Nome3DView->RayCasted = false;
        }
        if (Nome3DView->GetInteractivePoint().size() != 0)
        {
            displayPoint(); // and hides ray
        }
        Scene->SetTime((float) elapsedRender->elapsed() / 1000);
        Scene->SetFrame(1);

    });
    SceneUpdateClock->start();

    TemporaryMeshManager = std::make_unique<Scene::CTemporaryMeshManager>(Scene, SourceMgr);
}

void CMainWindow::UnloadNomeFile()
{
    TemporaryMeshManager.reset(nullptr);
    SceneUpdateClock->stop();
    delete SceneUpdateClock;
    elapsedRender->invalidate();
    delete elapsedRender;
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
    else
    {
        auto bankname = name.substr(0, name.find_last_of(".") + 1);
        // Check if bank has already been added
        auto alreadyAdded = false;
        for (auto& Pair : SliderNameToWidget)
        {
            if (Pair.first.substr(0, Pair.first.find_last_of(".") + 1) == bankname)
            {
                alreadyAdded = true;
            }
        }
        // If it hasn't been added, add a blank row
        if (!alreadyAdded)
        {
            auto* sliderName = new QLabel();
            sliderName->setText(QString::fromStdString(""));
            QFont f("Arial", 13);
            sliderName->setFont(f);
            auto* sliderLayout = new QHBoxLayout();
            SliderLayout->addRow(sliderName, sliderLayout);
            SliderNameToWidget.emplace(name, sliderLayout);
        }
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
    std::string sliderID = slider.GetASTNode()->GetPositionalIdentAsString(0);

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

bool CMainWindow::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::KeyRelease) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if (keyEvent->key() == Qt::Key_Shift) {
            //Nome3DView->FreeVertexSelection();
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
