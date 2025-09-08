#include "task_bar_icon.hpp"
#include "app.hpp"
#include "ecs/context/paths/paths_ctx.hpp"
#include "ecs/system/paths/paths_system.hpp"
#include "main_window.hpp"
#include "utils/capture_utils.hpp"
#include "utils/debug_utils.hpp"
#include "utils/keyboard_hooker.hpp"
#include "utils/timer_manager.hpp"


using namespace ewsm;

wxBEGIN_EVENT_TABLE(MyTaskBarIcon, wxTaskBarIcon)
    EVT_MENU(wxID_EXIT, MyTaskBarIcon::on_quit)
    EVT_MENU(ID_open_main_window, MyTaskBarIcon::on_open_main_window)
    EVT_MENU(ID_my_test, MyTaskBarIcon::on_my_test)
    EVT_TASKBAR_LEFT_DCLICK(MyTaskBarIcon::on_left_btn_dclick)
    EVT_CLOSE(MyTaskBarIcon::on_close_ev)
wxEND_EVENT_TABLE()

MyTaskBarIcon::MyTaskBarIcon() :
    wxTaskBarIcon()
{
    SetIcon(wxIcon("IDI_APPICON"), wxGetApp().GetAppName());
}

MyTaskBarIcon::~MyTaskBarIcon()
{
    RemoveIcon();
}

wxMenu* MyTaskBarIcon::CreatePopupMenu()
{
    wxMenu* menu = new wxMenu;
    auto& app = wxGetApp();
#ifdef _DEBUG
    /* debug only - my test */
    menu->Append(ID_my_test, "My Test");
    menu->AppendSeparator();
#endif
    /* keyboard hook sub menu */
    auto keyboard_hook_sub_menu = utils::create_keyboard_remapper_menu(this);
    menu->AppendSubMenu(keyboard_hook_sub_menu, "Keys Remapper");
    menu->AppendSeparator();
    menu->Append(ID_open_main_window, "Main Window");
    menu->AppendSeparator();
    /* quit */
    menu->Append(wxID_EXIT, wxT("Quit ") + app.GetAppName(), "Exit the application");
    return menu;
}

void MyTaskBarIcon::set_tooltip(const wxString& tooltip)
{
    SetIcon(wxIcon("IDI_APPICON"), tooltip);
}

void MyTaskBarIcon::on_quit(wxCommandEvent& event)
{
    wxGetApp().ExitMainLoop();
}

void MyTaskBarIcon::on_open_main_window(wxCommandEvent& event)
{
    auto* main_window = wxGetApp().main_window;
    main_window->Show(true);
    main_window->Center();
    main_window->Raise();
    // SendMessage(wxGetApp().GetTopWindow()->GetHWND(), WM_QUERYENDSESSION, 0, 0);
}

void MyTaskBarIcon::on_left_btn_dclick(wxTaskBarIconEvent& event)
{
    auto* main_window = wxGetApp().main_window;
    main_window->Show(true);
    main_window->Raise();
}

void ewsm::MyTaskBarIcon::on_close_ev(wxCloseEvent& event)
{
    if (wxGetApp().IsMainLoopRunning()) {
        wxGetApp().ExitMainLoop();
    }
}

void ewsm::MyTaskBarIcon::on_my_test(wxCommandEvent& event)
{
    DEBUG_MSG("on_my_test");
}
