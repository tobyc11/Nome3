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
#include <imgui.h>
#include <imgui_impl_dx11.h>

namespace Nome
{

CMainWindow::CMainWindow(QWidget *parent) : QMainWindow(parent),
                                            ui(new Ui::MainWindow),
                                            bIsBlankFile(true)
{
    ui->setupUi(this);
    LoadEmptyNomeFile();
}

CMainWindow::CMainWindow(const std::string& fileToOpen, QWidget* parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      bIsBlankFile(false)
{
    ui->setupUi(this);
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
		QMessageBox message{ QMessageBox::Warning, "Uh oh",
			"This window already contains modified content", QMessageBox::NoButton, this };
		message.show();
		return;
	}

	UnloadNomeFile();
	LoadEmptyNomeFile();
}

void CMainWindow::on_actionOpen_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Nome File"), "",
                                                    tr("Nome Code (*.nom);;All Files (*)"));
    if (fileName.isEmpty())
        return;

    if (bIsBlankFile)
    {
		UnloadNomeFile();
        LoadNomeFile(fileName.toStdString());
    }
    else
    {
        new CMainWindow(fileName.toStdString());
    }
}

void CMainWindow::on_actionPoint_triggered()
{
	Scene::CSceneModifier modifier{ Scene, SourceFile, ASTContext };
	std::string name = QInputDialog::getText(this, "Please input name", "name:").toStdString();
	std::string x = QInputDialog::getText(this, "Please input", "x:").toStdString();
	std::string y = QInputDialog::getText(this, "Please input", "y:").toStdString();
	std::string z = QInputDialog::getText(this, "Please input", "z:").toStdString();
	modifier.AddPoint(name, x, y, z);
}

void CMainWindow::on_actionInstance_triggered()
{
	Scene::CSceneModifier modifier{ Scene, SourceFile, ASTContext };
	std::string name = QInputDialog::getText(this, "Please input name", "name:").toStdString();
	std::string ent = QInputDialog::getText(this, "Please input entity name", "entity:").toStdString();
	modifier.AddInstance(name, ent);
}

void CMainWindow::IdleProcess()
{
	/* Frame order:
		 Handle input events
		 Update scene
		 Render
	*/
	ImGui_ImplDX11_NewFrame();
	ViewWidget->NewFrame();

	{
		ImGui::Begin("Nome Debug");
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}
	Scene->ImGuiUpdate();

	Scene->Update();

	// Rendering
	Scene->Render();
	GRenderer->Render();

	ImGui::Render();
	float color[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	ViewWidget->GetSwapChain()->ClearRenderTarget(color);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	ViewWidget->GetSwapChain()->Present();
}

void CMainWindow::LoadEmptyNomeFile()
{
	//Called from the constructor
    SourceManager = new CSourceManager();
    ASTContext = new CASTContext();

	Scene = new Scene::CScene();

	CodeWindow = new CCodeWindow(this);
	ViewWidget = new CNomeViewWidget(this);
	//Note: since the render logic is here, we also handle ImGui rendering
	ImGui_ImplDX11_Init(GRenderer->GetGD()->GetDevice(), GRenderer->GetGD()->GetImmediateContext());
	setCentralWidget(ViewWidget);

	IdleTimer = new QTimer(this);
	connect(IdleTimer, &QTimer::timeout, this, &CMainWindow::IdleProcess);
	IdleTimer->start();

	bIsBlankFile = true;
}

void CMainWindow::LoadNomeFile(const std::string& filePath)
{
	//Called from the constructor
    SourceManager = new CSourceManager();
	wp<CSourceFile> fileWeak = SourceManager->OpenFile(filePath);
	SourceFile = fileWeak.promote();
	sp<CRope> rope = SourceFile->GetAsRope();
    ASTContext = new CASTContext();

	//Parse the input nome file
	{
		CNomeDriver driver{ ASTContext, rope };
		driver.ParseToAST();
	}
	//TODO: add a check whether parsing was successful, and let the user decide whether to continue anyway
	//Builds a new scene out of the AST
	Scene::CASTSceneBuilder builder{ *ASTContext };
	builder.Traverse();
	Scene = builder.GetScene();

    bIsBlankFile = false;

	CodeWindow = new CCodeWindow(this);
	ViewWidget = new CNomeViewWidget(this);
	//Note: since the render logic is here, we also handle ImGui rendering
	ImGui_ImplDX11_Init(GRenderer->GetGD()->GetDevice(), GRenderer->GetGD()->GetImmediateContext());
	setCentralWidget(ViewWidget);

	IdleTimer = new QTimer(this);
	connect(IdleTimer, &QTimer::timeout, this, &CMainWindow::IdleProcess);
	IdleTimer->start();
}

void CMainWindow::UnloadNomeFile()
{
	IdleTimer->stop();
	delete IdleTimer;

	ImGui_ImplDX11_Shutdown();
	delete ViewWidget;
	delete CodeWindow;

	Scene = nullptr;
	ASTContext = nullptr;
	SourceManager = nullptr;
}

}
