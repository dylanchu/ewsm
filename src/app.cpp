#include "app.hpp"
#include "wx/snglinst.h"
#include "ecs/context/settings/settings_ctx.hpp"
#include "ecs/system/paths/paths_system.hpp"
#include "ecs/system/settings/settings_system.hpp"
#include "forms/service_manager_frame.hpp"
#include "task_bar_icon.hpp"
#include "utils/debug_utils.hpp"
#include "utils/hotkey_utils.hpp"
#include "utils/timer_manager.hpp"
#include "utils/win_priviledge_utils.hpp"
// #include "wx/display.h"

#define SAFE_DELETE(x) if(x) {delete x; x = nullptr;}

BEGIN_EVENT_TABLE(ewsm::Application, wxApp)
    EVT_QUERY_END_SESSION(Application::OnQueryEndSession)
END_EVENT_TABLE()

bool ewsm::Application::OnInit() {

    // // 检查管理员权限
    // if (!utils::IsRunAsAdmin()) {
    //     if (!utils::RestartAsAdmin()) {
    //         wxMessageBox("This application requires administrator privileges.",
    //                      "Permission Required", wxOK | wxICON_ERROR);
    //         return false;
    //     }
    //     return false; // 当前实例退出
    // }

    SetAppName("Ewsm");
    SetAppDisplayName("EWSM");
    utils::SetupConsole(GetAppName() + " Console");
    SetExitOnFrameDelete(false);
    ins_checker = new wxSingleInstanceChecker("EwsmInsRunning", "Ewsm");
    if(ins_checker->IsAnotherRunning()) {
        wxMessageBox("Another instance is running", "Quit", wxOK | wxICON_ERROR);
        Exit();
    }
    // log for debug
    // { // debug: dpi info
    //     for(unsigned int i = 0; i < wxDisplay::GetCount(); ++i) {
    //         const wxDisplay display(i);
    //         wxLogMessage("Display %u: ppi = %dx%d, scale factor = %G",
    //                    i, display.GetPPI().x, display.GetPPI().y, display.GetScaleFactor());
    //     }
    // }
    // png
    wxImage::AddHandler(new wxPNGHandler());

    // ecs
    paths_system = new PathsSystem(reg);
    settings_system = new SettingsSystem(reg);

    task_bar_icon = new MyTaskBarIcon();
    dummy_window = new DummyWindow();
    dummy_window->Show(false);
    register_test_hotkey();

    open_main_window();

    return true;
}

int ewsm::Application::OnExit() {
    utils::TimerManager::Get().StopAllTimers();

    // reg clean up


    if(dummy_window) {
        dummy_window->Destroy();
    }
    SAFE_DELETE(task_bar_icon);
    SAFE_DELETE(ins_checker);
    //
    SAFE_DELETE(settings_system);
    SAFE_DELETE(paths_system);

    return 0;
}

void ewsm::Application::OnQueryEndSession(wxCloseEvent &event) {
    ExitMainLoop();
    event.Skip(false);
};

void ewsm::Application::register_test_hotkey() noexcept {
    // register hotkey
    auto& settings = settings_system->GetSettings();
    if(!dummy_window->RegisterHotKey(DummyWindow::ID_hotkey_test, settings.hotkeys.capture_modifiers, settings.hotkeys.capture_key_code)) {
        wxMessageBox("Failed to register hotkey", "Error", wxOK | wxICON_ERROR);
    }
}

void ewsm::Application::unregister_test_hotkey() noexcept {
    dummy_window->UnregisterHotKey(DummyWindow::ID_hotkey_test);
}

void ewsm::Application::open_main_window() noexcept
{
    const auto is_showing = ServiceManagerFrame::IsShowing();
    const auto main_window = ServiceManagerFrame::Get();
    if (is_showing) {
        main_window->Raise();
    }
    else {
        main_window->Center();
        main_window->Show();
    }
}

#ifdef _DEBUG
wxIMPLEMENT_APP_CONSOLE(ewsm::Application);
#else
wxIMPLEMENT_APP(ewsm::Application);
#endif