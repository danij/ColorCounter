#ifndef _HISTOGRAM_H__
#define _HISTOGRAM_H__

#include <map>

class Histogram
{
public:
    Histogram();
    virtual ~Histogram();
    virtual void Add(int key, unsigned int value = 1);
    virtual void Clear();
    virtual int MaxValue() const;
    virtual int operator[](size_t index);
    virtual std::map<int, int>::const_iterator begin();
    virtual std::map<int, int>::const_iterator end();

protected:
    std::map<int, int> values;
};

#endif
