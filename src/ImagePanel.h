#ifndef _PAINTPANEL_H__
#define _PAINTPANEL_H__

#include <wx/wx.h>

class ImagePanel : public wxPanel
{
public:
    ImagePanel(wxWindow *parent, wxWindowID winid = wxID_ANY, const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL | wxNO_BORDER,
        const wxString& name = wxPanelNameStr);
    virtual ~ImagePanel();
    void SetBitmap(const wxBitmap& bitmap);

protected:
    wxMemoryDC memoryDC;
    wxBitmap bitmap;
    void OnPaint(wxPaintEvent& event);
    void OnEraseBackground(wxEraseEvent& event);
};

#endif
