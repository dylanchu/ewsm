#pragma once
#include <set>
#include "wx/wxprec.h"
#include "entt/entt.hpp"
#include "main_window.hpp"

class wxSingleInstanceChecker;

namespace ewsm {

    class PathsSystem;
    class SettingsSystem;
    class MainWindow;
    class MyTaskBarIcon;

    class Application final : public wxApp {
    public:
        bool OnInit() override;
        int OnExit() override;
        void OnQueryEndSession(wxCloseEvent& event);

        void register_test_hotkey() noexcept;
        void unregister_test_hotkey() noexcept;

        MainWindow *main_window = nullptr;
        MyTaskBarIcon *task_bar_icon = nullptr;
        wxSingleInstanceChecker* ins_checker= nullptr;

        // ecs
        entt::registry reg;
        PathsSystem* paths_system = nullptr;
        SettingsSystem* settings_system = nullptr;

        wxDECLARE_EVENT_TABLE();
    };

} // namespace ewsm

wxDECLARE_APP(ewsm::Application);