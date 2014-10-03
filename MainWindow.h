#ifndef _MAINWINDOW_H__
#define _MAINWINDOW_H__

#include <wx/wx.h>
#include "ImagePanel.h"
#include "ColorListBox.h"
#include "Histogram.h"
#include "HueProcessor.h"

class MainWindow : public wxFrame
{
public:
    MainWindow();
    virtual ~MainWindow();

protected:
    wxComboBox* sampleRangesComboBox;
    ImagePanel* histogramPanel;
    ImagePanel* pieHistogramPanel;
    ColorListBox* resultListBox;
    wxCheckBox* logValuesCheckBox;
    HueProcessor hueProcessor;
    wxBitmap* histograms[4];
    const int displaySaturation = 255;
    const int displayValue = 224;

    void Initialize();
    void ProcessFile(const wxString& fileName);
    void DrawHistogram(const Histogram& histogram);
    void RefreshSampleValues();
    void DisplaySampleValues(const Histogram& histogram);
    void RefreshHistogramsDisplay();
    void OnShow(wxShowEvent& event);
    void OnSelectImageClick(wxCommandEvent& event);
    void OnSampleRangeChange(wxCommandEvent& event);
    void OnLogValuesCheckBoxClick(wxCommandEvent& event);
};

#endif