#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "CodeWindow.h"
#include "Nome3DView.h"

#include <Parsing/NomeDriver.h>
#include <Scene/ASTSceneBuilder.h>
#include <Scene/SceneModifier.h>

#include <QFileDialog>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QInputDialog>
#include <QSettings>
#include <QLabel>
#include <QSlider>
#include <QDockWidget>

namespace Nome
{

CMainWindow::CMainWindow(QWidget *parent) : QMainWindow(parent),
                                            ui(new Ui::MainWindow),
                                            bIsBlankFile(true)
{
    ui->setupUi(this);
    SetupUI();
    LoadEmptyNomeFile();
}

CMainWindow::CMainWindow(const std::string& fileToOpen, QWidget* parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      bIsBlankFile(false)
{
    ui->setupUi(this);
    SetupUI();
    LoadNomeFile(fileToOpen);
}

CMainWindow::~CMainWindow()
{
    UnloadNomeFile();
    delete ui;
}

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

    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Nome File"), appSettings.value(kDefaultDir).toString(),
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
        //Open in new window
        //new CMainWindow(fileName.toStdString());
        //Not possible for now since ImGui supports only one context per process
    }
}

void CMainWindow::on_actionReload_triggered()
{
    //You can't reload if the current file is not on disk
    if (SourceFile && !bIsBlankFile)
    {
        std::string path = SourceFile->GetPath();
        UnloadNomeFile();
        LoadNomeFile(path);
    }
}

void CMainWindow::on_actionSave_triggered()
{
    Scene->GetBankAndSet().WriteSliderValues();
    SourceManager->Save(SourceFile);
}

void CMainWindow::on_actionPoint_triggered()
{
    Scene::CSceneModifier modifier{ Scene, SourceManager, SourceFile, ASTContext };
    std::string name = QInputDialog::getText(this, "Please input name", "name:").toStdString();
    std::string x = QInputDialog::getText(this, "Please input", "x:").toStdString();
    std::string y = QInputDialog::getText(this, "Please input", "y:").toStdString();
    std::string z = QInputDialog::getText(this, "Please input", "z:").toStdString();
    modifier.AddPoint(name, x, y, z);
}

void CMainWindow::on_actionInstance_triggered()
{
    Scene::CSceneModifier modifier{ Scene, SourceManager, SourceFile, ASTContext };
    std::string name = QInputDialog::getText(this, "Please input name", "name:").toStdString();
    std::string ent = QInputDialog::getText(this, "Please input entity name", "entity:").toStdString();
    modifier.AddInstance(name, ent);
}

void CMainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, tr("About Nome"),
        tr("<b>Nome 3.0</b>\n"
            "Author:\n"
            "Toby Chen"));
}

void CMainWindow::SetupUI()
{
    //Add vertical layout for main window content
    //  Might not need to do this if layout is in the ui file
    auto* layout = new QVBoxLayout();
    ui->centralwidget->setLayout(layout);

    //Initialize 3D view
    Nome3DView = std::make_unique<CNome3DView>();

    auto* viewContainer = QWidget::createWindowContainer(Nome3DView.get());
    viewContainer->setObjectName("visualLayerContainer");
    QSize screenSize = Nome3DView->screen()->size();
    viewContainer->setMinimumSize(QSize(640, 480));
    viewContainer->setMaximumSize(screenSize);
    viewContainer->setFocusPolicy(Qt::TabFocus);

    layout->addWidget(viewContainer);

    //Connect signals that are not otherwise auto-connected
    connect(ui->actionExit, &QAction::triggered, this, &CMainWindow::close);
    connect(ui->actionAboutQt, &QAction::triggered, this, &QApplication::aboutQt);
}

void CMainWindow::LoadEmptyNomeFile()
{
    //Called from the constructor
    SourceManager = new CSourceManager();
    ASTContext = new CASTContext();

    Scene = new Scene::CScene();
    Scene->GetBankAndSet().AddObserver(this);
    Nome3DView->TakeScene(Scene);

    setWindowFilePath("untitled.nom");

    bIsBlankFile = true;
}

void CMainWindow::LoadNomeFile(const std::string& filePath)
{
    setWindowFilePath(QString::fromStdString(filePath));

    //Called from the constructor
    SourceManager = new CSourceManager();
    SourceFile = SourceManager->Open(filePath);
    ASTContext = new CASTContext();

    //Parse the input nome file
    {
        CNomeDriver driver{ ASTContext, SourceManager, SourceFile };
        driver.ParseToAST();
    }
    //TODO: add a check whether parsing was successful, and let the user decide whether to continue anyway
    //Builds a new scene out of the AST
    Scene::CASTSceneBuilder builder{ *ASTContext, SourceManager, SourceFile };
    try
    {
        builder.Traverse();
    }
    catch (const CSemanticError& exception)
    {
        printf("Semantic error encountered during scene building:\n");
        printf("%s\n", exception.what());

        auto reply = QMessageBox::question(this, "Error", "See console for details. Continue anyway?",
            QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No)
        {
            UnloadNomeFile();
            LoadEmptyNomeFile();
            return;
        }
    }
    catch (const std::runtime_error& exception)
    {
        printf("Exception thrown during scene building (file parsing):\n");
        printf("%s\n", exception.what());

        auto reply = QMessageBox::question(this, "Error", "See console for details. Continue anyway?",
            QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No)
        {
            UnloadNomeFile();
            LoadEmptyNomeFile();
            return;
        }
    }
    Scene = builder.GetScene();
    Scene->GetBankAndSet().AddObserver(this);
    Nome3DView->TakeScene(Scene);

    bIsBlankFile = false;
}

void CMainWindow::UnloadNomeFile()
{
    Nome3DView->UnloadScene();
    Scene = nullptr;
    ASTContext = nullptr;
    if (SourceFile)
        SourceManager->Close(SourceFile);
    SourceFile = nullptr;
    SourceManager = nullptr;
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

    connect(sliderBar, &QAbstractSlider::valueChanged, [sliderDisplay, &slider](int value)
    {
        float fval = (float)value * slider.GetStep() + slider.GetMin();
        QString sval = QString("%1").arg(fval);
        sliderDisplay->setText(sval);
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
