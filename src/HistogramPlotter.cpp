#include "HistogramPlotter.h"
#include "ImageProcessing.h"
#include <wx/bitmap.h>
#include <cstdlib>
#include <algorithm>
#include <functional>

using namespace std;

#ifndef RAD2DEG
#define RAD2DEG(deg) ((float)(deg) * 180 / M_PI)
#endif

inline float CalculateAngle(int x, int y, int centerX, int centerY);

HistogramPlotter::HistogramPlotter(const unsigned char saturation, const unsigned char value, 
    shared_ptr<HistogramPlotCache> cache) :
    saturation(saturation), value(value), plotCache(cache)
{
    unsigned char r, g, b;

    for (short hue = 0; hue < sizeof(colors) / sizeof(colors[0]); hue++)
    {
        HsvToRgb(hue, saturation, value, r, g, b);
        colors[hue] = (b << 16) | (g << 8) | r;
    }

    if (nullptr == cache)
    {
        plotCache = shared_ptr<HistogramPlotCache>(new HistogramPlotCache());
    }
}

HistogramPlotter::~HistogramPlotter()
{
}

int HistogramPlotter::GetWidth() const
{
    return width;
}

void HistogramPlotter::SetWidth(int width)
{
    this->width = width;
}

int HistogramPlotter::GetHeight() const
{
    return height;
}

void HistogramPlotter::SetHeight(int height)
{
    this->height = height;
}

unsigned char HistogramPlotter::GetSaturation() const
{
    return saturation;
}

unsigned char HistogramPlotter::GetValue() const
{
    return value;
}

unsigned char* HistogramPlotter::GetPlotFromCache(const PlotCacheInfo& info)
{
    auto found = (*plotCache)[info];
    if (nullptr != found)
    {
        return found;
    }

    unsigned char* result;

    switch (info.plotType)
    {
    case PlotType::Normal:
        result = DrawNormalFullPlot(info);
        break;
    case PlotType::Pie:
        result = DrawPieFullPlot(info);
        break;
    default:
        result = nullptr;
    }
    plotCache->Insert(info, result);

    return result;
}

unsigned char* HistogramPlotter::DrawNormalFullPlot(const PlotCacheInfo& info)
{
    auto result = new unsigned char[width * height * PixelSize];

    int hue, color;
    for (int x = 0; x < width; ++x)
    {
        hue = x * 360 / width;
        color = colors[hue];
        for (int y = 0; y < height; ++y)
        {
            memcpy(&result[y * width * PixelSize + x * PixelSize], &color, PixelSize);
        }
    }

    return result;
}

unsigned char* HistogramPlotter::DrawPieFullPlot(const PlotCacheInfo& info)
{
    auto result = new unsigned char[width * height * PixelSize];

    int hue, color;
    int centerX = width / 2;
    int centerY = height / 2;
    for (int x = 0; x < width; ++x)
    {
        for (int y = 0; y < height; ++y)
        {            
            hue = RAD2DEG(CalculateAngle(x, y, centerX, centerY));
            color = colors[hue];

            memcpy(&result[y * width * PixelSize + x * PixelSize], &color, PixelSize);
        }
    }

    return result;
}

vector<unique_ptr<wxImage>> HistogramPlotter::PlotFrames(Histogram& histogram, size_t frameCount)
{
    vector<unique_ptr<wxImage>> result;

    auto dataSize = width * height * PixelSize;
    auto cacheData = GetPlotFromCache({ PlotType::Normal, width, height });

    int maxValue = histogram.MaxValue();
    double maxValueLog = maxValue > 0 ? log10(maxValue) : 0;

    vector<unsigned char*> datas(frameCount);
    vector<unsigned char*> alphas(frameCount);

    for (size_t frame = 0; frame < frameCount; frame++)
    {
        //data and alpha will be freed by wxImage, so allocate them using malloc, as requested
        auto data = static_cast<unsigned char*>(malloc(dataSize));
        memcpy(data, cacheData, dataSize);
        auto alpha = static_cast<unsigned char*>(malloc(width * height));
        memset(alpha, 0, width * height);

        datas[frame] = data;
        alphas[frame] = alpha;
    }

    int linearValue, logValue, value;
    int hue;

    for (auto x = 0; x < width; ++x)
    {
        hue = x * 360 / width;
        linearValue = histogram[hue] * height / maxValue;
        logValue = histogram[hue] > 0 ? log10(histogram[hue]) * height / maxValueLog : 0;

        for (size_t frame = 0; frame < frameCount; ++frame)
        {
            value = linearValue + frame * (logValue - linearValue) / (frameCount - 1);
            for (int y = height - value; y < height; ++y)
            {
                alphas[frame][y * width + x] = 255;
            }
        }
    }
    for (size_t frame = 0; frame < frameCount; ++frame)
    {
        result.push_back(unique_ptr<wxImage>(new wxImage(width, height, datas[frame], alphas[frame], false)));
    }
    
    return result;
}

