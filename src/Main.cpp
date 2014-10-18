#include "Main.h"
#include "MainWindow.h"

IMPLEMENT_APP(ColorCounterApp)

bool ColorCounterApp::OnInit()
{
    wxImage::AddHandler(new wxJPEGHandler());
    wxImage::AddHandler(new wxPNGHandler());
    wxImage::AddHandler(new wxGIFHandler());

    auto mainWindow = new MainWindow();
    if (argc > 1)
    {
        mainWindow->SetCommandLineOpenRequest(argv[1]);
    }
    mainWindow->Show();
    return true;
}