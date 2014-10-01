#include "Main.h"
#include "MainWindow.h"

IMPLEMENT_APP(ColorCounterApp)

bool ColorCounterApp::OnInit()
{
    wxImage::AddHandler(new wxJPEGHandler());
    wxImage::AddHandler(new wxPNGHandler());
    wxImage::AddHandler(new wxGIFHandler());

    auto mainWindow = new MainWindow();
    mainWindow->Show();
    return true;
}