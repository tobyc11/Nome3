#ifndef DISABLE_MAIN_FOR_TESTS

#include "Nome/App.h"

//#include "FlowViz/FlowViz.h"
#include "QtFrontend/MainWindow.h"
#include <QApplication>
#include <QTimer>

#if WIN32
#include <Windows.h>
#endif

#undef main
int main(int argc, char** argv)
{
#if WIN32
    SetProcessDPIAware();
#endif

//    if (int ret = Nome::CApp::AppSetup())
//        return ret;

    // Setup Qt
    QApplication application(argc, argv);
    Nome::CMainWindow mainWindow;
    mainWindow.show();
    //Flow::Viz::CFlowViz widget;
    //widget.show();

    //   // Main loop
    //QTimer timer;
    //QObject::connect(&timer, &QTimer::timeout, [&]() {
    //	int flags = Nome::CApp::AppSingleFrame();
    //	if (flags & Nome::CApp::StatusWannaQuit)
    //       {
    //	    application.quit();
    //       }
    //});
    //timer.start();

    return application.exec();
    //int ret = application.exec();

    int ret = 0;
    while (1)
    {
        int flags = Nome::CApp::AppSingleFrame();
        if (flags & Nome::CApp::StatusWannaQuit)
        {
            break;
        }
    }

    // Cleanup
    Nome::CApp::AppCleanup();

    return ret;
}

#endif
