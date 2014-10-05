#include "Histogram.h"

using namespace std;

Histogram::Histogram()
{
}

Histogram::~Histogram()
{
}

void Histogram::Add(int key, unsigned int value)
{
    auto it = values.find(key);
    if (it == values.end())
    {
        values.insert(make_pair(key, value));
    }
    else
    {
        it->second += value;
    }
}

void Histogram::Clear()
{
    values.clear();
}

int Histogram::MaxValue() const
{
    int max = 0;
    for (auto& pair : values)
    {
        if (pair.second > max)
        {
            max = pair.second;
        }
    }
    return max;
}

map<int, int>::const_iterator Histogram::begin() const
{
    return values.cbegin();
}

map<int, int>::const_iterator Histogram::end() const
{
    return values.cend();
}