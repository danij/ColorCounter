#include <wx/sizer.h>
#include <wx/rawbmp.h>
#include <wx/statline.h>
#include <wx/graphics.h>
#include <wx/arrstr.h>
#include <vector>
#include <algorithm>
#include <cmath>
#include "MainWindow.h"
#include "HueProcessor.h"
#include "ImageProcessing.h"
#include "ColorPercent.h"
#include "HistogramPlotter.h"

using namespace std;

MainWindow::MainWindow()
    : wxFrame(NULL, wxID_ANY, wxT("Color Counter"), wxDefaultPosition, wxSize(800, 750)),
    windowClosing(false), histogramTransitionIndex(0), histogramTransitionDirection(1),
    openRequestTimer(nullptr), plotCache(new HistogramPlotCache())
{
    Initialize();
}

MainWindow::~MainWindow()
{
    if (nullptr != openRequestTimer)
    {
        delete openRequestTimer;
    }
    if (nullptr != histogramTransitionTimer)
    {
        delete histogramTransitionTimer;
    }
}

void MainWindow::Initialize()
{
    SetMinClientSize(wxSize(800, 700));
    DragAcceptFiles(true);

#ifdef __WXMSW__
    SetIcon(wxIcon("IDI_ICON1"));
#endif

    auto mainPanel = new wxPanel(this);

    auto settingsSizer = new wxStaticBoxSizer(wxHORIZONTAL, mainPanel, wxT("Settings"));
    auto valuesLabel = new wxStaticText(settingsSizer->GetStaticBox(), wxID_ANY, wxT("Average results every (hue degrees):"));
    sampleRangesComboBox = new wxComboBox(settingsSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition,
        wxDefaultSize, 0, 0, wxCB_READONLY);
    logValuesCheckBox = new wxCheckBox(settingsSizer->GetStaticBox(), wxID_ANY, wxT("Logarithmic"));
    auto selectImageButton = new wxButton(settingsSizer->GetStaticBox(), wxID_ANY, wxT("&Select Image"));
    settingsSizer->Add(valuesLabel, wxSizerFlags(0).Center().Border(wxLEFT | wxRIGHT, 5));
    settingsSizer->Add(sampleRangesComboBox, wxSizerFlags(1).Center().Border(wxLEFT | wxRIGHT, 5));
    settingsSizer->Add(logValuesCheckBox, wxSizerFlags(0).Center().Border(wxLEFT | wxRIGHT, 5));
    settingsSizer->Add(selectImageButton, wxSizerFlags(0).Center().Border(wxLEFT | wxRIGHT, 5));

    auto resultSizer = new wxStaticBoxSizer(wxHORIZONTAL, mainPanel, wxT("Results"));

    auto histogramParentPanel = new wxPanel(resultSizer->GetStaticBox(), wxID_ANY);
    auto histogramSizer = new wxBoxSizer(wxVERTICAL);
    auto histogramSplitterLine = new wxStaticLine(histogramParentPanel, wxID_ANY);
    histogramPanel = new ImagePanel(histogramParentPanel, wxID_ANY, wxDefaultPosition, wxSize(360, 200));
    pieHistogramPanel = new ImagePanel(histogramParentPanel, wxID_ANY, wxDefaultPosition, wxSize(360, 360));
    histogramSizer->Add(histogramPanel, wxSizerFlags(0).Border(wxTOP, 0));
    histogramSizer->Add(histogramSplitterLine, wxSizerFlags(0).Expand().Border(wxTOP | wxBOTTOM, 10));
    histogramSizer->Add(pieHistogramPanel, wxSizerFlags(0).Border(wxBOTTOM, 5));
    histogramParentPanel->SetSizer(histogramSizer);

    resultListBox = new ColorListBox(resultSizer->GetStaticBox(), wxID_ANY);
    resultSizer->Add(histogramParentPanel, wxSizerFlags(0).Border(wxLEFT | wxRIGHT, 5));
    resultSizer->Add(resultListBox, wxSizerFlags(1).Expand().Border(wxLEFT | wxRIGHT | wxBOTTOM, 5));

    auto mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(settingsSizer, wxSizerFlags(0).Expand().Border(wxALL, 5));
    mainSizer->Add(resultSizer, wxSizerFlags(1).Expand().Border(wxALL, 5));
    mainPanel->SetSizer(mainSizer);

    selectImageButton->SetFocus();

    wxPanel* panels[] = { histogramPanel, pieHistogramPanel };
    for (int i = 0; i < histogramLayoutTypes; i++)
    {
        auto panel = panels[i];
        for (int j = 0; j < histogramScaleTypes; j++)
        {
            histogramBitmaps[i][j] = 
                shared_ptr<wxBitmap>(new wxBitmap(panel->GetSize().GetX(), panel->GetSize().GetY(), 32));
        }
        for (int j = 0; j < histogramTransitionFPS; j++)
        {
            histogramTransitionFrames[i][j] = 
                shared_ptr<wxBitmap>(new wxBitmap(panel->GetSize().GetX(), panel->GetSize().GetY(), 32));
        }
    }

    histogramTransitionTimer = new wxTimer(this);

    Bind(wxEVT_SHOW, &MainWindow::OnShow, this);
    Bind(wxEVT_CLOSE_WINDOW, &MainWindow::OnClose, this);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MainWindow::OnSelectImageClick, this, selectImageButton->GetId());
    Bind(wxEVT_COMMAND_COMBOBOX_SELECTED, &MainWindow::OnSampleRangeChange, this, sampleRangesComboBox->GetId());
    Bind(wxEVT_CHECKBOX, &MainWindow::OnLogValuesCheckBoxClick, this, logValuesCheckBox->GetId());
    Bind(wxEVT_TIMER, &MainWindow::OnHistogramTransitionTimer, this, histogramTransitionTimer->GetId());
    Bind(wxEVT_DROP_FILES, &MainWindow::OnDropFile, this, wxID_ANY);
}

