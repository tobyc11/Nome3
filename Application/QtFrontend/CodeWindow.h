#pragma once
#include "CodeEditor.h"
#include <QWidget>

class QLabel;
class QPushButton;

namespace Nome
{

class CCodeWindow : public QWidget
{
    Q_OBJECT

public:
    CCodeWindow(QWidget* parent = nullptr);

private:
    QLabel* StatusLabel;
    QPushButton* SyncBtn;
    CCodeEditor* CodeEdit;
};

}
