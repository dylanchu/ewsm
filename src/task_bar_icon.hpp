#pragma once
#include <memory>
#include "wx/string.h"
#include "wx/taskbar.h"


namespace ewsm
{

    class DummyWindow;

    class MyTaskBarIcon : public wxTaskBarIcon
    {
        enum wxOwnedId
        {
            ID_open_main_window = 101,
            ID_About,
            ID_my_test,
        };

    public:
        MyTaskBarIcon();
        ~MyTaskBarIcon() override;

        wxMenu* CreatePopupMenu() override;
        void set_tooltip(const wxString& tooltip);

    private:
        void on_quit(wxCommandEvent& event);
        void on_open_main_window(wxCommandEvent& event);
        void on_left_btn_dclick(wxTaskBarIconEvent& event);
        void on_close_ev(wxCloseEvent& event);
        void on_my_test(wxCommandEvent& event);

        wxDECLARE_EVENT_TABLE();
    };
} // namespace ewsm
