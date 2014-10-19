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

using namespace std;

#ifndef DEG2RAD
#define DEG2RAD(deg) ((float)(deg) * M_PI / 180)
#endif

MainWindow::MainWindow()
    : wxFrame(NULL, wxID_ANY, wxT("Color Counter"), wxDefaultPosition, wxSize(800, 670)),
    windowClosing(false), histogramTransitionIndex(0), histogramTransitionDirection(1),
    openRequestTimer(nullptr)
{
    Initialize();
}

MainWindow::~MainWindow()
{
    for (auto& item : histogramBitmaps)
    {
        delete item;
    }
    for (auto& item : histogramTransitionFrames)
    {
        for (auto& subItem : item)
        {
            delete subItem;
        }
    }
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
    SetMinClientSize(wxSize(800, 650));
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
    pieHistogramPanel = new ImagePanel(histogramParentPanel, wxID_ANY, wxDefaultPosition, wxSize(360, 300));
    histogramSizer->Add(histogramPanel, wxSizerFlags(0).Border(wxTOP, 5));
    histogramSizer->Add(histogramSplitterLine, wxSizerFlags(0).Expand().Border(wxTOP | wxBOTTOM, 5));
    histogramSizer->Add(pieHistogramPanel, wxSizerFlags(0).Border(wxBOTTOM, 5));
    histogramParentPanel->SetSizer(histogramSizer);

    resultListBox = new ColorListBox(resultSizer->GetStaticBox(), wxID_ANY);
    resultSizer->Add(histogramParentPanel, wxSizerFlags(0).Border(wxLEFT | wxRIGHT, 5));
    resultSizer->Add(resultListBox, wxSizerFlags(1).Expand().Border(wxLEFT | wxRIGHT, 5));

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
            histogramBitmaps[i * histogramScaleTypes + j] =
                new wxBitmap(panel->GetSize().GetX(), panel->GetSize().GetY(), 24);
        }
        for (int j = 0; j < histogramTransitionFPS; j++)
        {
            histogramTransitionFrames[i][j] =
                new wxBitmap(panel->GetSize().GetX(), panel->GetSize().GetY(), 24);
        }
    }

    histogramTransitionTimer = new wxTimer(this);

    Bind(wxEVT_SHOW, &MainWindow::OnShow, this);
    Bind(wxEVT_CLOSE_WINDOW, &MainWindow::OnClose, this);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MainWindow::OnSelectImageClick, this, selectImageButton->GetId());
    Bind(wxEVT_COMMAND_COMBOBOX_SELECTED, &MainWindow::OnSampleRangeChange, this, sampleRangesComboBox->GetId());
    Bind(wxEVT_CHECKBOX, &MainWindow::OnLogValuesCheckBoxClick, this, logValuesCheckBox->GetId());
    Bind(wxEVT_TIMER, &MainWindow::OnHistogramTransitionTimer, this, histogramTransitionTimer->GetId());
    Bind(wxEVT_DROP_FILES, &MainWindow::OnDropFile, this, NULL);
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

    for (int y = 0; y < image.GetHeight(); ++y)
    {
        for (int x = 0; x < image.GetWidth(); ++x)
        {
            hueProcessor.ProcessRGB(image.GetRed(x, y), image.GetGreen(x, y), image.GetBlue(x, y));
        }
    }

    DrawHistogram(hueProcessor.GetHueHistogram());
    RefreshSampleValues();
}

