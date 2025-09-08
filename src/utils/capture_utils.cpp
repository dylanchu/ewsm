#include "capture_utils.hpp"
#include <wx/msw/wrapwin.h>


wxBitmap* utils::get_cursor_bitmap()
{
    CURSORINFO ci{0};
    ICONINFO ii{0};

    ci.cbSize = sizeof(ci);
    if (!::GetCursorInfo(&ci) || (ci.flags & CURSOR_SHOWING) == 0) {
        wxLogError("Could not get the cursor info.");
        return nullptr;
    }

    // class AutoIconInfo from include\msw\private.h can be used instead
    if (!::GetIconInfo(ci.hCursor, &ii)) {
        wxLogError("Could not get icon info about the cursor.");
        return nullptr;
    }
    if (!ii.hbmColor)
        wxLogMessage("The cursor is monochrome.");
    else
        wxLogMessage("The cursor is in color.");
    ::DeleteObject(ii.hbmColor);
    ::DeleteObject(ii.hbmMask);

    wxIcon icon;

    if (!icon.CreateFromHICON(WXHICON(ci.hCursor))) {
        wxLogError("Could not create wxIcon from HCURSOR.");
        return nullptr;
    }

    auto bmp = new wxBitmap();
    bmp->CopyFromIcon(icon);
    // wxLogMessage("wxBitmap created from the cursor: size = %dx%d pixels, depth = %d, mask = %s.",
    //              bmp.GetWidth(), bmp.GetHeight(), bmp.GetDepth(), bmp.GetMask() ? "Yes" : "No");
    return bmp;
}
