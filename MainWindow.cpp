#include <wx/sizer.h>
#include <wx/rawbmp.h>
#include <vector>
#include "MainWindow.h"
#include "HueProcessor.h"
#include "ImageProcessing.h"
#include "ColorPercent.h"

using namespace std;

MainWindow::MainWindow()
    : wxFrame(NULL, wxID_ANY, wxT("Color Counter"), wxDefaultPosition, wxSize(800, 600))
{
    Initialize();
}

MainWindow::~MainWindow()
{
}

void MainWindow::Initialize()
{
    SetMinClientSize(wxSize(600, 550));

    auto mainPanel = new wxPanel(this);

    auto settingsSizer = new wxStaticBoxSizer(wxHORIZONTAL, mainPanel, wxT("Settings"));
    auto valuesLabel = new wxStaticText(settingsSizer->GetStaticBox(), wxID_ANY, wxT("Average Results Every:"));
    sampleRangesComboBox = new wxComboBox(settingsSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition,
        wxDefaultSize, 0, 0, wxCB_READONLY);
    auto selectImageButton = new wxButton(settingsSizer->GetStaticBox(), wxID_ANY, wxT("&Select Image"));
    settingsSizer->Add(valuesLabel, wxSizerFlags(0).Center().Border(wxLEFT | wxRIGHT, 5));
    settingsSizer->Add(sampleRangesComboBox, wxSizerFlags(1).Center().Border(wxLEFT | wxRIGHT, 5));
    settingsSizer->Add(selectImageButton, wxSizerFlags(0).Center().Border(wxLEFT, 5));

    auto resultSizer = new wxStaticBoxSizer(wxHORIZONTAL, mainPanel, wxT("Results"));
    histogramPanel = new ImagePanel(resultSizer->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxSize(360, 360));
    resultListBox = new ColorListBox(resultSizer->GetStaticBox(), wxID_ANY);
    resultSizer->Add(histogramPanel, wxSizerFlags(0).Border(wxLEFT | wxRIGHT, 5));
    resultSizer->Add(resultListBox, wxSizerFlags(1).Expand().Border(wxLEFT, 5));

    auto mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(settingsSizer, wxSizerFlags(0).Expand().Border(wxALL, 5));
    mainSizer->Add(resultSizer, wxSizerFlags(1).Expand().Border(wxALL, 5));
    mainPanel->SetSizer(mainSizer);

    selectImageButton->SetFocus();

    Bind(wxEVT_SHOW, &MainWindow::OnShow, this);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MainWindow::OnSelectImageClick, this, selectImageButton->GetId());
    Bind(wxEVT_COMMAND_COMBOBOX_SELECTED, &MainWindow::OnSampleRangeChange, this, sampleRangesComboBox->GetId());
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
    auto height = histogramPanel->GetSize().GetY();
    wxBitmap bitmap(360, height, 24);
    wxMemoryDC dc;

    dc.SelectObject(bitmap);
    dc.Clear();
    
    auto maxValue = histogram.MaxValue();
    for (auto& pair : histogram)
    {
        auto hue = pair.first;
        auto value = pair.second;
        unsigned char r, g, b;
        
        HsvToRgb(hue, 255, 192, r, g, b);
        dc.SetPen(wxPen(wxColour(r, g, b)));

        dc.DrawLine(wxPoint(hue, height - 1), wxPoint(hue, height - value * height / maxValue - 1));
    }
    
    dc.SelectObject(wxNullBitmap);

    histogramPanel->SetBitmap(bitmap);
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
        HsvToRgb(hue, 255, 224, r, g, b);

        items.push_back(ColorPercent(wxColour(r, g, b), percent));

        resultListBox->SetItems(items);
    }
}
