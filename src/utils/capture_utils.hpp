#pragma once
#include <vector>
#include "wx/wx.h"

namespace utils
{
    wxBitmap* get_cursor_bitmap();

    template <typename T>
    wxMenu* create_delayed_capture_menu(const std::vector<unsigned int>& delays, T* handler, void (T::*func)(unsigned int))
    {
        auto menu = new wxMenu();
        for (auto t : delays) {
            auto i = menu->Append(wxID_ANY, wxString::Format("%ds", t));
            handler->Bind(wxEVT_MENU, [=](auto&) { (handler->*func)(t); }, i->GetId());
        }
        return menu;
    }
} // namespace utils
