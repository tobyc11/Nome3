#pragma once

namespace Nome
{

class CMainWindow;
class CNome3DView;

class CFrontendContext
{
public:
    CMainWindow* MainWindow {};
    CNome3DView* NomeView {};
};

extern CFrontendContext* GFrtCtx;

}
