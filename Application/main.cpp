#ifndef DISABLE_MAIN_FOR_TESTS

#include "QtFrontend/MainWindow.h"
#include <QApplication>
#include <QCommandLineParser>

#undef main
int main(int argc, char** argv)
{
    // Setup Qt
    QApplication application(argc, argv);
    QCoreApplication::setApplicationName("Nome");
    QGuiApplication::setApplicationDisplayName("Nome 3.0");
    QCoreApplication::setOrganizationName("UCBerkeley");
    QCoreApplication::setApplicationVersion("3.0");

    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::applicationName());
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", "The file(s) to open.");
    parser.process(application);

    Nome::CMainWindow* mainWindow = nullptr;
    const QStringList posArgs = parser.positionalArguments();
    for (const QString& file : posArgs)
    {
        Nome::CMainWindow* newWin = new Nome::CMainWindow(file);
        newWin->show();
        mainWindow = newWin;
    }

    if (!mainWindow)
        mainWindow = new Nome::CMainWindow;
    mainWindow->show();

    return application.exec();
}

#endif
