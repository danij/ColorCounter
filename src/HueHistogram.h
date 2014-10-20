#ifndef _HUE_HISTOGRAM_H__
#define _HUE_HISTOGRAM_H__

#include <map>
#include "Histogram.h"

class HueHistogram : public Histogram
{
public:
    HueHistogram();
    virtual ~HueHistogram();
    virtual void Add(int key, unsigned int value = 1);
    virtual void Clear();
    virtual int MaxValue() const;
    virtual std::map<int, int>::const_iterator begin();
    virtual std::map<int, int>::const_iterator end();

protected:
    int values[360];
    std::map<int, int> generatedMap;
    bool addedSinceLastMapRefresh;
    virtual void RefreshMap();
};

#endif
