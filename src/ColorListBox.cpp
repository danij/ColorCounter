#include "ColorListBox.h"
#include <stdlib.h>

ColorListBox::ColorListBox(wxWindow *parent, wxWindowID id, const wxPoint& pos,
    const wxSize& size, long style, const wxString& name)
    : wxVListBox(parent, id, pos, size, style, name), itemSize(40), borderSize(5)
{
}

ColorListBox::~ColorListBox()
{
}

void ColorListBox::SetItems(const std::vector<ColorPercent>& items)
{
    this->items = items;
    maxPercent = 0;

    for (auto& item : items)
    {
        float percent;
        if ((percent = item.GetPercent()) > maxPercent)
        {
            maxPercent = percent;
        }
    }

    SetItemCount(items.size());
    Refresh();
}

void ColorListBox::OnDrawItem(wxDC &dc, const wxRect &rect, size_t n) const
{
    auto& item = items[n];
    dc.SetPen(wxPen(item.GetColor()));
    dc.SetBrush(wxBrush(item.GetColor()));

    dc.DrawRectangle(wxPoint(rect.x + borderSize, rect.y + borderSize), 
        wxSize(itemSize - 2 * borderSize, itemSize - 2 * borderSize));

    if (IsSelected(n) && FindFocus() == this)
    {
        dc.SetTextForeground(*wxWHITE);
    }
    else
    {
        dc.SetTextForeground(*wxBLACK);
    }

    auto percent = item.GetPercent();
    if (abs(percent - maxPercent) < 0.01)
    {
        dc.SetFont(dc.GetFont().Bold());
    }
    else
    {
        dc.SetFont(*wxNORMAL_FONT);
    }

    auto text = wxString::Format(wxT("%.2f %%"), percent);
    auto extent = dc.GetTextExtent(text);    
    
    dc.DrawText(text, rect.x + rect.width - borderSize - extent.x,
        rect.y + (rect.height - extent.y) / 2);
}

wxCoord ColorListBox::OnMeasureItem(size_t n) const
{
    return itemSize;
}
