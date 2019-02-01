#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "CodeWindow.h"

#include <Parsing/NomeDriver.h>
#include <Scene/ASTSceneBuilder.h>
#include <Scene/SceneModifier.h>
#include <Render/GraphicsDevice.h>

#include <QFileDialog>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QInputDialog>

namespace Nome
{

CMainWindow::CMainWindow(QWidget *parent) : QMainWindow(parent),
                                            ui(new Ui::MainWindow),
                                            bIsBlankFile(true)
{
    ui->setupUi(this);
	connect(ui->actionExit, &QAction::triggered, this, &CMainWindow::close);
    connect(ui->actionAboutQt, &QAction::triggered, this, &QApplication::aboutQt);
    LoadEmptyNomeFile();
}

CMainWindow::CMainWindow(const std::string& fileToOpen, QWidget* parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      bIsBlankFile(false)
{
    ui->setupUi(this);
	connect(ui->actionExit, &QAction::triggered, this, &CMainWindow::close);
    connect(ui->actionAboutQt, &QAction::triggered, this, &QApplication::aboutQt);
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
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Nome File"), "",
                                                    tr("Nome Code (*.nom);;All Files (*)"));
    if (fileName.isEmpty())
        return;

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
	Scene::CSceneModifier modifier{ Scene, SourceManager,SourceFile, ASTContext };
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

void CMainWindow::IdleProcess()
{
	ViewWidget->UpdateAndDraw();
}

void CMainWindow::LoadEmptyNomeFile()
{
	//Called from the constructor
    SourceManager = new CSourceManager();
    ASTContext = new CASTContext();

	Scene = new Scene::CScene();

	ViewportClient = new CEditorViewportClient(Scene);
	CodeWindow = new CCodeWindow(this);
	ViewWidget = new CNomeViewWidget(this, ViewportClient);
	setCentralWidget(ViewWidget);

	setWindowFilePath("untitled.nom");

	IdleTimer = new QTimer(this);
	//Limit to 200 fps
	IdleTimer->setTimerType(Qt::PreciseTimer);
	IdleTimer->setInterval(5);
	connect(IdleTimer, &QTimer::timeout, this, &CMainWindow::IdleProcess);
	IdleTimer->start();

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

    bIsBlankFile = false;

	ViewportClient = new CEditorViewportClient(Scene);
	CodeWindow = new CCodeWindow(this);
	ViewWidget = new CNomeViewWidget(this, ViewportClient);
	setCentralWidget(ViewWidget);

	IdleTimer = new QTimer(this);
	//Limit to 200 fps
	IdleTimer->setTimerType(Qt::PreciseTimer);
	IdleTimer->setInterval(5);
	connect(IdleTimer, &QTimer::timeout, this, &CMainWindow::IdleProcess);
	IdleTimer->start();
}

void CMainWindow::UnloadNomeFile()
{
	if (IdleTimer)
	{
		IdleTimer->stop();
		delete IdleTimer; IdleTimer = nullptr;
	}

	delete ViewWidget; ViewWidget = nullptr;
	delete CodeWindow; CodeWindow = nullptr;
	delete ViewportClient; ViewportClient = nullptr;

	Scene = nullptr;
	ASTContext = nullptr;
	if (SourceFile)
		SourceManager->Close(SourceFile);
	SourceFile = nullptr;
	SourceManager = nullptr;
}

}
