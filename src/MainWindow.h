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
    void SetCommandLineOpenRequest(const wxString& fileName);

protected:
    static const int displaySaturation = 255;
    static const int displayValue = 224;
    static const int normalHistogramIndex = 0;
    static const int pieHistogramIndex = 1;
    static const int linearHistogramIndex = 0;
    static const int logHistogramIndex = 1;
    static const int histogramLayoutTypes = 2; //normal and pie
    static const int histogramScaleTypes = 2; //linear and log
    static const int histogramTransitionDuration = 200; //ms
    static const int histogramTransitionFPS = 20;

    bool windowClosing;
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
    wxString commandLineOpenRequest;
    wxTimer* openRequestTimer;

    void Initialize();    
    void ProcessFile(const wxString& fileName);
    void DrawHistogram(Histogram& histogram);
    void RefreshSampleValues();
    void DisplaySampleValues(Histogram& histogram);
    void RefreshHistogramsDisplay();
    void OnShow(wxShowEvent& event);
    void OnClose(wxCloseEvent& event);
    void OnSelectImageClick(wxCommandEvent& event);
    void OnSampleRangeChange(wxCommandEvent& event);
    void OnLogValuesCheckBoxClick(wxCommandEvent& event);
    void OnHistogramTransitionTimer(wxTimerEvent& event);
    void OnOpenRequestTimer(wxTimerEvent& event);
    void OnDropFile(wxDropFilesEvent& event);
};

#endif