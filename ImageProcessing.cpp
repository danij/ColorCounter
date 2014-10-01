#include "ImageProcessing.h"

void HsvToRgb(short h, unsigned char s, unsigned char v,
    unsigned char& r, unsigned char& g, unsigned char& b)
{
    if (s == 0)
    {
        r = v;
        g = v;
        b = v;
        return;
    }
    if (h >= 360)
    {
        h = 0;
    }

    float _s = (float)s / 255;
    float _v = (float)v / 255;
    float hh, ff;
    unsigned char p, q, t;
    int region;

    hh = h / 60.0f;
    region = (int)hh;
    ff = hh - region;

    p = (unsigned char)(255 * _v * (1.0 - _s));
    q = (unsigned char)(255 * _v * (1.0 - (_s * ff)));
    t = (unsigned char)(255 * _v * (1.0 - (_s * (1.0 - ff))));

    switch (region)
    {
    case 0:
        r = v; g = t; b = p;
        break;
    case 1:
        r = q; g = v; b = p;
        break;
    case 2:
        r = p; g = v; b = t;
        break;
    case 3:
        r = p; g = q; b = v;
        break;
    case 4:
        r = t; g = p; b = v;
        break;
    default:
        r = v; g = p; b = q;
        break;
    }

    return;
}

void RgbToHsv(unsigned char r, unsigned char g, unsigned char b,
    short& h, unsigned char& s, unsigned char& v)
{
    unsigned char rgbMin, rgbMax;

    rgbMin = r < g ? (r < b ? r : b) : (g < b ? g : b);
    rgbMax = r > g ? (r > b ? r : b) : (g > b ? g : b);

    v = rgbMax;
    if (v == 0)
    {
        h = 0;
        s = 0;
        return;
    }

    s = 255 * long(rgbMax - rgbMin) / v;
    if (s == 0)
    {
        h = 0;
        return;
    }

    if (rgbMax == r)
        h = 0 + 60 * (g - b) / (rgbMax - rgbMin);
    else if (rgbMax == g)
        h = 2 * 60 + 60 * (b - r) / (rgbMax - rgbMin);
    else
        h = 4 * 60 + 60 * (r - g) / (rgbMax - rgbMin);

    if (h < 0)
    {
        h = 360 + h;
    }

    return;
}

