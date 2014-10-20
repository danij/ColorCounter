#ifndef _HUEPROCESSOR_H__
#define _HUEPROCESSOR_H__

#include "HueHistogram.h"
#include "ImageProcessing.h"

class HueProcessor
{
public:
    HueProcessor();
    virtual ~HueProcessor();
    void Clear();
    Histogram& GetHueHistogram();
    Histogram& GetHuePartialHistogram();
    void ProcessRGB(unsigned char r, unsigned char g, unsigned char b);
    void ProcessRGB(unsigned char* rgb, size_t width, size_t height);
    void CalculateSamples(int sampleRange); 

private:
    HueHistogram hueHistogram;
    HueHistogram huePartialHistogram;
};

#endif