vector<unique_ptr<wxImage>> HistogramPlotter::PlotPieFrames(Histogram& histogram, size_t frameCount)
{
    vector<unique_ptr<wxImage>> result;

    auto dataSize = width * height * PixelSize;
    auto cacheData = GetPlotFromCache({ PlotType::Pie, width, height });

    int maxValue = histogram.MaxValue();
    double maxValueLog = maxValue > 0 ? log10(maxValue) : 0;

    vector<unsigned char*> datas(frameCount);
    vector<unsigned char*> alphas(frameCount);

    for (size_t frame = 0; frame < frameCount; frame++)
    {
        //data and alpha will be freed by wxImage, so allocate them using malloc, as requested
        auto data = static_cast<unsigned char*>(malloc(dataSize));
        memcpy(data, cacheData, dataSize);
        auto alpha = static_cast<unsigned char*>(malloc(width * height));
        memset(alpha, 0, width * height);

        datas[frame] = data;
        alphas[frame] = alpha;
    }

    int linearValue, logValue, value;
    int hue;
    int skipRadius = 20;
    int skipRadiusSquare = skipRadius * skipRadius;
    int centerX = width / 2;
    int centerY = height / 2;
    int valueSquare;

    int radius = min(width, height) / 2;
    int distanceSquare;

    for (auto x = 0; x < width; ++x)
    {
        for (int y = 0; y < height; ++y)
        {
            hue = RAD2DEG(CalculateAngle(x, y, centerX, centerY));
            linearValue = histogram[hue] * (radius - skipRadius) / maxValue;
            logValue = histogram[hue] > 0 ? log10(histogram[hue]) * (radius - skipRadius) / maxValueLog : 0;
            distanceSquare = (x - centerX) * (x - centerX) + (y - centerY) * (y - centerY);

            for (size_t frame = 0; frame < frameCount; ++frame)
            {
                value = linearValue + frame * (logValue - linearValue) / (frameCount - 1) + skipRadius;
                valueSquare = value * value;
                if (distanceSquare > skipRadiusSquare && distanceSquare < valueSquare)
                {
                    alphas[frame][y * width + x] = 255;
                }
            }
        }
    }
    for (size_t frame = 0; frame < frameCount; ++frame)
    {
        result.push_back(unique_ptr<wxImage>(new wxImage(width, height, datas[frame], alphas[frame], false)));
    }

    return result;
}


bool PlotCacheInfo::operator<(const PlotCacheInfo& other) const
{
    if (plotType != other.plotType)
    {
        return plotType < other.plotType;
    }
    if (width != other.width)
    {
        return width < other.width;
    }
    if (height != other.height)
    {
        return height < other.height;
    }

    return false;
}

HistogramPlotCache::HistogramPlotCache()
{

}

HistogramPlotCache::~HistogramPlotCache()
{
    for (auto& pair : cache)
    {
        delete pair.second;
    }
}

unsigned char* HistogramPlotCache::operator[](const PlotCacheInfo& info)
{
    auto result = cache.find(info);
    if (result == cache.end())
    {
        return nullptr;
    }
    return result->second;
}

void HistogramPlotCache::Insert(const PlotCacheInfo& info, unsigned char* data)
{
    cache.insert(make_pair(info, data));
}

inline float CalculateAngle(int x, int y, int centerX, int centerY)
{
    auto result = (float)(atan(abs((float)y - centerY) / abs((float)x - centerX)));
    if (y > centerY && x > centerX)
    {
        result = M_PI + M_PI - result;
    }
    else if (y > centerY)
    {
        result = M_PI + result;
    }
    else if (x < centerX)
    {
        result = M_PI - result;
    }

    return result;
}