void MainWindow::SetCommandLineOpenRequest(const wxString& fileName)
{
    commandLineOpenRequest = fileName;
}

void MainWindow::OnShow(wxShowEvent& event)
{
    for (auto& i : { 15, 30, 60, 120 })
    {
        sampleRangesComboBox->AppendString(wxString::Format(wxT("%d"), i));
    }

    sampleRangesComboBox->Select(2);

    if ( ! commandLineOpenRequest.IsEmpty())
    {
        openRequestTimer = new wxTimer(this);
        Bind(wxEVT_TIMER, &MainWindow::OnOpenRequestTimer, this, openRequestTimer->GetId());
        openRequestTimer->StartOnce(100);
    }
}

void MainWindow::OnOpenRequestTimer(wxTimerEvent& event)
{
    if ( ! commandLineOpenRequest.IsEmpty())
    {
        ProcessFile(commandLineOpenRequest);
    }
}

void MainWindow::OnClose(wxCloseEvent& event)
{
    windowClosing = true;
    Destroy();
}

void MainWindow::OnDropFile(wxDropFilesEvent& event)
{
    if (event.GetNumberOfFiles() < 1)
    {
        return;
    }

    ProcessFile(event.GetFiles()[0]);
}

void MainWindow::OnSelectImageClick(wxCommandEvent& event)
{
    wxArrayString extensions;
    wxString filter;
    const wxString prefix = wxT("*.");
    for (auto obj : wxImage::GetHandlers())
    {
        auto handler = static_cast<wxImageHandler*>(obj);
        extensions.Add(prefix + handler->GetExtension());
        for (auto extension : handler->GetAltExtensions())
        {
            extensions.Add(prefix + extension);
            extensions.Add(prefix + extension.Lower());
            extensions.Add(prefix + extension.Upper());
        }
    }

    filter = wxT("Image Files|") + wxJoin(extensions, ';') + wxT("|All Files|*.*");

    wxFileDialog openFileDialog(this, wxT("Select Image"), wxEmptyString, wxEmptyString, filter,
        wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (openFileDialog.ShowModal() == wxID_CANCEL)
    {
        return;
    }

    ProcessFile(openFileDialog.GetPath());
}

void MainWindow::ProcessFile(const wxString& fileName)
{
    wxBusyCursor busyCursor;

    wxImage image(fileName);
    if (!image.IsOk())
    {
        wxMessageBox(wxT("The image you have selected is invalid!"), wxT("Error"), wxOK | wxICON_ERROR, this);
        return;
    }

    hueProcessor.Clear();
    hueProcessor.ProcessRGB(image.GetData(), image.GetWidth(), image.GetHeight());

    DrawHistogram(hueProcessor.GetHueHistogram());
    RefreshSampleValues();

	SetTitle(wxString::Format("Color Counter - %s", fileName));
}

void MainWindow::DrawHistogram(Histogram& histogram)
{
    auto& normalBitmap = histogramBitmaps[normalHistogramIndex][linearHistogramIndex];
    auto& normalLogBitmap = histogramBitmaps[normalHistogramIndex][logHistogramIndex];
    auto& pieBitmap = histogramBitmaps[pieHistogramIndex][linearHistogramIndex];
    auto& pieLogBitmap = histogramBitmaps[pieHistogramIndex][logHistogramIndex];

    auto bitmapHeight = normalBitmap->GetHeight();

    HistogramPlotter plotter(displaySaturation, displayValue, plotCache);
    {
        //normal plots
        plotter.SetWidth(normalBitmap->GetWidth());
        plotter.SetHeight(normalBitmap->GetHeight());

        auto normalFrames = plotter.PlotFrames(histogram, histogramTransitionFPS);
        for (size_t i = 0; i < normalFrames.size(); i++)
        {
            histogramTransitionFrames[normalHistogramIndex][i] =
                shared_ptr<wxBitmap>(new wxBitmap(*(normalFrames[i]), 32));
        }
        normalBitmap = shared_ptr<wxBitmap>(new wxBitmap(*(normalFrames[0]), 32));
        normalLogBitmap = shared_ptr<wxBitmap>(new wxBitmap(*(normalFrames[normalFrames.size() - 1]), 32));
    }
    {
        //pie plots
        plotter.SetWidth(pieBitmap->GetWidth());
        plotter.SetHeight(pieBitmap->GetHeight());

        auto pieFrames = plotter.PlotPieFrames(histogram, histogramTransitionFPS);
        for (size_t i = 0; i < pieFrames.size(); i++)
        {
            histogramTransitionFrames[pieHistogramIndex][i] =
                shared_ptr<wxBitmap>(new wxBitmap(*(pieFrames[i]), 32));
        }
        pieBitmap = shared_ptr<wxBitmap>(new wxBitmap(*(pieFrames[0]), 32));
        pieLogBitmap = shared_ptr<wxBitmap>(new wxBitmap(*(pieFrames[pieFrames.size() - 1]), 32));
    }

    RefreshHistogramsDisplay();
}

void MainWindow::RefreshSampleValues()
{
    auto sampleRange = wxAtoi(sampleRangesComboBox->GetStringSelection());
    hueProcessor.CalculateSamples(sampleRange);
    DisplaySampleValues(hueProcessor.GetHuePartialHistogram());
}

void MainWindow::OnSampleRangeChange(wxCommandEvent& event)
{
    RefreshSampleValues();
}

void MainWindow::DisplaySampleValues(Histogram& histogram)
{
    resultListBox->Clear();
    vector<ColorPercent> items;

    int total = 0;
    for (auto& pair : histogram)
    {
        total += pair.second;
    }

    for (auto& pair : histogram)
    {
        float percent = (float)pair.second / total * 100;

        if (percent < 0.01)
        {
            continue;
        }

        auto hue = pair.first;
        unsigned char r, g, b;
        HsvToRgb(hue, displaySaturation, displayValue, r, g, b);

        items.push_back(ColorPercent(wxColour(r, g, b), percent));

        resultListBox->SetItems(items);
    }
}

void MainWindow::RefreshHistogramsDisplay()
{
    int scaleType = logValuesCheckBox->IsChecked() ? logHistogramIndex : linearHistogramIndex;

    histogramPanel->SetBitmap(*histogramBitmaps[normalHistogramIndex][scaleType]);
    pieHistogramPanel->SetBitmap(*histogramBitmaps[pieHistogramIndex][scaleType]);
}

void MainWindow::OnLogValuesCheckBoxClick(wxCommandEvent& event)
{
    histogramTransitionDirection = logValuesCheckBox->IsChecked() ? 1 : -1;

    if (resultListBox->GetItemCount() < 1)
    {
        histogramTransitionIndex = logValuesCheckBox->IsChecked() ? histogramTransitionFPS - 1 : 0;
        return;
    }
    histogramTransitionTimer->Start(histogramTransitionDuration / histogramTransitionFPS);
}

void MainWindow::OnHistogramTransitionTimer(wxTimerEvent& event)
{
    if (windowClosing)
    {
        return;
    }

    if (histogramTransitionIndex >= histogramTransitionFPS)
    {
        histogramPanel->SetBitmap(*histogramBitmaps[normalHistogramIndex][logHistogramIndex]);
        pieHistogramPanel->SetBitmap(*histogramBitmaps[pieHistogramIndex][logHistogramIndex]);
        histogramTransitionTimer->Stop();
        histogramTransitionIndex = histogramTransitionFPS - 1;
        return;
    }
    if (histogramTransitionIndex < 0)
    {
        histogramPanel->SetBitmap(*histogramBitmaps[normalHistogramIndex][linearHistogramIndex]);
        pieHistogramPanel->SetBitmap(*histogramBitmaps[pieHistogramIndex][linearHistogramIndex]);
        histogramTransitionTimer->Stop();
        histogramTransitionIndex = 0;
        return;
    }

    histogramPanel->SetBitmap(*histogramTransitionFrames[normalHistogramIndex][histogramTransitionIndex]);
    pieHistogramPanel->SetBitmap(*histogramTransitionFrames[pieHistogramIndex][histogramTransitionIndex]);
    histogramTransitionIndex += histogramTransitionDirection;
}
