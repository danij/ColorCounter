#ifndef _HISTOGRAM_H__
#define _HISTOGRAM_H__

#include <map>

class Histogram
{
public:
    Histogram();
    virtual ~Histogram();
    void Add(int key, unsigned int value = 1);
    void Clear();
    int MaxValue() const;
    std::map<int, int>::const_iterator begin() const;
    std::map<int, int>::const_iterator end() const;

protected:
    std::map<int, int> values;
};

#endif