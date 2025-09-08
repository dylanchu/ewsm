#pragma once
#include "wx/wx.h"
#include <uiautomation.h>

namespace utils
{
#ifdef __WXMSW__
    wxString GetExecutablePathWithQuotes();
    bool IsAutoStartWithWindows();
    void SetAutoStartWithWindows(bool val);
    void DisableIME(wxWindow* w);
    wxString GetElementUnderCursor(wxRect& rect, wxString& info);
    wxRect GetWindowUnderCursor();

    struct ElementPicker
    {
        ~ElementPicker();
        bool Init();
        wxString GetElement(wxRect& rect);
        static std::pair<HWND, wxRect> GetWindowUnderMouseWithRect();
        IUIAutomation* automation = nullptr;
    };

#endif
} // namespace utils