void MainWindow::DrawHistogram(const Histogram& histogram)
{
    wxBitmap& normalBitmap = *histogramBitmaps[normalHistogramIndex * histogramScaleTypes + linearHistogramIndex];
    wxBitmap& pieBitmap = *histogramBitmaps[pieHistogramIndex * histogramScaleTypes + linearHistogramIndex];

    auto bitmapHeight = normalBitmap.GetHeight();

    wxMemoryDC dcs[histogramLayoutTypes * histogramScaleTypes + histogramLayoutTypes * histogramTransitionFPS];
    wxMemoryDC& normalDC = dcs[normalHistogramIndex * histogramScaleTypes + linearHistogramIndex];
    wxMemoryDC& normalDCLog = dcs[normalHistogramIndex * histogramScaleTypes + logHistogramIndex];
    wxMemoryDC& pieDC = dcs[pieHistogramIndex * histogramScaleTypes + linearHistogramIndex];
    wxMemoryDC& pieDCLog = dcs[pieHistogramIndex * histogramScaleTypes + logHistogramIndex];

    for (int i = 0; i < sizeof(dcs) / sizeof(dcs[0]); i++)
    {
        auto& dc = dcs[i];
        if (i < histogramLayoutTypes * histogramScaleTypes)
        {
            dc.SelectObject(*histogramBitmaps[i]);
        }
        else
        {
            auto index = i - histogramLayoutTypes * histogramScaleTypes;
            dc.SelectObject(*histogramTransitionFrames[index / histogramTransitionFPS][index % histogramTransitionFPS]);
        }
        dc.SetBackground(wxBrush(histogramPanel->GetBackgroundColour()));
        dc.Clear();
    }

    auto pieWidth = min(pieBitmap.GetWidth(), pieBitmap.GetHeight());
    auto pieCenter = wxPoint(pieBitmap.GetWidth() / 2, pieBitmap.GetHeight() / 2);
    auto pieRadius = pieWidth / 2;

    auto maxValue = histogram.MaxValue();
    auto maxValueLog = log10(histogram.MaxValue());
    for (auto& pair : histogram)
    {
        auto hue = pair.first;
        auto value = pair.second;
        unsigned char r, g, b;

        HsvToRgb(hue, displaySaturation, displayValue, r, g, b);
        auto pen = wxPen(wxColour(r, g, b));

        for (auto& dc : dcs)
        {
            dc.SetPen(pen);
        }

        {
            auto linearY = bitmapHeight - value * bitmapHeight / maxValue - 1;
            auto logY = bitmapHeight - log10(value) * bitmapHeight / maxValueLog - 1;
            for (int y = bitmapHeight - 1; y >= linearY; y--)
            {
            	normalDC.DrawPoint(hue, y);
            }
            for (int y = bitmapHeight - 1; y >= logY; y--)
            {
            	normalDCLog.DrawPoint(hue, y);
            }

            for (int i = 0; i < histogramTransitionFPS; i++)
            {
                auto height = linearY - i * (linearY - logY) / histogramTransitionFPS;
                auto& dc = dcs[histogramLayoutTypes * histogramScaleTypes + i];
                for (int y = bitmapHeight - 1; y >= height; y--)
                {
                    dc.DrawPoint(hue, y);
                }
            }
        }

        auto angle = DEG2RAD(hue);
        auto nextAngle = DEG2RAD((hue + 1) % 360);
        auto circleRadius = pieRadius / 8;
        auto radius = circleRadius + (pieRadius - circleRadius) * value / maxValue;
        auto radiusLog = circleRadius + (pieRadius - circleRadius) * log10(value) / maxValueLog;

        for (auto a = angle; a <= nextAngle; a += 0.0001)
        {
            auto pieOuter = wxPoint(pieCenter.x + cos(a) * radius, pieCenter.y - sin(a) * radius);
            pieDC.DrawLine(pieCenter, pieOuter);

            auto pieOuterLog = wxPoint(pieCenter.x + cos(a) * radiusLog, pieCenter.y - sin(a) * radiusLog);
            pieDCLog.DrawLine(pieCenter, pieOuterLog);

            for (int i = 0; i < histogramTransitionFPS; i++)
            {
                auto x = pieOuter.x + i * (pieOuterLog.x - pieOuter.x) / histogramTransitionFPS;
                auto y = pieOuter.y - i * (pieOuter.y - pieOuterLog.y) / histogramTransitionFPS;
                auto& dc = dcs[histogramLayoutTypes * histogramScaleTypes + histogramTransitionFPS + i];
                dc.DrawLine(pieCenter, wxPoint(x, y));
            }
        }

        for (auto dc : { &pieDC, &pieDCLog })
        {
            dc->SetPen(pieDC.GetBackground().GetColour());
            dc->SetBrush(pieDC.GetBackground().GetColour());
            dc->DrawCircle(pieCenter, circleRadius);
        }
        for (int i = 0; i < histogramTransitionFPS; i++)
        {
            auto dc = &dcs[histogramLayoutTypes * histogramScaleTypes + histogramTransitionFPS + i];
            dc->SetPen(pieDC.GetBackground().GetColour());
            dc->SetBrush(pieDC.GetBackground().GetColour());
            dc->DrawCircle(pieCenter, circleRadius);
        }

    }

    for (auto& dc : dcs)
    {
        dc.SelectObject(wxNullBitmap);
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

void MainWindow::DisplaySampleValues(const Histogram& histogram)
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
    int indexShift = logValuesCheckBox->IsChecked() ? logHistogramIndex : linearHistogramIndex;

    histogramPanel->SetBitmap(*histogramBitmaps[normalHistogramIndex * histogramScaleTypes + indexShift]);
    pieHistogramPanel->SetBitmap(*histogramBitmaps[pieHistogramIndex * histogramScaleTypes + indexShift]);
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
        histogramPanel->SetBitmap(*histogramBitmaps[normalHistogramIndex * histogramScaleTypes + logHistogramIndex]);
        pieHistogramPanel->SetBitmap(*histogramBitmaps[pieHistogramIndex * histogramScaleTypes + logHistogramIndex]);
        histogramTransitionTimer->Stop();
        histogramTransitionIndex = histogramTransitionFPS - 1;
        return;
    }
    if (histogramTransitionIndex < 0)
    {
        histogramPanel->SetBitmap(*histogramBitmaps[normalHistogramIndex * histogramScaleTypes + linearHistogramIndex]);
        pieHistogramPanel->SetBitmap(*histogramBitmaps[pieHistogramIndex * histogramScaleTypes + linearHistogramIndex]);
        histogramTransitionTimer->Stop();
        histogramTransitionIndex = 0;
        return;
    }

    histogramPanel->SetBitmap(*histogramTransitionFrames[normalHistogramIndex][histogramTransitionIndex]);
    pieHistogramPanel->SetBitmap(*histogramTransitionFrames[pieHistogramIndex][histogramTransitionIndex]);
    histogramTransitionIndex += histogramTransitionDirection;
}
