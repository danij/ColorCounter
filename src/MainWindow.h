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
    static const int displaySaturation = 255;
    static const int displayValue = 224;
    static const int normalHistogramIndex = 0;
    static const int pieHistogramIndex = 1;
    static const int linearHistogramIndex = 0;
    static const int logHistogramIndex = 1;
    static const int histogramLayoutTypes = 2; //normal and pie
    static const int histogramScaleTypes = 2; //linear and log
    static const int histogramTransitionDuration = 500; //ms
    static const int histogramTransitionFPS = 10;

    wxComboBox* sampleRangesComboBox;
    ImagePanel* histogramPanel;
    ImagePanel* pieHistogramPanel;
    ColorListBox* resultListBox;
    wxCheckBox* logValuesCheckBox;
    HueProcessor hueProcessor;
    wxBitmap* histogramBitmaps[histogramLayoutTypes * histogramScaleTypes];
    wxBitmap* histogramTransitionFrames[histogramLayoutTypes][histogramTransitionFPS];
    wxTimer* histogramTransitionTimer;
    int histogramTransitionIndex;
    int histogramTransitionDirection;

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
    void OnHistogramTransitionTimer(wxTimerEvent& event);
};

#endif