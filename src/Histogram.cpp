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

int Histogram::operator[](size_t index)
{
    return values[index];
}

map<int, int>::const_iterator Histogram::begin()
{
    return values.cbegin();
}

map<int, int>::const_iterator Histogram::end()
{
    return values.cend();
}
