#pragma once
#include <set>
#include "wx/wx.h"
#include "entt/entt.hpp"
#include "forms/dummy_window.hpp"

class wxSingleInstanceChecker;

namespace ewsm {

    class PathsSystem;
    class SettingsSystem;
    class DummyWindow;
    class MyTaskBarIcon;

    class Application final : public wxApp {
    public:
        bool OnInit() override;
        int OnExit() override;
        void OnQueryEndSession(wxCloseEvent& event);

        void register_test_hotkey() noexcept;
        void unregister_test_hotkey() noexcept;

        void open_main_window() noexcept;

        DummyWindow *dummy_window = nullptr;
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