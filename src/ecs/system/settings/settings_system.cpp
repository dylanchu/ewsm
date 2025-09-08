#include "settings_system.hpp"
#include "wx/fileconf.h"
#include "app.hpp"
#include "ecs/context/paths/paths_ctx.hpp"
#include "ecs/context/settings/settings_ctx.hpp"
#include "entt/entity/registry.hpp"
#include "utils/win_utils.hpp"
#include "utils/timer_manager.hpp"


ewsm::SettingsSystem::SettingsSystem(entt::registry& registry) : SystemBase(registry)
{
    auto& settings = reg.ctx().emplace<Settings>();
    ReadFromConf(settings);
    Reconcile(settings);

    auto_save_timer = utils::TimerManager::Get().AddRepeatTimer(30000, [this]() {
        //
    });
}

ewsm::SettingsSystem::~SettingsSystem()
{
    if (auto_save_timer) {
        utils::TimerManager::Get().RemoveTimer(auto_save_timer);
    }

    auto& settings = reg.ctx().get<Settings>();
    WriteToConf(settings);
}

ewsm::Settings& ewsm::SettingsSystem::GetSettings() const noexcept
{
    return reg.ctx().get<Settings>();
}


// ----------------------------------------------------------

void ewsm::SettingsSystem::ReadFromConf(Settings& settings)
{
    wxFileConfig app_config;
    app_config.SetPath("/general");
#define SETTING_FIELD(Type, Name, Default) app_config.Read(#Name, &settings.general.Name, Default);
    GENERAL_SETTING_FIELDS;
#undef SETTING_FIELD

    app_config.SetPath("/hotkeys");
#define SETTING_FIELD(Type, Name, Default) app_config.Read(#Name, &settings.hotkeys.Name, Default);
    HOTKEYS_SETTING_FIELDS
#undef SETTING_FIELD
}

void ewsm::SettingsSystem::WriteToConf(const Settings& settings)
{
    wxFileConfig app_config;
    app_config.SetPath("/general");
#define SETTING_FIELD(Type, Name, Default) app_config.Write(#Name, settings.general.Name);
    GENERAL_SETTING_FIELDS;
#undef SETTING_FIELD

    app_config.SetPath("/hotkeys");
#define SETTING_FIELD(Type, Name, Default) app_config.Write(#Name, settings.hotkeys.Name);
    HOTKEYS_SETTING_FIELDS;
#undef SETTING_FIELD
    app_config.Flush();
}

void ewsm::SettingsSystem::Reconcile(Settings& settings)
{
    settings.general.auto_start = utils::IsAutoStartWithWindows();
}

void ewsm::SettingsSystem::Apply(const Settings& settings)
{
    utils::SetAutoStartWithWindows(settings.general.auto_start);
    // TODO: refresh sizes of folded images
}
