#ifndef _HISTOGRAMPLOTTER_H__
#define _HISTOGRAMPLOTTER_H__

#include <wx/image.h>
#include <wx/colour.h>
#include <vector>
#include <map>
#include <memory>
#include "Histogram.h"

enum class PlotType;
struct PlotCacheInfo;
class HistogramPlotCache;

class HistogramPlotter
{
public:
    HistogramPlotter(const unsigned char saturation, const unsigned char value, 
        std::shared_ptr<HistogramPlotCache> cache);
    virtual ~HistogramPlotter();
    int GetWidth() const;
    void SetWidth(int width);
    int GetHeight() const;
    void SetHeight(int height);
    unsigned char GetSaturation() const;
    unsigned char GetValue() const;
    std::unique_ptr<wxImage> Plot(Histogram& histogram, bool logarithmic = false);
    std::unique_ptr<wxImage> PlotPie(Histogram& histogram, bool logarithmic = false);
    std::vector<std::unique_ptr<wxImage>> PlotFrames(Histogram& histogram, size_t frameCount);
    std::vector<std::unique_ptr<wxImage>> PlotPieFrames(Histogram& histogram, size_t frameCount);

protected:
    static const int PixelSize = 3 * sizeof(unsigned char);

    int width;
    int height;
    unsigned char saturation;
    unsigned char value;
    int colors[360];
    std::shared_ptr<HistogramPlotCache> plotCache;

    unsigned char* GetPlotFromCache(const PlotCacheInfo& info);
    unsigned char* DrawNormalFullPlot(const PlotCacheInfo& info);
    unsigned char* DrawPieFullPlot(const PlotCacheInfo& info);
};

enum class PlotType
{
    Normal,
    Pie
};

struct PlotCacheInfo
{
    PlotType plotType;
    int width;
    int height;

    bool operator<(const PlotCacheInfo& other) const;
};

class HistogramPlotCache
{
public:
    HistogramPlotCache();
    virtual ~HistogramPlotCache();
    unsigned char* operator[](const PlotCacheInfo& info);
    void Insert(const PlotCacheInfo& info, unsigned char* data);

protected:
    std::map<PlotCacheInfo, unsigned char*> cache;
};

#endif
