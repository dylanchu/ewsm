#pragma once
#include "wx/event.h"
#include "wx/frame.h"

namespace ewsm {

class MainWindow: public wxFrame {
    using Super = wxFrame;
public:
    enum wxOwnedID {
        ID_Hello = 1,
        ID_About,
        ID_Exit,
        ID_hotkey_test,
    };
    wxMenuBar *create_menu_bar();

public:
    MainWindow();
    ~MainWindow();
    void on_hello(wxCommandEvent &event);
    void on_about(wxCommandEvent &event);
    void on_hotkey(wxKeyEvent &event);

    bool is_showing = true;
    bool is_finilized = false;

    DECLARE_EVENT_TABLE();
};

} // namespace ewsm