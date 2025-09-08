#pragma once
#include <wx/clipbrd.h>
#include <wx/msgdlg.h>

namespace utils
{
    inline bool copy_bmp_to_clipboard(const wxBitmap& bmp)
    {
        if (wxTheClipboard->Open()) {
            wxTheClipboard->SetData(new wxBitmapDataObject(bmp));
            wxTheClipboard->Close();
            return true;
        }
        wxMessageBox(wxT("Failed to open clipboard"), wxT("Error"), wxOK | wxICON_ERROR, nullptr);
        return false;
    }
}