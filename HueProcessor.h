#ifndef _HUEPROCESSOR_H__
#define _HUEPROCESSOR_H__

#include "Histogram.h"
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
    void CalculateSamples(int sampleRange); 

private:
    Histogram hueHistogram;
    Histogram huePartialHistogram;
};

#endif