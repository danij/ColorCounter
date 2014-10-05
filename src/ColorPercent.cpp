#include "ColorPercent.h"

ColorPercent::ColorPercent() : ColorPercent(*wxWHITE, 0)
{
}

ColorPercent::ColorPercent(wxColour color, float percent) : color(color), percent(percent)
{
}

ColorPercent::~ColorPercent()
{
}

wxColour ColorPercent::GetColor() const
{
    return color;
}

float ColorPercent::GetPercent() const
{
    return percent;
}

void ColorPercent::SetColor(wxColour color)
{
    this->color = color;
}

void ColorPercent::SetPercent(float percent)
{
    this->percent = percent;
}