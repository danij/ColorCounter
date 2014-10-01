#ifndef _COLORPERCENT_H__
#define _COLORPERCENT_H__

#include <wx/wx.h>

class ColorPercent
{
public:
    ColorPercent();
    ColorPercent(wxColour color, float percent);
    virtual ~ColorPercent();
    wxColour GetColor() const;
    float GetPercent() const;
    void SetColor(wxColour color);
    void SetPercent(float percent);

protected:
    wxColour color;
    float percent;
};

#endif