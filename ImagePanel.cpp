#include "ImagePanel.h"

ImagePanel::ImagePanel(wxWindow *parent, wxWindowID winid, const wxPoint& pod,
    const wxSize& size, long style, const wxString& name)
    : wxPanel(parent, winid, pod, size, style, name)
{
    bitmap = wxBitmap(size.GetWidth(), size.GetHeight(), 24);
    memoryDC.SelectObject(bitmap);
    memoryDC.SetBackground(wxBrush(GetBackgroundColour()));
    memoryDC.Clear();

    Bind(wxEVT_PAINT, &ImagePanel::OnPaint, this);
    Bind(wxEVT_ERASE_BACKGROUND, &ImagePanel::OnEraseBackground, this);
}

ImagePanel::~ImagePanel()
{
}

void ImagePanel::OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(this);
    dc.Blit(wxPoint(0, 0), memoryDC.GetSize(), &memoryDC, wxPoint(0, 0));
}

void ImagePanel::OnEraseBackground(wxEraseEvent& event)
{
}

void ImagePanel::SetBitmap(const wxBitmap& bitmap)
{
    memoryDC.DrawBitmap(bitmap, 0, 0);
    Refresh();
}
