#ifndef _IMAGEPROCESSING_H__
#define _IMAGEPROCESSING_H__

void HsvToRgb(short h, unsigned char s, unsigned char v, 
    unsigned char& r, unsigned char& g, unsigned char& b);

void RgbToHsv(unsigned char r, unsigned char g, unsigned char b,
    short& h, unsigned char& s, unsigned char& v);

#endif
