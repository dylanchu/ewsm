#include "win_utils.hpp"
#include "wx/wx.h"
#include "wx/msw/registry.h"
#include "wx/stdpaths.h"
#include "app.hpp"
#include <map>
#ifdef __WXMSW__
#include <windows.h>
#include <uiautomation.h>
#pragma comment(lib, "UIAutomationCore.lib")  // for GetElementUnderCursor
#endif


const static wxString autorun_reg_path("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run");

wxString utils::GetExecutablePathWithQuotes()
{
    return wxT("\"") + wxStandardPaths::Get().GetExecutablePath() + wxT("\"");
}

bool utils::IsAutoStartWithWindows()
{
    wxRegKey reg_key(autorun_reg_path);
    auto app_name = wxGetApp().GetAppName();
    if (reg_key.Exists() && reg_key.HasValue(app_name)) {
        wxString saved_path = wxEmptyString;
        reg_key.QueryValue(app_name, saved_path);
        return GetExecutablePathWithQuotes().Lower() == saved_path.Lower();
    }
    return false;
}

void utils::SetAutoStartWithWindows(bool val)
{
    bool cur_autostart = IsAutoStartWithWindows();
    auto app_name = wxGetApp().GetAppName();
    wxRegKey reg_key(autorun_reg_path);
    if (!cur_autostart && val) {
        if (!reg_key.Exists()) {
            reg_key.Create();
        }
        reg_key.SetValue(app_name, GetExecutablePathWithQuotes());
    }
    if (cur_autostart && !val) {
        reg_key.DeleteValue(app_name);
    }
}

void utils::DisableIME(wxWindow* w)
{
    HWND hwnd = (HWND)w->GetHandle();
    ImmAssociateContext(hwnd, nullptr); // remove ime association
}

namespace
{
    wxString control_type_to_string(const CONTROLTYPEID type)
    {
        static std::map<CONTROLTYPEID, wxString> typeMap = {
                {UIA_ButtonControlTypeId, "Button"},
                {UIA_EditControlTypeId, "TextBox"},
                {UIA_CheckBoxControlTypeId, "CheckBox"},
                {UIA_ComboBoxControlTypeId, "ComboBox"},
                {UIA_ListControlTypeId, "List"},
                {UIA_ListItemControlTypeId, "ListItem"},
                {UIA_HyperlinkControlTypeId, "Hyperlink"},
                {UIA_MenuControlTypeId, "Menu"},
                {UIA_MenuBarControlTypeId, "MenuBar"},
                {UIA_ProgressBarControlTypeId, "ProgressBar"},
                {UIA_ToolBarControlTypeId, "ToolBar"},
                {UIA_ToolTipControlTypeId, "ToolTip"},
                {UIA_StatusBarControlTypeId, "StatusBar"},
                {UIA_TreeControlTypeId, "Tree"},
                {UIA_TreeItemControlTypeId, "TreeItem"},
                {UIA_WindowControlTypeId, "Window"},
                {UIA_PaneControlTypeId, "Pane"},
                {UIA_TextControlTypeId, "Text"},
            };
        return typeMap.contains(type) ? typeMap[type] : wxString::Format("%d", type);
    }

    // 获取运行时类名（需要扩展）
    wxString get_runtime_class_name(IUIAutomationElement* element)
    {
        // 实际开发中需通过GetCurrentPropertyValue获取ClassName
        return "N/A";
    }

    // gen element uuid (based on RuntimeId)
    wxString get_element_unique_id(IUIAutomationElement* element)
    {
        if (!element)
            return wxEmptyString;

        SAFEARRAY* psa = nullptr;
        if (SUCCEEDED(element->GetRuntimeId(&psa)) && psa) {
            // convert RuntimeId to string
            LONG lBound, uBound;
            SafeArrayGetLBound(psa, 1, &lBound);
            SafeArrayGetUBound(psa, 1, &uBound);

            wxString idStr;
            int* pData = nullptr;
            if (SUCCEEDED(SafeArrayAccessData(psa, (void**)&pData))) {
                for (LONG i = lBound; i <= uBound; ++i) {
                    idStr += wxString::Format("%d-", pData[i]);
                }
                SafeArrayUnaccessData(psa);
            }
            SafeArrayDestroy(psa);
            return idStr;
        }
        return wxEmptyString;
    }
}

wxString utils::GetElementUnderCursor(wxRect& rect, wxString& info)
{
    POINT pt;
    GetCursorPos(&pt);

    IUIAutomation* automation = nullptr;
    IUIAutomationElement* element = nullptr;
    wxString uniq_id;

    if (SUCCEEDED(CoCreateInstance(CLSID_CUIAutomation, nullptr, CLSCTX_INPROC_SERVER, IID_IUIAutomation, (void**)&automation))) {
        if (SUCCEEDED(automation->ElementFromPoint(pt, &element))) {
            // get rect
            RECT rc;
            element->get_CurrentBoundingRectangle(&rc);
            rect = wxRect(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);

            // get element control type
            CONTROLTYPEID control_type;
            element->get_CurrentControlType(&control_type);
            if (control_type == UIA_PaneControlTypeId) {
                // 
            }

            // get element name
            BSTR name_bstr;
            element->get_CurrentName(&name_bstr);
            wxString name(name_bstr, wxConvUTF8);
            SysFreeString(name_bstr);

            // format info str
            info = wxString::Format(
                "Type: %s\nName: %s\nClass: %s",
                control_type_to_string(control_type),
                name,
                get_runtime_class_name(element)
                );

            uniq_id = get_element_unique_id(element);
            element->Release();
        }
        automation->Release();
    }
    return uniq_id;
}

wxRect utils::GetWindowUnderCursor()
{
    const auto pos = wxGetMousePosition();
    const HWND hwnd = WindowFromPoint(POINT{pos.x, pos.y});
    RECT rc;
    GetWindowRect(hwnd, &rc);
    return {rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top};
}

bool utils::ElementPicker::Init()
{
    return SUCCEEDED(CoCreateInstance(
        CLSID_CUIAutomation, nullptr, CLSCTX_INPROC_SERVER, IID_IUIAutomation, (void**)&automation
    ));
}

wxString utils::ElementPicker::GetElement(wxRect& rect)
{
    POINT pt;
    GetCursorPos(&pt);

    IUIAutomationElement* element = nullptr;
    wxString uniq_id;

    if (automation && SUCCEEDED(automation->ElementFromPoint(pt, &element))) {
        // get element control type
        CONTROLTYPEID control_type;
        element->get_CurrentControlType(&control_type);
        // if (control_type == UIA_TitleBarControlTypeId) {
        //     IUIAutomationElement* tmp = nullptr;
        //     element->GetCachedParent(&tmp);
        // }
        // get rect
        RECT rc;
        element->get_CurrentBoundingRectangle(&rc);
        rect = wxRect(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
        uniq_id = get_element_unique_id(element);
        element->Release();
    }
    return uniq_id;
}

std::pair<HWND, wxRect> utils::ElementPicker::GetWindowUnderMouseWithRect()
{
    POINT pt;
    ::GetCursorPos(&pt);
    HWND hwnd = ::WindowFromPoint(pt);
    if (!hwnd) {
        return {nullptr, wxRect()};
    }
    RECT r;
    ::GetWindowRect(hwnd, &r);
    return {hwnd, wxRect(r.left, r.top, r.right - r.left, r.bottom - r.top)};
}

utils::ElementPicker::~ElementPicker()
{
    if (automation) {
        automation->Release();
    }
}
