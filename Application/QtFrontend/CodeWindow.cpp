#include "CodeWindow.h"
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>

namespace Nome
{

CCodeWindow::CCodeWindow(QWidget* parent) : QWidget(parent, Qt::Dialog)
{
    StatusLabel = new QLabel("OK");
    SyncBtn = new QPushButton("Sync");
    CodeEdit = new CCodeEditor();

    QVBoxLayout* mainLayout = new QVBoxLayout;
    QHBoxLayout* toolsLayout = new QHBoxLayout;
    toolsLayout->addWidget(StatusLabel);
    toolsLayout->addWidget(SyncBtn);
    mainLayout->addLayout(toolsLayout);
    mainLayout->addWidget(CodeEdit);

    setLayout(mainLayout);
    setWindowTitle("Code");
}

}
