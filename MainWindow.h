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
    HueProcessor hueProcessor;
    const int displaySaturation = 255;
    const int displayValue = 224;

    void Initialize();
    void ProcessFile(const wxString& fileName);
    void DrawHistogram(const Histogram& histogram);
    void RefreshSampleValues();
    void DisplaySampleValues(const Histogram& histogram);
    void OnShow(wxShowEvent& event);
    void OnSelectImageClick(wxCommandEvent& event);
    void OnSampleRangeChange(wxCommandEvent& event);
};

#endif