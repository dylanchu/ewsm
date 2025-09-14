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

    static void LogError(const wxString& message)
    {
        const DWORD err = ::GetLastError();
        wchar_t* err_msg = nullptr;

        ::FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            err,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            reinterpret_cast<LPWSTR>(&err_msg),
            0,
            nullptr);

        const wxString full_message = wxString::Format(
            "%s: %s (Error %d)",
            message,
            err_msg ? wxString(err_msg) : wxString("Unknown error"),
            err);

        wxLogError(full_message);

        if (err_msg) {
            ::LocalFree(err_msg);
        }
    }

#endif
} // namespace utils
