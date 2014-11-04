#include "ImagePanel.h"

using namespace std;

ImagePanel::ImagePanel(wxWindow *parent, wxWindowID winid, const wxPoint& pod,
    const wxSize& size, long style, const wxString& name)
    : wxPanel(parent, winid, pod, size, style, name),
    bitmap(wxNullBitmap)
{
    Bind(wxEVT_PAINT, &ImagePanel::OnPaint, this);
    Bind(wxEVT_ERASE_BACKGROUND, &ImagePanel::OnEraseBackground, this);
}

ImagePanel::~ImagePanel()
{
}

void ImagePanel::OnPaint(wxPaintEvent& event)
{
    if (bitmap.GetWidth() < 1)
    {
        return;
    }

    wxBufferedPaintDC dc(this);
    dc.Clear();
    dc.DrawBitmap(bitmap, 0, 0);
}

void ImagePanel::OnEraseBackground(wxEraseEvent& event)
{
}

void ImagePanel::SetBitmap(const wxBitmap& bitmap)
{
    this->bitmap = bitmap;
    Refresh();
}
