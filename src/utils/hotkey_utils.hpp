#pragma once
#include "wx/accel.h"
#include "wx/fileconf.h"

namespace utils {

inline bool FormatCombinedHotkeyString(int modifiers, int key_code, wxString &out) {
    if(modifiers & wxMOD_CONTROL)
        out += wxT("Ctrl+");
    if(modifiers & wxMOD_SHIFT)
        out += wxT("Shift+");
    if(modifiers & wxMOD_ALT)
        out += wxT("Alt+");
    const bool has_ord = key_code != WXK_CONTROL && key_code != WXK_SHIFT && key_code != WXK_ALT;
    if(has_ord) {
        out += wxAcceleratorEntry(0, key_code).ToString();
    }
    if (key_code == WXK_PAUSE || key_code == WXK_SCROLL)
        return true;
    const bool has_modifiers = modifiers & wxMOD_ALL;
    return has_modifiers && has_ord;
}

} // namespace utils