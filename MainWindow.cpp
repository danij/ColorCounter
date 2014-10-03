#include <wx/sizer.h>
#include <wx/rawbmp.h>
#include <wx/statline.h>
#include <wx/graphics.h>
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
    : wxFrame(NULL, wxID_ANY, wxT("Color Counter"), wxDefaultPosition, wxSize(800, 650))
{
    Initialize();
}

MainWindow::~MainWindow()
{
    for (auto& item : histograms)
    {
        delete item;
    }
}

void MainWindow::Initialize()
{
    SetMinClientSize(wxSize(800, 650));

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
    settingsSizer->Add(selectImageButton, wxSizerFlags(0).Center().Border(wxLEFT, 5));

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
    resultSizer->Add(resultListBox, wxSizerFlags(1).Expand().Border(wxLEFT, 5));

    auto mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(settingsSizer, wxSizerFlags(0).Expand().Border(wxALL, 5));
    mainSizer->Add(resultSizer, wxSizerFlags(1).Expand().Border(wxALL, 5));
    mainPanel->SetSizer(mainSizer);

    selectImageButton->SetFocus();

    for (int i = 0; i < 2; i++)
    {
        histograms[i] = new wxBitmap(histogramPanel->GetSize().GetX(), histogramPanel->GetSize().GetY(), 24);
        histograms[i + 2] = new wxBitmap(pieHistogramPanel->GetSize().GetX(), pieHistogramPanel->GetSize().GetY(), 24);
    }

    Bind(wxEVT_SHOW, &MainWindow::OnShow, this);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MainWindow::OnSelectImageClick, this, selectImageButton->GetId());
    Bind(wxEVT_COMMAND_COMBOBOX_SELECTED, &MainWindow::OnSampleRangeChange, this, sampleRangesComboBox->GetId());
    Bind(wxEVT_CHECKBOX, &MainWindow::OnLogValuesCheckBoxClick, this, logValuesCheckBox->GetId());
}

void MainWindow::OnShow(wxShowEvent& event)
{
    sampleRangesComboBox->AppendString(wxT("15"));
    sampleRangesComboBox->AppendString(wxT("30"));
    sampleRangesComboBox->AppendString(wxT("60"));
    sampleRangesComboBox->AppendString(wxT("120"));

    sampleRangesComboBox->Select(2);
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
    
    auto previousCursor = GetCursor();
    SetCursor(wxCURSOR_WAIT);

    ProcessFile(openFileDialog.GetPath());

    SetCursor(previousCursor);
}

void MainWindow::ProcessFile(const wxString& fileName)
{
    wxImage image(fileName);
    if ( ! image.IsOk())
    {
        wxMessageBox(wxT("The image you have selected is invalid!"), wxT("Error"), wxOK | wxICON_ERROR, this);
        return;
    }

    hueProcessor.Clear();
    wxImagePixelData pixelData(image);
    wxImagePixelData::Iterator it(pixelData);

    for (int y = 0; y < image.GetHeight(); y++)
    {
        for (int x = 0; x < image.GetWidth(); x++, ++it)
        {
            hueProcessor.ProcessRGB(it.Red(), it.Green(), it.Blue());
        }
    }
    
    DrawHistogram(hueProcessor.GetHueHistogram());
    RefreshSampleValues();
}

void MainWindow::DrawHistogram(const Histogram& histogram)
{
    wxBitmap& bitmap = *histograms[0];
    wxBitmap& logBitmap = *histograms[1];
    wxBitmap& pieBitmap = *histograms[2];
    wxBitmap& pieLogBitmap = *histograms[3];

    auto bitmapHeight = bitmap.GetHeight();

    wxMemoryDC dcs[4];
    wxMemoryDC& linearDC = dcs[0];
    wxMemoryDC& linearDCLog = dcs[1];
    wxMemoryDC& pieDC = dcs[2];
    wxMemoryDC& pieDCLog = dcs[3];

    for (int i = 0; i < 4; i++)
    {
        auto& dc = dcs[i];
        dc.SelectObject(*histograms[i]);
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

        linearDC.DrawLine(wxPoint(hue, bitmapHeight - 1), 
            wxPoint(hue, bitmapHeight - value * bitmapHeight / maxValue - 1));
        linearDCLog.DrawLine(wxPoint(hue, bitmapHeight - 1),
            wxPoint(hue, bitmapHeight - log10(value) * bitmapHeight / maxValueLog - 1));
        
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
        }

        for (auto dc : { &pieDC, &pieDCLog })
        {
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
    int indexShift = logValuesCheckBox->IsChecked() ? 1 : 0;

    histogramPanel->SetBitmap(*histograms[0 + indexShift]);
    pieHistogramPanel->SetBitmap(*histograms[2 + indexShift]);
}

void MainWindow::OnLogValuesCheckBoxClick(wxCommandEvent& event)
{
    if (resultListBox->GetItemCount() < 1)
    {
        return;
    }
    RefreshHistogramsDisplay();
}
