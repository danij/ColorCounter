#include "HueProcessor.h"
#include "ImageProcessing.h"

HueProcessor::HueProcessor()
{
}

HueProcessor::~HueProcessor()
{
}

void HueProcessor::Clear()
{
    hueHistogram.Clear();
    huePartialHistogram.Clear();
}

Histogram& HueProcessor::GetHueHistogram()
{
    return hueHistogram;
}

Histogram& HueProcessor::GetHuePartialHistogram()
{
    return huePartialHistogram;
}

void HueProcessor::ProcessRGB(unsigned char r, unsigned char g, unsigned char b)
{
    short h;
    unsigned char s, v;
    
    RgbToHsv(r, g, b, h, s, v);

    if (s < 10 || (r < 2 && g < 2 && b < 2))
    {
        //inconclusive hue
        return;
    }

    hueHistogram.Add(h);
}

void HueProcessor::ProcessRGB(unsigned char* rgb, size_t width, size_t height)
{
    int i = 0;
    int max = width * height;
    unsigned char* r = rgb;
    unsigned char* g = r + 1;
    unsigned char* b = g + 1;
    while (i < max)
    {
        ProcessRGB(*r, *g, *b);
        r += 3;
        g += 3;
        b += 3;
        ++i;
    }
}

void HueProcessor::CalculateSamples(int sampleRange)
{
    huePartialHistogram.Clear();
    if (sampleRange < 1 || sampleRange > 180)
    {
        return;
    }

    for (auto& pair : hueHistogram)
    {
        auto hue = pair.first;
        hue = ((hue + (sampleRange / 2)) / sampleRange) * sampleRange;

        while (hue >= 360)
        {
            hue -= 360;
        }

        huePartialHistogram.Add(hue, pair.second);
    }
}
