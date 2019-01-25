#ifndef DISABLE_MAIN_FOR_TESTS

#include "QtFrontend/MainWindow.h"
#include <QApplication>
#include <QTimer>

#undef main
int main(int argc, char** argv)
{
    // Setup Qt
    QApplication application(argc, argv);
	QCoreApplication::setApplicationName("Nome");
	QGuiApplication::setApplicationDisplayName("Nome 3.0");
	QCoreApplication::setOrganizationName("UCBerkeley");
	QCoreApplication::setApplicationVersion("3.0");

    Nome::CMainWindow mainWindow;
    mainWindow.show();

    return application.exec();
}

#endif
