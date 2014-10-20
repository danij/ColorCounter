#ifndef _COLORLISTBOX_H__
#define _COLORLISTBOX_H__

#include <wx/wx.h>
#include <wx/vlbox.h>
#include <vector>
#include "ColorPercent.h"

class ColorListBox : public wxVListBox
{
public:
    ColorListBox(wxWindow *parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize, long style = 0, const wxString& name = wxListBoxNameStr);
    virtual ~ColorListBox();
    void SetItems(const std::vector<ColorPercent>& items);

protected:
    std::vector<ColorPercent> items;
    int itemSize;
    int borderSize;
    float maxPercent;
    virtual void OnDrawItem(wxDC &dc, const wxRect &rect, size_t n) const;
    virtual wxCoord OnMeasureItem(size_t n) const;
};

#endif
