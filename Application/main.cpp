#ifndef DISABLE_MAIN_FOR_TESTS

#include "QtFrontend/MainWindow.h"
#include <QApplication>
#include <QCommandLineParser>


#undef main
int main(int argc, char** argv)
{
    //QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    //QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    // Setup Qt and set up rendering API
    QApplication application(argc, argv);
    QCoreApplication::setApplicationName("Nome");
    QGuiApplication::setApplicationDisplayName("Nome 3.5");
    QCoreApplication::setOrganizationName("UCBerkeley");
    QCoreApplication::setApplicationVersion("3.5");

    QCommandLineParser parser;
    QCommandLineOption detached3dview { QStringList() << "d"
                                                      << "detached3dview",
                                        "Detach the 3D view from the main window" };
    parser.setApplicationDescription(QCoreApplication::applicationName());
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", "The file(s) to open.");
    parser.addOption(detached3dview);
    parser.process(application);

    bool detach3DView = !parser.isSet(detached3dview);

    Nome::CMainWindow* mainWindow = nullptr;
    const QStringList posArgs = parser.positionalArguments();
    for (const QString& file : posArgs)
    {
        Nome::CMainWindow* newWin = new Nome::CMainWindow(file, nullptr, detach3DView);
        newWin->show();
        mainWindow = newWin;
    }

    if (!mainWindow)
        mainWindow = new Nome::CMainWindow(nullptr, detach3DView);
    mainWindow->show();

    return application.exec();
}

#endif
