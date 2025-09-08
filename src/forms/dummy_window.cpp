#include "dummy_window.hpp"
#include "wx/fileconf.h"
#include "src/app.hpp"


BEGIN_EVENT_TABLE(ewsm::DummyWindow, wxFrame)
    EVT_HOTKEY(ID_hotkey_test, DummyWindow::on_hotkey)
END_EVENT_TABLE()

ewsm::DummyWindow::DummyWindow() :
    wxFrame(nullptr, wxID_ANY, wxGetApp().GetAppName())
{
    const auto menu_bar = create_menu_bar();
    Super::SetMenuBar(menu_bar);
    Super::CreateStatusBar();
    Super::GetStatusBar()->SetCanFocus(false);
    Super::SetStatusText(wxT("Welcome to ") + wxGetApp().GetAppName() + wxT("!"));

    menu_bar->Bind(wxEVT_MENU, [&](wxCommandEvent& event) {
        if (event.GetId() == ID_Hello) {
            on_hello(event);
        }
        else if (event.GetId() == wxID_ABOUT) {
            on_about(event);
        }
        else if (event.GetId() == wxID_EXIT) {
            is_finilized = true;
            wxGetApp().ExitMainLoop();
        }
        else
            event.Skip();
    });

    Bind(wxEVT_CLOSE_WINDOW, [&](wxCloseEvent& event) {
        if (!is_finilized) {
            Show(false);
            event.Veto();
        };
    });
}

ewsm::DummyWindow::~DummyWindow()
{
    Super::UnregisterHotKey(ID_hotkey_test);
}

void ewsm::DummyWindow::on_about(wxCommandEvent& event)
{
    wxMessageBox(
        wxGetApp().GetAppName().Capitalize() + " is a xx tool made with C++ and wxWidgets.",
        wxT("About ") + wxGetApp().GetAppName().Capitalize(),
        wxOK | wxICON_INFORMATION);
}

void ewsm::DummyWindow::on_hello(wxCommandEvent& event)
{
    // wxMessageBox(wxT("Hello from ") + wxGetApp().GetAppName() + wxT("!"));
}

wxMenuBar* ewsm::DummyWindow::create_menu_bar()
{
    auto menu_file = new wxMenu();
    menu_file->Append(ID_Hello, "&Hello...\tCtrl-H", "Help string shown in status bar for this menu item");
    menu_file->AppendSeparator();
    menu_file->Append(wxID_EXIT);
    auto menu_help = new wxMenu();
    menu_help->Append(wxID_ABOUT);
    auto menu_bar = new wxMenuBar();
    menu_bar->Append(menu_file, "&File");
    menu_bar->Append(menu_help, "&Help");
    return menu_bar;
}

void ewsm::DummyWindow::on_hotkey(wxKeyEvent& event)
{
    if (event.GetId() == ID_hotkey_test) {
    }
}
