#include "HueHistogram.h"

using namespace std;

HueHistogram::HueHistogram() : addedSinceLastMapRefresh(true)
{
    memset(values, 0, sizeof(values));
}

HueHistogram::~HueHistogram()
{
}

void HueHistogram::Add(int key, unsigned int value)
{
    addedSinceLastMapRefresh = true;
    values[key % (sizeof(values) / sizeof(values[0]))] += value;
}

void HueHistogram::Clear()
{
    addedSinceLastMapRefresh = true;
    memset(values, 0, sizeof(values));
}

int HueHistogram::MaxValue() const
{
    int max = 0;
    for (auto& val : values)
    {
        if (val > max)
        {
            max = val;
        }
    }
    return max;
}

int HueHistogram::operator[](size_t index)
{
    return values[index];
}

map<int, int>::const_iterator HueHistogram::begin() 
{
    RefreshMap();
    return generatedMap.cbegin();
}

map<int, int>::const_iterator HueHistogram::end() 
{
    RefreshMap();
    return generatedMap.cend();
}

void HueHistogram::RefreshMap()
{
    if (!addedSinceLastMapRefresh)
    {
        return;
    }
    addedSinceLastMapRefresh = false;

    generatedMap.clear();
    for (int i = 0; i < sizeof(values) / sizeof(values[0]); i++)
    {
        if (values[i] > 0)
        {
            generatedMap.insert(make_pair(i, values[i]));
        }
    }
}
